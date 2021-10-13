#include <pch.hpp>
#include <Network/Connection.hpp>
#include <Network/Message.hpp>
#include <Network/OwnedMessage.hpp>
#include <Network/MessageType.hpp>
#include <Network/ANode.hpp>
#include <Network/Server/AServer.hpp>


// ------------------------------------------------------------------ explicit instantiations

template class ::network::Connection<::network::MessageType>;



// ------------------------------------------------------------------ *structors

template <
    ::detail::isEnum MessageType
> ::network::Connection<MessageType>::Connection(
    ::network::ANode<MessageType>& owner,
    ::boost::asio::ip::tcp::socket tcpSocket,
    ::boost::asio::ip::udp::socket udpSocket
)
    : m_owner{ owner }
    , m_tcpSocket{ ::std::move(tcpSocket) }
    , m_udpSocket{ ::std::move(udpSocket) }
{}


template <
    ::detail::isEnum MessageType
> ::network::Connection<MessageType>::~Connection() = default;



// ------------------------------------------------------------------ async - connection

template <
    ::detail::isEnum MessageType
> auto ::network::Connection<MessageType>::connectToClient(
    ::detail::Id id
)
    -> bool
{
    if (m_owner.getType() == ::network::ANode<MessageType>::Type::server) {
        if (m_tcpSocket.is_open()) {
            if (
                dynamic_cast<::network::AServer<MessageType>&>(m_owner)
                    .onClientConnect(this->shared_from_this())
            ) {
                m_id = id;
                this->identificate();
                return true;
            } else {
                m_owner.onConnectionDenial(this->shared_from_this());
                this->disconnect();
                return false;
            }
        } else {
            ::std::cerr << "[ERROR:NewConnection] Invalid socket.\n";
            return false;
        }
    } else {
        ::std::cerr << "[ERROR:Connection] A client cannot connect to another client.\n";
        return false;
    }
}

template <
    ::detail::isEnum MessageType
> void ::network::Connection<MessageType>::connectToServer(
    const ::std::string& host,
    const ::std::uint16_t port
)
{
    if (m_owner.getType() == ::network::ANode<MessageType>::Type::client) {
        m_tcpSocket.async_connect(
            ::boost::asio::ip::tcp::endpoint{ ::boost::asio::ip::address::from_string(host), port },
            [this](
                const boost::system::error_code& errorCode
            ) {
                if (errorCode) {
                    ::std::cerr << "[ERROR:NewConnection] Client failed to connect to the tcp Server.\n";
                    this->disconnect();
                } else {
                    this->identificate();
                }
            }
        );
    } else {
        throw ::std::runtime_error("A server cannot connect to another server.\n");
    }
}

template <
    ::detail::isEnum MessageType
> void ::network::Connection<MessageType>::targetServerUdpPort(
    const ::std::uint16_t port
)
{
    if (m_owner.getType() == ::network::ANode<MessageType>::Type::client) {
        m_udpSocket.async_connect(
            ::boost::asio::ip::udp::endpoint{ m_tcpSocket.remote_endpoint().address(), port },
            [this](
                const boost::system::error_code& errorCode
            ) {
                if (errorCode) {
                    ::std::cerr << "[ERROR:NewConnection] Client failed to connect to the tcp Server.\n";
                    this->disconnect();
                }
            }
        );
    } else {
        throw ::std::runtime_error("A server cannot target another server udp port.\n");
    }
}

template <
    ::detail::isEnum MessageType
> void ::network::Connection<MessageType>::disconnect()
{
    ::boost::asio::post(
        m_owner.getAsioContext(),
        [this]()
        {
            m_tcpSocket.close();
            m_owner.onDisconnect(this->shared_from_this());
        }
    );
}

template <
    ::detail::isEnum MessageType
> auto ::network::Connection<MessageType>::isConnected() const
    -> bool
{
    return m_tcpSocket.is_open();
}



// ------------------------------------------------------------------ async - tcpOut

template <
    ::detail::isEnum MessageType
> void ::network::Connection<MessageType>::tcpSend(
    ::network::Message<MessageType> message
)
{
    ::boost::asio::post(
        m_owner.getAsioContext(),
        [this, message]()
        {
            auto wasOutQueueEmpty{ m_tcpMessagesOut.empty() };
            m_tcpMessagesOut.push_back(::std::move(message));
            if (m_isValid && wasOutQueueEmpty) {
                this->tcpWriteAwaitingMessages();
            }
        }
    );
}



// ------------------------------------------------------------------ async - udpOut

template <
    ::detail::isEnum MessageType
> void ::network::Connection<MessageType>::udpSend(
    ::network::Message<MessageType> message
)
{
    ::boost::asio::post(
        m_owner.getAsioContext(),
        [this, message]()
        {
            auto wasOutQueueEmpty{ m_udpMessagesOut.empty() };
            m_udpMessagesOut.push_back(::std::move(message));
            if (m_isValid && wasOutQueueEmpty) {
                this->udpWriteHeader();
            }
        }
    );
}



// ------------------------------------------------------------------ other

template <
    ::detail::isEnum MessageType
> auto ::network::Connection<MessageType>::getId() const
    -> ::detail::Id
{
    return m_id;
}

template <
    ::detail::isEnum MessageType
> auto ::network::Connection<MessageType>::getOwner() const
    -> const ::network::ANode<MessageType>&
{
    return m_owner;
}



// ------------------------------------------------------------------ async - tcpOut

template <
    ::detail::isEnum MessageType
> void ::network::Connection<MessageType>::tcpWriteAwaitingMessages()
{
    this->tcpSendMessage<
        [](::network::Connection<MessageType>& self){
            self.m_tcpMessagesOut.remove_front();
            if (!self.m_tcpMessagesOut.empty()) {
                self.tcpWriteAwaitingMessages();
            }
        },
        [](::network::Connection<MessageType>& self, const boost::system::error_code& errorCode){
            ::std::cerr << "[ERROR:" << self.m_id << "] Write header failed: " << errorCode.message() << ".\n";
            self.disconnect();
        },
        [](::network::Connection<MessageType>& self, const boost::system::error_code& errorCode){
            ::std::cerr << "[ERROR:" << self.m_id << "] Write body failed: " << errorCode.message() << ".\n";
            self.disconnect();
        }
    >(m_tcpMessagesOut.front());
}



// ------------------------------------------------------------------ async - udpOut

template <
    ::detail::isEnum MessageType
> void ::network::Connection<MessageType>::udpWriteHeader(
    ::std::size_t bytesAlreadySent /* = 0 */
)
{
    m_udpSocket.async_send(
        ::boost::asio::buffer(
            m_udpMessagesOut.front().getHeaderAddr() + bytesAlreadySent,
            m_udpMessagesOut.front().getHeaderSize() - bytesAlreadySent
        ),
        [this, bytesAlreadySent](
            const boost::system::error_code& errorCode,
            const ::std::size_t length
        ) {
            if (errorCode) {
                ::std::cerr << "[ERROR:" << m_id << "] Write header failed: " << errorCode.message() << ".\n";
                this->disconnect();
            } else if (bytesAlreadySent + length < m_udpMessagesOut.front().getHeaderSize()) {
                this->udpWriteHeader(bytesAlreadySent + length);
            } else {
                if (!m_udpMessagesOut.front().isBodyEmpty()) {
                    this->udpWriteBody();
                } else {
                    m_udpMessagesOut.remove_front();
                    if (!m_udpMessagesOut.empty()) {
                        this->udpWriteHeader();
                    }
                }
            }
        }
    );
}

template <
    ::detail::isEnum MessageType
> void ::network::Connection<MessageType>::udpWriteBody(
    ::std::size_t bytesAlreadySent /* = 0 */
)
{
    m_udpSocket.async_send(
        ::boost::asio::buffer(
            m_udpMessagesOut.front().getBodyAddr() + bytesAlreadySent,
            m_udpMessagesOut.front().getBodySize() - bytesAlreadySent
        ),
        [this, bytesAlreadySent](
            const boost::system::error_code& errorCode,
            const ::std::size_t length
        ) {
            if (errorCode) {
                ::std::cerr << "[ERROR:" << m_id << "] Write body failed: " << errorCode.message() << ".\n";
                this->disconnect();
            } else if (bytesAlreadySent + length < m_udpMessagesOut.front().getBodySize()) {
                this->udpWriteBody(bytesAlreadySent + length);
            } else {
                m_udpMessagesOut.remove_front();
                if (!m_udpMessagesOut.empty()) {
                    this->udpWriteHeader();
                }
            }
        }
    );
}




// ------------------------------------------------------------------ async - tcpIn

template <
    ::detail::isEnum MessageType
> void ::network::Connection<MessageType>::startReadMessage()
{
    this->tcpReceiveMessage<
        [](::network::Connection<MessageType>& self){
            self.tcpTransferBufferToInQueue();
            self.startReadMessage();
        },
        [](::network::Connection<MessageType>& self, const boost::system::error_code& errorCode){
            if (errorCode != ::boost::asio::error::eof) {
                ::std::cerr << "[ERROR:" << self.m_id << "] Read header failed: " << errorCode.message() << ".\n";
            } else {
                ::std::cerr << "[" << self.m_id << "] Connection ended\n";
            }
            self.disconnect();
        },
        [](::network::Connection<MessageType>& self, const boost::system::error_code& errorCode){
            ::std::cerr << "[ERROR:" << self.m_id << "] Read body failed: " << errorCode.message() << ".\n";
            self.disconnect();
        }
    >();
}

template <
    ::detail::isEnum MessageType
> void ::network::Connection<MessageType>::tcpTransferBufferToInQueue()
{
    if (m_owner.getType() == ::network::ANode<MessageType>::Type::server) {
        m_owner.pushIncommingMessage(
            network::OwnedMessage<MessageType>{ m_tcpBufferIn, this->shared_from_this() }
        );
    } else {
        m_owner.pushIncommingMessage(network::OwnedMessage<MessageType>{ m_tcpBufferIn, nullptr });
    }
}



// ------------------------------------------------------------------ async - udpIn

template <
    ::detail::isEnum MessageType
> void ::network::Connection<MessageType>::udpReadHeader(
    ::std::size_t bytesAlreadyRead /* = 0 */
)
{
    m_udpSocket.async_receive(
        ::boost::asio::buffer(
            m_udpBufferIn.getHeaderAddr() + bytesAlreadyRead,
            m_udpBufferIn.getHeaderSize() - bytesAlreadyRead
        ),
        [this, bytesAlreadyRead](
            const boost::system::error_code& errorCode,
            const ::std::size_t length
        ) {
            if (errorCode) {
                ::std::cerr << "[ERROR:" << m_id << "] Read header failed: " << errorCode.message() << ".\n";
                this->disconnect();
            } else if (bytesAlreadyRead + length < m_udpBufferIn.getHeaderSize()) {
                this->udpReadHeader(bytesAlreadyRead + length);
            } else {
                if (!m_udpBufferIn.isBodyEmpty()) {
                    m_udpBufferIn.updateBodySize();
                    this->udpReadBody();
                } else {
                    this->udpTransferBufferToInQueue();
                }
            }
        }
    );
}

template <
    ::detail::isEnum MessageType
> void ::network::Connection<MessageType>::udpReadBody(
    ::std::size_t bytesAlreadyRead /* = 0 */
)
{
    m_udpSocket.async_receive(
        ::boost::asio::buffer(
            m_udpBufferIn.getBodyAddr() + bytesAlreadyRead,
            m_udpBufferIn.getBodySize() - bytesAlreadyRead
        ),
        [this, bytesAlreadyRead](
            const boost::system::error_code& errorCode,
            const ::std::size_t length
        ) {
            if (errorCode) {
                ::std::cerr << "[ERROR:" << m_id << "] Read body failed: " << errorCode.message() << ".\n";
                this->disconnect();
            } else if (bytesAlreadyRead + length < m_udpBufferIn.getBodySize()) {
                this->udpReadBody(bytesAlreadyRead + length);
            } else {
                this->udpTransferBufferToInQueue();
            }
        }
    );
}

template <
    ::detail::isEnum MessageType
> void ::network::Connection<MessageType>::udpTransferBufferToInQueue()
{
    if (m_owner.getType() == ::network::ANode<MessageType>::Type::server) {
        m_owner.pushIncommingMessage(
            network::OwnedMessage<MessageType>{ m_udpBufferIn, this->shared_from_this() }
        );
    } else {
        m_owner.pushIncommingMessage(network::OwnedMessage<MessageType>{ m_udpBufferIn, nullptr });
    }
    this->udpReadHeader();
}



// ------------------------------------------------------------------ async - securityProtocol

template <
    ::detail::isEnum MessageType
> void ::network::Connection<MessageType>::identificate()
{
    // send public key
    this->tcpSendRawData<
        [](...){},
        [](::network::Connection<MessageType>& self, const boost::system::error_code& errorCode){
            ::std::cerr << "[ERROR:" << self.m_id << "] Write header failed: " << errorCode.message() << ".\n";
            self.disconnect();
        }
    >(m_cipher.getPublicKeyAddr(), m_cipher.getPublicKeySize());

    // read public key
    this->tcpReceiveToRawData<
        [](::network::Connection<MessageType>& self){
            if (self.m_owner.getType() == ::network::ANode<MessageType>::Type::server) {
                return self.serverHandshake();
            } else {
                return self.clientHandshake();
            }
        },
        [](::network::Connection<MessageType>& self, const boost::system::error_code& errorCode){
            ::std::cerr << "[ERROR:" << self.m_id << "] Read public key failed: " << errorCode.message() << ".\n";
            self.disconnect();
        }
    >(m_cipher.getTargetPublicKeyAddr(), m_cipher.getPublicKeySize());
}



template <
    ::detail::isEnum MessageType
> void ::network::Connection<MessageType>::serverHandshake()
{
    // TODO generating random data through sodium
    auto baseValue{ static_cast<::std::uint64_t>(
        ::std::chrono::system_clock::now().time_since_epoch().count()
    ) };
    this->serverSendHandshake(m_cipher.encrypt(&baseValue, sizeof(::std::uint64_t)));
    this->serverReadHandshake(
        baseValue,
        new ::std::array<::std::byte, ::security::Cipher::getEncryptedSize(sizeof(::std::uint64_t))>
    );
}

template <
    ::detail::isEnum MessageType
> void ::network::Connection<MessageType>::serverSendHandshake(
    ::std::vector<::std::byte>&& encryptedBaseValue
)
{
    this->tcpSendRawData<
        [](...){},
        [](::network::Connection<MessageType>& self, const boost::system::error_code& errorCode){
            ::std::cerr << "[ERROR:" << self.m_id << "] Write handshake failed: " << errorCode.message() << ".\n";
            self.disconnect();
        }
    >(encryptedBaseValue.data(), encryptedBaseValue.size());
}

template <
    ::detail::isEnum MessageType
> void ::network::Connection<MessageType>::serverReadHandshake(
    ::std::uint64_t& baseValue,
    ::std::array<
        ::std::byte,
        ::security::Cipher::getEncryptedSize(sizeof(::std::uint64_t))
    >* receivedValue
)
{
    ::boost::asio::async_read(
        m_tcpSocket,
        ::boost::asio::buffer(receivedValue->data(), receivedValue->size()),
        [
            this,
            baseValue,
            receivedValue
        ](
            const boost::system::error_code& errorCode,
            const ::std::size_t length
        ) {
            if (errorCode) {
                ::std::cerr << "[ERROR:" << m_id << "] Read handshake failed: " << errorCode.message() << ".\n";
                this->disconnect();
            } else {
                auto decryptedValue{
                    *reinterpret_cast<::std::uint64_t*>(
                        m_cipher.decrypt(receivedValue->data(), receivedValue->size()).data()
                    )
                };
                if (decryptedValue == m_cipher.scramble(baseValue)) {
                    if (
                        dynamic_cast<::network::AServer<MessageType>&>(m_owner)
                            .onClientIdentificate(this->shared_from_this())
                    ) {
                        this->sendIdentificationAcceptanceHeader();
                    } else {
                        this->disconnect();
                    }
                } else {
                    ::std::cerr << "[ERROR:" << m_id << "] Handshake failed, incorrect value\n";
                    m_owner.onIdentificationDenial(this->shared_from_this());
                    this->disconnect();
                }
            }
            delete receivedValue;
        }
    );
}

template <
    ::detail::isEnum MessageType
> void ::network::Connection<MessageType>::sendIdentificationAcceptanceHeader()
{

    this->tcpSendMessage<
        [](::network::Connection<MessageType>& self){
            self.m_isValid = true;
            ::std::cout << "[" << self.m_id << "] Identificated successfully.\n";
            dynamic_cast<::network::AServer<MessageType>&>(self.m_owner)
                .validateConnection(self.shared_from_this());
            self.startReadMessage();
        },
        [](::network::Connection<MessageType>& self, const boost::system::error_code& errorCode){
            ::std::cerr << "[ERROR:" << self.m_id << "] Send identificaion acceptance header failed: "
                << errorCode.message() << ".\n";
            self.disconnect();
        },
        [](::network::Connection<MessageType>& self, const boost::system::error_code& errorCode){
            ::std::cerr << "[ERROR:" << self.m_id << "] Send identificaion acceptance body failed: "
                << errorCode.message() << ".\n";
            self.disconnect();
        }
    >(
        ::network::Message<MessageType>{
            MessageType::identificationAccepted,
            ::network::TransmissionProtocol::tcp,
            ::std::uint16_t(m_udpSocket.local_endpoint().port())
        }
    );
}



template <
    ::detail::isEnum MessageType
> void ::network::Connection<MessageType>::clientHandshake()
{
    this->clientReadHandshake(
        new ::std::array<::std::byte, ::security::Cipher::getEncryptedSize(sizeof(::std::uint64_t))>
    );
}

template <
    ::detail::isEnum MessageType
> void ::network::Connection<MessageType>::clientReadHandshake(
    ::std::array<
        ::std::byte,
        ::security::Cipher::getEncryptedSize(sizeof(::std::uint64_t))
    >* receivedValue
)
{
    ::boost::asio::async_read(
        m_tcpSocket,
        ::boost::asio::buffer(receivedValue->data(), receivedValue->size()),
        [
            this,
            receivedValue
        ](
            const boost::system::error_code& errorCode,
            const ::std::size_t length
        ) {
            if (errorCode) {
                ::std::cerr << "[ERROR:" << m_id << "] Read handshake failed: " << errorCode.message() << ".\n";
                this->disconnect();
            } else {
                this->clientResolveHandshake(receivedValue);
            }
        }
    );
}

template <
    ::detail::isEnum MessageType
> void ::network::Connection<MessageType>::clientResolveHandshake(
    ::std::array<
        ::std::byte,
        ::security::Cipher::getEncryptedSize(sizeof(::std::uint64_t))
    >* receivedValue
)
{
    auto handshakeReceived{ m_cipher.decrypt(receivedValue->data(), receivedValue->size()) };
    auto handshakeResolved{ m_cipher.scramble(*reinterpret_cast<::std::uint64_t*>(handshakeReceived.data())) };
    auto handshakeResolvedEncrypted{ m_cipher.encrypt(&handshakeResolved, sizeof(handshakeResolved)) };
    delete receivedValue;
    ::boost::asio::async_write(
        m_tcpSocket,
        ::boost::asio::buffer(handshakeResolvedEncrypted.data(), handshakeResolvedEncrypted.size()),
        [this](
            const boost::system::error_code& errorCode,
            const ::std::size_t length
        ) {
            if (errorCode) {
                ::std::cerr << "[ERROR:" << m_id << "] Write handshake failed: " << errorCode.message() << ".\n";
                this->disconnect();
            } else {
                this->clientWaitIdentificationAcceptanceHeader();
            }
        }
    );
}

// TODO: add timeout functionnalities
template <
    ::detail::isEnum MessageType
> void ::network::Connection<MessageType>::clientWaitIdentificationAcceptanceHeader()
{
    this->tcpReceiveMessage<
        [](::network::Connection<MessageType>& self){
            if (self.m_tcpBufferIn.getType() == MessageType::identificationAccepted) {
                // get udp port
                ::std::uint16_t udpPort;
                self.m_tcpBufferIn.extract(udpPort);
                self.targetServerUdpPort(udpPort);

                // start reading/writing tcp
                self.startReadMessage();
                if (!self.m_tcpMessagesOut.empty()) {
                    self.tcpWriteAwaitingMessages();
                }

                // start reading/writing udp
                self.udpReadHeader();
                if (!self.m_udpMessagesOut.empty()) {
                    self.udpWriteHeader();
                }

                self.m_isValid = true;
                ::std::cout << "[" << self.m_id << "] Identificaion accepted.\n";
            } else if (self.m_tcpBufferIn.getType() == MessageType::identificationDenied) {
                ::std::cerr << "[ERROR:" << self.m_id << "] Iidentificaion denied.\n";
                self.disconnect();
            } else {
                ::std::cerr << "[ERROR:" << self.m_id << "] Identificaion failed, "
                    << "unexpected message received.\n";
                self.disconnect();
            }
        },
        [](::network::Connection<MessageType>& self, const boost::system::error_code& errorCode){
            ::std::cerr << "[ERROR:" << self.m_id << "] Identificaion acceptance failed: "
                << errorCode.message() << ".\n";
            self.disconnect();
        },
        [](::network::Connection<MessageType>& self, const boost::system::error_code& errorCode){
            ::std::cerr << "[ERROR:" << self.m_id << "] Iidentificaion acceptance failed: "
                << errorCode.message() << ".\n";
            self.disconnect();
        }
    >();

    // ::boost::asio::async_read(
        // m_tcpSocket,
        // ::boost::asio::buffer(m_tcpBufferIn.getHeaderAddr(), m_tcpBufferIn.getHeaderSize()),
        // [this](
            // const boost::system::error_code& errorCode,
            // const ::std::size_t length
        // ) {
            // if (errorCode) {
                // ::std::cerr << "[ERROR:" << m_id << "] Iidentificaion acceptance failed: "
                    // << errorCode.message() << ".\n";
                // this->disconnect();
            // } else {
                // if (m_tcpBufferIn.getType() == MessageType::identificationAccepted) {
                    // this->clientWaitIdentificationAcceptanceBody();
                // } else if (m_tcpBufferIn.getType() == MessageType::identificationDenied) {
                    // ::std::cerr << "[ERROR:" << m_id << "] Iidentificaion denied.\n";
                    // this->disconnect();
                // } else {
                    // ::std::cerr << "[ERROR:" << m_id << "] Identificaion failed, "
                        // << "unexpected message received.\n";
                    // this->disconnect();
                // }
            // }
        // }
    // );
}

template <
    ::detail::isEnum MessageType
> void ::network::Connection<MessageType>::clientWaitIdentificationAcceptanceBody()
{
    m_tcpBufferIn.resize(sizeof(::std::uint16_t));
    ::boost::asio::async_read(
        m_tcpSocket,
        ::boost::asio::buffer(m_tcpBufferIn.getBodyAddr(), m_tcpBufferIn.getBodySize()),
        [this](
            const boost::system::error_code& errorCode,
            const ::std::size_t length
        ) {
            if (errorCode) {
                ::std::cerr << "[ERROR:" << m_id << "] Identificaion acceptance failed: "
                    << errorCode.message() << ".\n";
                this->disconnect();
            } else {
                this->startReadMessage();
                if (!m_tcpMessagesOut.empty()) {
                    this->tcpWriteAwaitingMessages();
                }

                ::std::uint16_t udpPort;
                m_tcpBufferIn >> udpPort;
                this->targetServerUdpPort(udpPort);
                this->udpReadHeader();
                if (!m_udpMessagesOut.empty()) {
                    this->udpWriteHeader();
                }

                m_isValid = true;
                ::std::cout << "[" << m_id << "] Identificaion accepted.\n";
            }
        }
    );
}
