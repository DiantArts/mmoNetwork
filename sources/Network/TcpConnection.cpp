#include <pch.hpp>
#include <Network/TcpConnection.hpp>
#include <Network/Message.hpp>
#include <Network/OwnedMessage.hpp>
#include <Network/MessageType.hpp>
#include <Network/ANode.hpp>
#include <Network/Server/AServer.hpp>


// ------------------------------------------------------------------ explicit instantiations

template class ::network::TcpConnection<::network::MessageType>;



// ------------------------------------------------------------------ *structors

template <
    ::detail::isEnum MessageType
> ::network::TcpConnection<MessageType>::TcpConnection(
    ::network::ANode<MessageType>& owner,
    ::asio::ip::tcp::socket socket
)
    : m_owner{ owner }
    , m_socket{ ::std::move(socket) }
{}


template <
    ::detail::isEnum MessageType
> ::network::TcpConnection<MessageType>::~TcpConnection()
{
    if (this->isConnected()) {
        m_socket.cancel();
        m_socket.close();
        ::std::cout << "[Connection:TCP:" << m_id << "] Disconnected.\n";
    }
}



// ------------------------------------------------------------------ async - connection

template <
    ::detail::isEnum MessageType
> auto ::network::TcpConnection<MessageType>::connectToClient(
    ::detail::Id id
)
    -> bool
{
    if (m_owner.getType() == ::network::ANode<MessageType>::Type::server) {
        if (m_socket.is_open()) {
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
        ::std::cerr << "[ERROR:TcpConnection] A client cannot connect to another client.\n";
        return false;
    }
}

template <
    ::detail::isEnum MessageType
> void ::network::TcpConnection<MessageType>::connect(
    const ::std::string& host,
    const ::std::uint16_t port
)
{
    if (m_owner.getType() == ::network::ANode<MessageType>::Type::client) {
        m_socket.async_connect(
            ::asio::ip::tcp::endpoint{ ::asio::ip::address::from_string(host), port },
            [this](
                const ::std::error_code& errorCode
            ) {
                if (errorCode) {
                    if (errorCode == ::asio::error::operation_aborted) {
                        ::std::cerr << "[Connection:TCP] Operation canceled\n";
                    } else {
                        ::std::cerr << "[ERROR:TCP] Client failed to connect to the Server.\n";
                        this->disconnect();
                    }
                } else {
                    ::std::cout << "[Connection:TCP] Connection accepted." << ::std::endl;
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
> void ::network::TcpConnection<MessageType>::disconnect()
{
    if (this->isConnected()) {
        m_socket.cancel();
        m_socket.close();
        auto id{ m_id };
        m_owner.onDisconnect(this->shared_from_this());
        ::std::cout << "[Connection:TCP:" << id << "] Disconnected.\n";
    }
}

template <
    ::detail::isEnum MessageType
> auto ::network::TcpConnection<MessageType>::isConnected() const
    -> bool
{
    return m_socket.is_open();
}



// ------------------------------------------------------------------ async - out

template <
    ::detail::isEnum MessageType
> void ::network::TcpConnection<MessageType>::send(
    ::network::Message<MessageType> message
)
{
    // TODO ::std::move(message)
    ::asio::post(m_owner.getAsioContext(), ::std::bind_front(
        [this](
            ::network::Message<MessageType> message
        )
        {
            auto wasOutQueueEmpty{ m_messagesOut.empty() };
            message.setTransmissionProtocol(::network::TransmissionProtocol::tcp);
            m_messagesOut.push_back(::std::move(message));
            if (m_isValid && wasOutQueueEmpty) {
                this->writeAwaitingMessages();
            }
        },
        ::std::move(message)
    ));
}



// ------------------------------------------------------------------ other

template <
    ::detail::isEnum MessageType
> auto ::network::TcpConnection<MessageType>::getId() const
    -> ::detail::Id
{
    return m_id;
}

template <
    ::detail::isEnum MessageType
> auto ::network::TcpConnection<MessageType>::getOwner() const
    -> const ::network::ANode<MessageType>&
{
    return m_owner;
}

template <
    ::detail::isEnum MessageType
> auto ::network::TcpConnection<MessageType>::getPort() const
    -> ::std::uint16_t
{
    return m_socket.local_endpoint().port();
}

template <
    ::detail::isEnum MessageType
> auto ::network::TcpConnection<MessageType>::getAddress() const
    -> ::std::string
{
    return m_socket.local_endpoint().address().to_string();
}



// ------------------------------------------------------------------ async - out

template <
    ::detail::isEnum MessageType
> void ::network::TcpConnection<MessageType>::writeAwaitingMessages()
{

    this->sendMessage<
        [](::network::TcpConnection<MessageType>& self){
            self.m_messagesOut.remove_front();
            if (!self.m_messagesOut.empty()) {
                self.writeAwaitingMessages();
            }
        },
        [](::network::TcpConnection<MessageType>& self, const ::std::error_code& errorCode){
            ::std::cerr << "[ERROR:TCP:" << self.m_id << "] Write header failed: " << errorCode.message() << ".\n";
            self.disconnect();
        },
        [](::network::TcpConnection<MessageType>& self, const ::std::error_code& errorCode){
            ::std::cerr << "[ERROR:TCP:" << self.m_id << "] Write body failed: " << errorCode.message() << ".\n";
            self.disconnect();
        }
    >(m_messagesOut.front());
}



// ------------------------------------------------------------------ async - in

template <
    ::detail::isEnum MessageType
> void ::network::TcpConnection<MessageType>::startReadMessage()
{
    this->receiveMessage<
        [](::network::TcpConnection<MessageType>& self){
            self.transferBufferToInQueue();
            self.startReadMessage();
        },
        [](::network::TcpConnection<MessageType>& self, const ::std::error_code& errorCode){
            if (errorCode == ::asio::error::operation_aborted) {
                ::std::cerr << "[Connection:TCP] Operation canceled\n";
            } else if (errorCode == ::asio::error::eof) {
                ::std::cerr << "[Connection:TCP:" << self.m_id << "] Node stopped the connection.\n";
                self.disconnect();
            } else {
                ::std::cerr << "[ERROR:TCP:" << self.m_id << "] Read header failed: " << errorCode.message() << ".\n";
                self.disconnect();
            }
        },
        [](::network::TcpConnection<MessageType>& self, const ::std::error_code& errorCode){
            ::std::cerr << "[ERROR:TCP:" << self.m_id << "] Read body failed: " << errorCode.message() << ".\n";
            self.disconnect();
        }
    >();
}

template <
    ::detail::isEnum MessageType
> void ::network::TcpConnection<MessageType>::transferBufferToInQueue()
{
    if (m_owner.getType() == ::network::ANode<MessageType>::Type::server) {
        m_owner.pushIncommingMessage(
            network::OwnedMessage<MessageType>{ m_bufferIn, this->shared_from_this() }
        );
    } else {
        m_owner.pushIncommingMessage(network::OwnedMessage<MessageType>{ m_bufferIn, nullptr });
    }
}



// ------------------------------------------------------------------ async - securityProtocol

template <
    ::detail::isEnum MessageType
> void ::network::TcpConnection<MessageType>::identificate()
{
#if ENABLE_ENCRYPTION
    // send public key
    this->sendRawData<
        [](...){},
        [](::network::TcpConnection<MessageType>& self, const ::std::error_code& errorCode){
            ::std::cerr << "[ERROR:TCP:" << self.m_id << "] Write header failed: " << errorCode.message() << ".\n";
            self.disconnect();
        }
    >(m_cipher.getPublicKeyAddr(), m_cipher.getPublicKeySize());

    // read public key
    this->receiveToRawData<
        [](::network::TcpConnection<MessageType>& self){
            if (self.m_owner.getType() == ::network::ANode<MessageType>::Type::server) {
                self.serverHandshake();
            } else {
                self.clientHandshake();
            }
        },
        [](::network::TcpConnection<MessageType>& self, const ::std::error_code& errorCode){
            ::std::cerr << "[ERROR:TCP:" << self.m_id << "] Read public key failed: " << errorCode.message() << ".\n";
            self.disconnect();
        }
    >(m_cipher.getTargetPublicKeyAddr(), m_cipher.getPublicKeySize());
#else // ENABLE_ENCRYPTION
    if (m_owner.getType() == ::network::ANode<MessageType>::Type::server) {
        this->sendIdentificationAcceptance();
    } else {
        this->clientWaitIdentificationAcceptance();
    }
#endif // ENABLE_ENCRYPTION
}



#if ENABLE_ENCRYPTION



template <
    ::detail::isEnum MessageType
> void ::network::TcpConnection<MessageType>::serverHandshake()
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
> void ::network::TcpConnection<MessageType>::serverSendHandshake(
    ::std::vector<::std::byte>&& encryptedBaseValue
)
{
    this->sendRawData<
        [](...){},
        [](::network::TcpConnection<MessageType>& self, const ::std::error_code& errorCode){
            ::std::cerr << "[ERROR:TCP:" << self.m_id << "] Write handshake failed: " << errorCode.message() << ".\n";
            self.disconnect();
        }
    >(encryptedBaseValue.data(), encryptedBaseValue.size());
}

template <
    ::detail::isEnum MessageType
> void ::network::TcpConnection<MessageType>::serverReadHandshake(
    ::std::uint64_t& baseValue,
    ::std::array<
        ::std::byte,
        ::security::Cipher::getEncryptedSize(sizeof(::std::uint64_t))
    >* receivedValue
)
{
    ::asio::async_read(
        m_socket,
        ::asio::buffer(receivedValue->data(), receivedValue->size()),
        [
            this,
            baseValue,
            receivedValue
        ](
            const ::std::error_code& errorCode,
            const ::std::size_t length
        ) {
            if (errorCode) {
                if (errorCode == ::asio::error::operation_aborted) {
                    ::std::cerr << "[Connection:TCP] Operation canceled\n";
                } else {
                    ::std::cerr << "[ERROR:TCP:" << m_id << "] Read handshake failed: " << errorCode.message() << ".\n";
                    this->disconnect();
                }
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
                        this->sendIdentificationAcceptance();
                    } else {
                        this->disconnect();
                    }
                } else {
                    ::std::cerr << "[ERROR:TCP:" << m_id << "] Handshake failed, incorrect value\n";
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
> void ::network::TcpConnection<MessageType>::clientHandshake()
{
    this->clientReadHandshake(
        new ::std::array<::std::byte, ::security::Cipher::getEncryptedSize(sizeof(::std::uint64_t))>
    );
}

template <
    ::detail::isEnum MessageType
> void ::network::TcpConnection<MessageType>::clientReadHandshake(
    ::std::array<
        ::std::byte,
        ::security::Cipher::getEncryptedSize(sizeof(::std::uint64_t))
    >* receivedValue
)
{
    ::asio::async_read(
        m_socket,
        ::asio::buffer(receivedValue->data(), receivedValue->size()),
        [
            this,
            receivedValue
        ](
            const ::std::error_code& errorCode,
            const ::std::size_t length
        ) {
            if (errorCode) {
                if (errorCode == ::asio::error::operation_aborted) {
                    ::std::cerr << "[Connection:TCP] Operation canceled\n";
                } else {
                    ::std::cerr << "[ERROR:TCP:" << m_id << "] Read handshake failed: " << errorCode.message() << ".\n";
                    this->disconnect();
                }
            } else {
                this->clientResolveHandshake(receivedValue);
            }
        }
    );
}

template <
    ::detail::isEnum MessageType
> void ::network::TcpConnection<MessageType>::clientResolveHandshake(
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
    ::asio::async_write(
        m_socket,
        ::asio::buffer(handshakeResolvedEncrypted.data(), handshakeResolvedEncrypted.size()),
        [this](
            const ::std::error_code& errorCode,
            const ::std::size_t length
        ) {
            if (errorCode) {
                if (errorCode == ::asio::error::operation_aborted) {
                    ::std::cerr << "[Connection:TCP] Operation canceled\n";
                } else {
                    ::std::cerr << "[ERROR:TCP:" << m_id << "] Write handshake failed: " << errorCode.message() << ".\n";
                    this->disconnect();
                }
            } else {
                this->clientWaitIdentificationAcceptance();
            }
        }
    );
}



#endif // ENABLE_ENCRYPTION



template <
    ::detail::isEnum MessageType
> void ::network::TcpConnection<MessageType>::sendIdentificationAcceptance()
{

    this->sendMessage<
        [](::network::TcpConnection<MessageType>& self){
            self.m_isValid = true;
#if ENABLE_ENCRYPTION
            ::std::cout << "[Connection:TCP:" << self.m_id << "] Identificated successfully.\n";
#else // ENABLE_ENCRYPTION
            ::std::cout << "[Connection:TCP:" << self.m_id << "] Identificaion ignored.\n";
#endif // ENABLE_ENCRYPTION
            dynamic_cast<::network::AServer<MessageType>&>(self.m_owner)
                .validateConnection(self.shared_from_this());
            self.startReadMessage();
        },
        [](::network::TcpConnection<MessageType>& self, const ::std::error_code& errorCode){
            ::std::cerr << "[ERROR:TCP:" << self.m_id << "] Send identificaion acceptance header failed: "
                << errorCode.message() << ".\n";
            self.disconnect();
        },
        [](::network::TcpConnection<MessageType>& self, const ::std::error_code& errorCode){
            ::std::cerr << "[ERROR:TCP:" << self.m_id << "] Send identificaion acceptance body failed: "
                << errorCode.message() << ".\n";
            self.disconnect();
        }
    >(
        ::network::Message<MessageType>{
            MessageType::identificationAccepted,
            m_id
        }
    );
}

// TODO: add timeout functionnalities
template <
    ::detail::isEnum MessageType
> void ::network::TcpConnection<MessageType>::clientWaitIdentificationAcceptance()
{
    this->receiveMessage<
        [](::network::TcpConnection<MessageType>& self){
            if (self.m_bufferIn.getType() == MessageType::identificationAccepted) {
                self.m_bufferIn.extract(self.m_id);
                // start reading/writing tcp
                self.startReadMessage();
                if (!self.m_messagesOut.empty()) {
                    self.writeAwaitingMessages();
                }
                self.m_isValid = true;
#if ENABLE_ENCRYPTION
                ::std::cout << "[Connection:TCP:" << self.m_id << "] Identificaion accepted.\n";
#else // ENABLE_ENCRYPTION
                ::std::cout << "[Connection:TCP:" << self.m_id << "] Identificaion ignored.\n";
#endif // ENABLE_ENCRYPTION
            } else if (self.m_bufferIn.getType() == MessageType::identificationDenied) {
                ::std::cerr << "[ERROR:TCP:" << self.m_id << "] Iidentificaion denied.\n";
                self.disconnect();
            } else {
                ::std::cerr << "[ERROR:TCP:" << self.m_id << "] Identificaion failed, "
                    << "unexpected message received.\n";
                self.disconnect();
            }
        },
        [](::network::TcpConnection<MessageType>& self, const ::std::error_code& errorCode){
            ::std::cerr << "[ERROR:TCP:" << self.m_id << "] Identificaion acceptance failed: "
                << errorCode.message() << ".\n";
            self.disconnect();
        },
        [](::network::TcpConnection<MessageType>& self, const ::std::error_code& errorCode){
            ::std::cerr << "[ERROR:TCP:" << self.m_id << "] Iidentificaion acceptance failed: "
                << errorCode.message() << ".\n";
            self.disconnect();
        }
    >();
}
