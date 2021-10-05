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
    ::boost::asio::ip::tcp::socket socket,
    ::network::ANode<MessageType>& connectionOwner
)
    : m_owner{ connectionOwner }
    , m_socket{ ::std::move(socket) }
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
            ::std::cerr << "Invalid socket.\n";
            return false;
        }
    } else {
        ::std::cerr << "A client cannot connect to another client.\n";
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
        // resolve host/ip addr into a physical addr
        ::boost::asio::ip::tcp::resolver resolver{ m_owner.getAsioContext() };
        auto endpoints = resolver.resolve(host, ::std::to_string(port));

        ::boost::asio::async_connect(
            m_socket,
            endpoints,
            [this](
                const boost::system::error_code& errorCode,
                const ::boost::asio::ip::tcp::endpoint
            ) {
                if (!errorCode) {
                    this->identificate();
                } else {
                    ::std::cerr << "Client failed to connect to the Server.\n";
                }
            }
        );
    } else {
        throw ::std::runtime_error("A server cannot connect to another server.\n");
    }
}

template <
    ::detail::IsEnum MessageType
> void ::network::Connection<MessageType>::disconnect()
{
    if (this->isConnected()) {
        ::boost::asio::post(
            m_owner.getAsioContext(),
            [this]()
            {
                m_socket.close();
                m_id = 0;
            }
        );
    }
}

template <
    ::detail::IsEnum MessageType
> auto ::network::Connection<MessageType>::isConnected() const
    -> bool
{
    return m_socket.is_open();
}




// ------------------------------------------------------------------ async - in

template <
    ::detail::IsEnum MessageType
> void ::network::Connection<MessageType>::readHeader()
{
    ::boost::asio::async_read(
        m_socket,
        ::boost::asio::buffer(m_bufferIn.getHeaderAddr(), m_bufferIn.getHeaderSize()),
        [this](
            const boost::system::error_code& errorCode,
            const ::std::size_t length
        ) {
            if (!errorCode) {
                // m_bufferIn.displayHeader("<-");
                if (!m_bufferIn.isBodyEmpty()) {
                    m_bufferIn.updateBodySize();
                    this->readBody();
                } else {
                    this->transferBufferToInQueue();
                }
            } else {
                ::std::cerr << "[" << m_id << "] Read header failed: "
                    << errorCode.message() << ".\n";
                this->disconnect();
            }
        }
    );
}

template <
    ::detail::IsEnum MessageType
> void ::network::Connection<MessageType>::readBody()
{
    ::boost::asio::async_read(
        m_socket,
        ::boost::asio::buffer(m_bufferIn.getBodyAddr(), m_bufferIn.getBodySize()),
        [this](
            const boost::system::error_code& errorCode,
            const ::std::size_t length
        ) {
            if (!errorCode) {
                // m_bufferIn.displayBody("<-");
                this->transferBufferToInQueue();
            } else {
                ::std::cerr << "[" << m_id << "] Read body failed: "
                    << errorCode.message() << ".\n";
                this->disconnect();
            }
        }
    );
}

template <
    ::detail::IsEnum MessageType
> void ::network::Connection<MessageType>::transferBufferToInQueue()
{
    if (m_owner.getType() == ::network::ANode<MessageType>::Type::server) {
        m_owner.pushIncommingMessage(
            network::OwnedMessage<MessageType>{ this->shared_from_this(), m_bufferIn }
        );
    } else {
        m_owner.pushIncommingMessage(network::OwnedMessage<MessageType>{ nullptr, m_bufferIn });
    }
    this->readHeader();
}



// ------------------------------------------------------------------ async - out

template <
    ::detail::IsEnum MessageType
> void ::network::Connection<MessageType>::send(
    ::network::Message<MessageType> message
)
{
    ::boost::asio::post(
        m_owner.getAsioContext(),
        [this, message]()
        {
            auto wasOutQueueEmpty{ m_messagesOut.empty() };
            m_messagesOut.push_back(::std::move(message));
            if (wasOutQueueEmpty) {
                this->writeHeader();
            }
        }
    );
}

template <
    ::detail::IsEnum MessageType
> void ::network::Connection<MessageType>::writeHeader()
{
    ::boost::asio::async_write(
        m_socket,
        ::boost::asio::buffer(m_messagesOut.front().getHeaderAddr(), m_messagesOut.front().getHeaderSize()),
        [this](
            const boost::system::error_code& errorCode,
            const ::std::size_t length
        ) {
            if (!errorCode) {
                // m_messagesOut.front().displayHeader("->");
                if (!m_messagesOut.front().isBodyEmpty()) {
                    this->writeBody();
                } else {
                    m_messagesOut.remove_front();
                    if (!m_messagesOut.empty()) {
                        this->writeHeader();
                    }
                }
            } else {
                ::std::cerr << "[" << m_id << "] Write header failed: "
                    << errorCode.message() << ".\n";
                this->disconnect();
            }
        }
    );
}

template <
    ::detail::IsEnum MessageType
> void ::network::Connection<MessageType>::writeBody()
{
    ::boost::asio::async_write(
        m_socket,
        ::boost::asio::buffer(m_messagesOut.front().getBodyAddr(), m_messagesOut.front().getBodySize()),
        [this](
            const boost::system::error_code& errorCode,
            const ::std::size_t length
        ) {
            if (!errorCode) {
                // m_messagesOut.front().displayBody("->");
                m_messagesOut.remove_front();
                if (!m_messagesOut.empty()) {
                    this->writeHeader();
                }
            } else {
                ::std::cerr << "[" << m_id << "] Write body failed: "
                    << errorCode.message() << ".\n";
                this->disconnect();
            }
        }
    );
}



// ------------------------------------------------------------------ other

template <
    ::detail::IsEnum MessageType
> [[ nodiscard ]] auto ::network::Connection<MessageType>::getId() const
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
        m_socket,
        ::boost::asio::buffer(m_cipher.getPublicKeyAddr(), m_cipher.getPublicKeySize()),
        [this](
            const boost::system::error_code& errorCode,
            const ::std::size_t length
        ) {
            if (errorCode) {
                ::std::cerr << "[" << m_id << "] Write header failed: " << errorCode.message() << ".\n";
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
        m_socket,
        ::boost::asio::buffer(m_cipher.getTargetPublicKeyAddr(), m_cipher.getPublicKeySize()),
        [this](
            const boost::system::error_code& errorCode,
            const ::std::size_t length
        ) {
            if (!errorCode) {
                if (m_owner.getType() == ::network::ANode<MessageType>::Type::server) {
                    return this->serverHandshake();
                } else {
                    return this->clientHandshake();
                }
                // no return means error
                m_owner.onIdentificationDenial(this->shared_from_this());
                this->disconnect();
            } else {
                ::std::cerr << "[" << m_id << "] Read public key failed: " << errorCode.message() << ".\n";
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
        m_socket,
        ::boost::asio::buffer(encryptedHandshakeBaseValue.data(), encryptedHandshakeBaseValue.size()),
        [this](
            const boost::system::error_code& errorCode,
            const ::std::size_t length
        ) {
            if (errorCode) {
                ::std::cerr << "[" << m_id << "] Write handshake failed: " << errorCode.message() << ".\n";
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
        m_socket,
        ::boost::asio::buffer(handshakeReceivedPtr->data(), handshakeReceivedPtr->size()),
        [
            this,
            handshakeBaseValue,
            handshakeReceivedPtr
        ](
            const boost::system::error_code& errorCode,
            const ::std::size_t length
        ) {
            if (!errorCode) {
                auto decrypted{
                    m_cipher.decrypt(handshakeReceivedPtr->data(), handshakeReceivedPtr->size())
                };
                if (
                    *reinterpret_cast<::std::uint64_t*>(decrypted.data()) ==
                    m_cipher.scramble(handshakeBaseValue)
                ) {
                    if (
                        dynamic_cast<::network::AServer<MessageType>&>(m_owner)
                            .onClientIdentificate(this->shared_from_this())
                    ) {
                        ::std::cout << "[" << m_id << "] Identificated successfully\n";
                        this->readHeader();
                    } else {
                        this->disconnect();
                    }
                } else {
                    ::std::cerr << "[" << m_id << "] Hand shake failed\n";
                    m_owner.onIdentificationDenial(this->shared_from_this());
                    this->disconnect();
                }
                delete handshakeReceivedPtr;
            } else {
                ::std::cerr << "[" << m_id << "] Read handshake failed: " << errorCode.message() << ".\n";
                this->disconnect();
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
        m_socket,
        ::boost::asio::buffer(handshakeReceivedPtr->data(), handshakeReceivedPtr->size()),
        [
            this,
            handshakeReceivedPtr
        ](
            const boost::system::error_code& errorCode,
            const ::std::size_t length
        ) {
            if (!errorCode) {
                this->clientResolveHandshake(handshakeReceivedPtr);
            } else {
                ::std::cerr << "[" << m_id << "] Read handshake failed: " << errorCode.message() << ".\n";
                this->disconnect();
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
        m_socket,
        ::boost::asio::buffer(handshakeResolvedEncrypted.data(), handshakeResolvedEncrypted.size()),
        [this](
            const boost::system::error_code& errorCode,
            const ::std::size_t length
        ) {
            if (!errorCode) {
                this->readHeader();
            } else {
                ::std::cerr << "[" << m_id << "] Write handshake failed: " << errorCode.message() << ".\n";
                this->disconnect();
            }
        }
    );
}
