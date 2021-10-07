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
    ::detail::IsEnum MessageType
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
    ::detail::IsEnum MessageType
> ::network::Connection<MessageType>::~Connection() = default;



// ------------------------------------------------------------------ async - connection

template <
    ::detail::IsEnum MessageType
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
    ::detail::IsEnum MessageType
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
    ::detail::IsEnum MessageType
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
                } else {
                    this->identificate();
                }
            }
        );
    } else {
        throw ::std::runtime_error("A server cannot target another server udp port.\n");
    }
}



template <
    ::detail::IsEnum MessageType
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
    ::detail::IsEnum MessageType
> auto ::network::Connection<MessageType>::isConnected() const
    -> bool
{
    return m_tcpSocket.is_open();
}




// ------------------------------------------------------------------ async - tcpIn

template <
    ::detail::IsEnum MessageType
> void ::network::Connection<MessageType>::tcpReadHeader()
{
    ::boost::asio::async_read(
        m_tcpSocket,
        ::boost::asio::buffer(m_bufferIn.getHeaderAddr(), m_bufferIn.getHeaderSize()),
        [this](
            const boost::system::error_code& errorCode,
            const ::std::size_t length
        ) {
            if (errorCode) {
                ::std::cerr << "[ERROR:" << m_id << "] Read header failed: " << errorCode.message() << ".\n";
                if (errorCode == ::boost::asio::error::eof) {
                }
                this->disconnect();
            } else {
                if (!m_bufferIn.isBodyEmpty()) {
                    m_bufferIn.updateBodySize();
                    this->tcpReadBody();
                } else {
                    this->tcpTransferBufferToInQueue();
                }
            }
        }
    );
}

template <
    ::detail::IsEnum MessageType
> void ::network::Connection<MessageType>::tcpReadBody()
{
    ::boost::asio::async_read(
        m_tcpSocket,
        ::boost::asio::buffer(m_bufferIn.getBodyAddr(), m_bufferIn.getBodySize()),
        [this](
            const boost::system::error_code& errorCode,
            const ::std::size_t length
        ) {
            if (errorCode) {
                ::std::cerr << "[ERROR:" << m_id << "] Read body failed: " << errorCode.message() << ".\n";
                this->disconnect();
            } else {
                this->tcpTransferBufferToInQueue();
            }
        }
    );
}

template <
    ::detail::IsEnum MessageType
> void ::network::Connection<MessageType>::tcpTransferBufferToInQueue()
{
    if (m_owner.getType() == ::network::ANode<MessageType>::Type::server) {
        m_owner.pushIncommingMessage(
            network::OwnedMessage<MessageType>{ m_bufferIn, this->shared_from_this() }
        );
    } else {
        m_owner.pushIncommingMessage(network::OwnedMessage<MessageType>{ m_bufferIn, nullptr });
    }
    this->tcpReadHeader();
}



// ------------------------------------------------------------------ async - tcpOut

template <
    ::detail::IsEnum MessageType
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
            if (wasOutQueueEmpty) {
                this->tcpWriteHeader();
            }
        }
    );
}

template <
    ::detail::IsEnum MessageType
> void ::network::Connection<MessageType>::tcpWriteHeader()
{
    ::boost::asio::async_write(
        m_tcpSocket,
        ::boost::asio::buffer(
            m_tcpMessagesOut.front().getHeaderAddr(),
            m_tcpMessagesOut.front().getHeaderSize()
        ),
        [this](
            const boost::system::error_code& errorCode,
            const ::std::size_t length
        ) {
            if (errorCode) {
                ::std::cerr << "[ERROR:" << m_id << "] Write header failed: " << errorCode.message() << ".\n";
                this->disconnect();
            } else {
                if (!m_tcpMessagesOut.front().isBodyEmpty()) {
                    this->tcpWriteBody();
                } else {
                    m_tcpMessagesOut.remove_front();
                    if (!m_tcpMessagesOut.empty()) {
                        this->tcpWriteHeader();
                    }
                }
            }
        }
    );
}

template <
    ::detail::IsEnum MessageType
> void ::network::Connection<MessageType>::tcpWriteBody()
{
    ::boost::asio::async_write(
        m_tcpSocket,
        ::boost::asio::buffer(
            m_tcpMessagesOut.front().getBodyAddr(),
            m_tcpMessagesOut.front().getBodySize()
        ),
        [this](
            const boost::system::error_code& errorCode,
            const ::std::size_t length
        ) {
            if (errorCode) {
                ::std::cerr << "[ERROR:" << m_id << "] Write body failed: " << errorCode.message() << ".\n";
                this->disconnect();
            } else {
                m_tcpMessagesOut.remove_front();
                if (!m_tcpMessagesOut.empty()) {
                    this->tcpWriteHeader();
                }
            }
        }
    );
}




// ------------------------------------------------------------------ async - udpIn

template <
    ::detail::IsEnum MessageType
> void ::network::Connection<MessageType>::udpReadHeader(
    ::std::size_t bytesAlreadyRead /* = 0 */
)
{
    m_udpSocket.async_receive(
        ::boost::asio::buffer(
            m_bufferIn.getHeaderAddr() + bytesAlreadyRead,
            m_bufferIn.getHeaderSize() - bytesAlreadyRead
        ),
        [this, bytesAlreadyRead](
            const boost::system::error_code& errorCode,
            const ::std::size_t length
        ) {
            if (errorCode) {
                ::std::cerr << "[ERROR:" << m_id << "] Read header failed: " << errorCode.message() << ".\n";
                this->disconnect();
            } else if (bytesAlreadyRead + length < m_bufferIn.getHeaderSize()) {
                this->udpReadHeader(bytesAlreadyRead + length);
            } else {
                if (!m_bufferIn.isBodyEmpty()) {
                    m_bufferIn.updateBodySize();
                    this->udpReadBody();
                } else {
                    this->udpTransferBufferToInQueue();
                }
            }
        }
    );
}

template <
    ::detail::IsEnum MessageType
> void ::network::Connection<MessageType>::udpReadBody(
    ::std::size_t bytesAlreadyRead /* = 0 */
)
{
    m_udpSocket.async_receive(
        ::boost::asio::buffer(
            m_bufferIn.getBodyAddr() + bytesAlreadyRead,
            m_bufferIn.getBodySize() - bytesAlreadyRead
        ),
        [this, bytesAlreadyRead](
            const boost::system::error_code& errorCode,
            const ::std::size_t length
        ) {
            if (errorCode) {
                ::std::cerr << "[ERROR:" << m_id << "] Read body failed: " << errorCode.message() << ".\n";
                this->disconnect();
            } else if (bytesAlreadyRead + length < m_bufferIn.getBodySize()) {
                this->udpReadBody(bytesAlreadyRead + length);
            } else {
                this->udpTransferBufferToInQueue();
            }
        }
    );
}

template <
    ::detail::IsEnum MessageType
> void ::network::Connection<MessageType>::udpTransferBufferToInQueue()
{
    if (m_owner.getType() == ::network::ANode<MessageType>::Type::server) {
        m_owner.pushIncommingMessage(
            network::OwnedMessage<MessageType>{ m_bufferIn, this->shared_from_this() }
        );
    } else {
        m_owner.pushIncommingMessage(network::OwnedMessage<MessageType>{ m_bufferIn, nullptr });
    }
    this->udpReadHeader();
}



// ------------------------------------------------------------------ async - udpOut

template <
    ::detail::IsEnum MessageType
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
            if (wasOutQueueEmpty) {
                this->udpWriteHeader();
            }
        }
    );
}

template <
    ::detail::IsEnum MessageType
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
    ::detail::IsEnum MessageType
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



// ------------------------------------------------------------------ other

template <
    ::detail::IsEnum MessageType
> auto ::network::Connection<MessageType>::getId() const
    -> ::detail::Id
{
    return m_id;
}



// ------------------------------------------------------------------ async - securityProtocol

template <
    ::detail::IsEnum MessageType
> void ::network::Connection<MessageType>::identificate()
{
    this->sendPublicKey();
    this->readPublicKey();
}



template <
    ::detail::IsEnum MessageType
> void ::network::Connection<MessageType>::sendPublicKey()
{
    ::boost::asio::async_write(
        m_tcpSocket,
        ::boost::asio::buffer(m_cipher.getPublicKeyAddr(), m_cipher.getPublicKeySize()),
        [this](
            const boost::system::error_code& errorCode,
            const ::std::size_t length
        ) {
            if (errorCode) {
                ::std::cerr << "[ERROR:" << m_id << "] Write header failed: " << errorCode.message() << ".\n";
                this->disconnect();
            }
        }
    );
}

template <
    ::detail::IsEnum MessageType
> void ::network::Connection<MessageType>::readPublicKey()
{
    ::boost::asio::async_read(
        m_tcpSocket,
        ::boost::asio::buffer(m_cipher.getTargetPublicKeyAddr(), m_cipher.getPublicKeySize()),
        [this](
            const boost::system::error_code& errorCode,
            const ::std::size_t length
        ) {
            if (errorCode) {
                ::std::cerr << "[ERROR:" << m_id << "] Read public key failed: " << errorCode.message() << ".\n";
                this->disconnect();
            } else {
                if (m_owner.getType() == ::network::ANode<MessageType>::Type::server) {
                    return this->serverHandshake();
                } else {
                    return this->clientHandshake();
                }
                // no return means error
                m_owner.onIdentificationDenial(this->shared_from_this());
                this->disconnect();
            }
        }
    );
}



template <
    ::detail::IsEnum MessageType
> void ::network::Connection<MessageType>::serverHandshake()
{
    auto handshakeBaseValue{ static_cast<::std::uint64_t>(
        ::std::chrono::system_clock::now().time_since_epoch().count()
    ) };
    this->serverSendHandshake(m_cipher.encrypt(&handshakeBaseValue, sizeof(::std::uint64_t)));
    this->serverReadHandshake(
        handshakeBaseValue,
        new ::std::array<::std::byte, ::security::Cipher::getEncryptedSize(sizeof(::std::uint64_t))>
    );
}

template <
    ::detail::IsEnum MessageType
> void ::network::Connection<MessageType>::serverSendHandshake(
    ::std::vector<::std::byte>&& encryptedHandshakeBaseValue
)
{
    ::boost::asio::async_write(
        m_tcpSocket,
        ::boost::asio::buffer(encryptedHandshakeBaseValue.data(), encryptedHandshakeBaseValue.size()),
        [this](
            const boost::system::error_code& errorCode,
            const ::std::size_t length
        ) {
            if (errorCode) {
                ::std::cerr << "[ERROR:" << m_id << "] Write handshake failed: " << errorCode.message() << ".\n";
                this->disconnect();
            }
        }
    );
}

template <
    ::detail::IsEnum MessageType
> void ::network::Connection<MessageType>::serverReadHandshake(
    ::std::uint64_t& handshakeBaseValue,
    ::std::array<
        ::std::byte,
        ::security::Cipher::getEncryptedSize(sizeof(::std::uint64_t))
    >* handshakeReceivedPtr
)
{
    ::boost::asio::async_read(
        m_tcpSocket,
        ::boost::asio::buffer(handshakeReceivedPtr->data(), handshakeReceivedPtr->size()),
        [
            this,
            handshakeBaseValue,
            handshakeReceivedPtr
        ](
            const boost::system::error_code& errorCode,
            const ::std::size_t length
        ) {
            if (errorCode) {
                ::std::cerr << "[ERROR:" << m_id << "] Read handshake failed: " << errorCode.message() << ".\n";
                this->disconnect();
            } else {
                auto decrypted{
                    m_cipher.decrypt(handshakeReceivedPtr->data(), handshakeReceivedPtr->size())
                };
                if (
                    *reinterpret_cast<::std::uint64_t*>(decrypted.data()) ==
                    m_cipher.scramble(handshakeBaseValue)
                ) {
                    auto& server{ dynamic_cast<::network::AServer<MessageType>&>(m_owner) };
                    if (server.onClientIdentificate(this->shared_from_this())) {
                        ::std::cout << "[" << m_id << "] Identificated successfully\n";
                        // send the udp port
                        this->tcpSend(
                            MessageType::udpPort,
                            ::std::uint16_t(m_udpSocket.local_endpoint().port())
                        );
                        server.validateConnection(this->shared_from_this());
                        this->tcpReadHeader();
                    } else {
                        this->disconnect();
                    }
                } else {
                    ::std::cerr << "[ERROR:" << m_id << "] Handshake failed, incorrect value\n";
                    m_owner.onIdentificationDenial(this->shared_from_this());
                    this->disconnect();
                }
                delete handshakeReceivedPtr;
            }
        }
    );
}



template <
    ::detail::IsEnum MessageType
> void ::network::Connection<MessageType>::clientHandshake()
{
    this->clientReadHandshake(
        new ::std::array<::std::byte, ::security::Cipher::getEncryptedSize(sizeof(::std::uint64_t))>
    );
}

template <
    ::detail::IsEnum MessageType
> void ::network::Connection<MessageType>::clientReadHandshake(
    ::std::array<
        ::std::byte,
        ::security::Cipher::getEncryptedSize(sizeof(::std::uint64_t))
    >* handshakeReceivedPtr
)
{
    ::boost::asio::async_read(
        m_tcpSocket,
        ::boost::asio::buffer(handshakeReceivedPtr->data(), handshakeReceivedPtr->size()),
        [
            this,
            handshakeReceivedPtr
        ](
            const boost::system::error_code& errorCode,
            const ::std::size_t length
        ) {
            if (errorCode) {
                ::std::cerr << "[ERROR:" << m_id << "] Read handshake failed: " << errorCode.message() << ".\n";
                this->disconnect();
            } else {
                this->clientResolveHandshake(handshakeReceivedPtr);
            }
        }
    );
}

template <
    ::detail::IsEnum MessageType
> void ::network::Connection<MessageType>::clientResolveHandshake(
    ::std::array<
        ::std::byte,
        ::security::Cipher::getEncryptedSize(sizeof(::std::uint64_t))
    >* handshakeReceivedPtr
)
{
    auto handshakeReceived{ m_cipher.decrypt(handshakeReceivedPtr->data(), handshakeReceivedPtr->size()) };
    auto handshakeResolved{ m_cipher.scramble(*reinterpret_cast<::std::uint64_t*>(handshakeReceived.data())) };
    auto handshakeResolvedEncrypted{ m_cipher.encrypt(&handshakeResolved, sizeof(handshakeResolved)) };
    delete handshakeReceivedPtr;
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
                this->tcpReadHeader();
            }
        }
    );
}
