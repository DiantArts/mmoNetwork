#include <pch.hpp>
#include <Connection.hpp>
#include <Message.hpp>
#include <OwnedMessage.hpp>
#include <MessageType.hpp>
#include <ANode.hpp>
#include <Server/AServer.hpp>


// ------------------------------------------------------------------ explicit instantiations

template class ::network::Connection<::network::MessageType>;



// ------------------------------------------------------------------ *structors

template <
    ::network::detail::IsEnum MessageType
> ::network::Connection<MessageType>::Connection(
    ::boost::asio::ip::tcp::socket socket,
    ::network::ANode<MessageType>& connectionOwner
)
    : m_owner{ connectionOwner }
    , m_socket{ ::std::move(socket) }
{}


template <
    ::network::detail::IsEnum MessageType
> ::network::Connection<MessageType>::~Connection() = default;



// ------------------------------------------------------------------ async - connection

template <
    ::network::detail::IsEnum MessageType
> auto ::network::Connection<MessageType>::connectToClient(
    ::network::Id id
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
    ::network::detail::IsEnum MessageType
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
    ::network::detail::IsEnum MessageType
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
    ::network::detail::IsEnum MessageType
> auto ::network::Connection<MessageType>::isConnected() const
    -> bool
{
    return m_socket.is_open();
}




// ------------------------------------------------------------------ async - in

template <
    ::network::detail::IsEnum MessageType
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
    ::network::detail::IsEnum MessageType
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
    ::network::detail::IsEnum MessageType
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
    ::network::detail::IsEnum MessageType
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

// TODO: client memory error
template <
    ::network::detail::IsEnum MessageType
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
    ::network::detail::IsEnum MessageType
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
    ::network::detail::IsEnum MessageType
> [[ nodiscard ]] auto ::network::Connection<MessageType>::getId() const
    -> ::network::Id
{
    return m_id;
}



// ------------------------------------------------------------------ async - securityProtocol

template <
    ::network::detail::IsEnum MessageType
> void ::network::Connection<MessageType>::identificate()
{
    this->sendPublicKey();
    this->readPublicKey();
}



template <
    ::network::detail::IsEnum MessageType
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
    ::network::detail::IsEnum MessageType
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
                    if (m_cipher.checkClientPublicKey()) {
                        return this->serverHandshake();
                    }
                } else {
                    if (m_cipher.checkClientPublicKey()) {
                        return this->clientHandshake();
                    }
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
    ::network::detail::IsEnum MessageType
> void ::network::Connection<MessageType>::serverHandshake()
{
    auto handshakeBaseValue{ static_cast<::std::uint64_t>(
        ::std::chrono::system_clock::now().time_since_epoch().count()
    ) };
    this->serverSendHandshake(handshakeBaseValue);
    this->serverReadHandshake(handshakeBaseValue, new ::std::uint64_t);
}

template <
    ::network::detail::IsEnum MessageType
> void ::network::Connection<MessageType>::serverSendHandshake(
    ::std::uint64_t& handshakeValue
)
{
    ::boost::asio::async_write(
        m_socket,
        ::boost::asio::buffer(&handshakeValue, sizeof(::std::uint64_t)),
        [&](
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
    ::network::detail::IsEnum MessageType
> void ::network::Connection<MessageType>::serverReadHandshake(
    ::std::uint64_t& handshakeBaseValue,
    ::std::uint64_t* handshakeReceivedPtr
)
{
    ::boost::asio::async_read(
        m_socket,
        ::boost::asio::buffer(handshakeReceivedPtr, sizeof(::std::uint64_t)),
        [
            this,
            handshakeBaseValue,
            handshakeReceivedPtr
        ](
            const boost::system::error_code& errorCode,
            const ::std::size_t length
        ) {
            if (!errorCode) {
                if (*handshakeReceivedPtr == m_cipher.scramble(handshakeBaseValue)) {
                    if (
                        dynamic_cast<::network::AServer<MessageType>&>(m_owner)
                            .onClientIdentificate(this->shared_from_this())
                    ) {
                        ::std::cout << "[" << m_id << "] Identificated successfully\n";
                        this->readHeader();
                    } else {
                        m_owner.onIdentificationDenial(this->shared_from_this());
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
    ::network::detail::IsEnum MessageType
> void ::network::Connection<MessageType>::clientHandshake()
{
    this->clientReadHandshake(new ::std::uint64_t);
}

template <
    ::network::detail::IsEnum MessageType
> void ::network::Connection<MessageType>::clientReadHandshake(
    ::std::uint64_t* handshakeReceivedPtr
)
{
    ::boost::asio::async_read(
        m_socket,
        ::boost::asio::buffer(handshakeReceivedPtr, sizeof(::std::uint64_t)),
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
    ::network::detail::IsEnum MessageType
> void ::network::Connection<MessageType>::clientResolveHandshake(
    ::std::uint64_t* handshakeReceivedPtr
)
{
    *handshakeReceivedPtr = m_cipher.scramble(*handshakeReceivedPtr);
    ::boost::asio::async_write(
        m_socket,
        ::boost::asio::buffer(handshakeReceivedPtr, sizeof(::std::uint64_t)),
        [
            this,
            handshakeReceivedPtr
        ](
            const boost::system::error_code& errorCode,
            const ::std::size_t length
        ) {
            if (!errorCode) {
                this->readHeader();
                delete handshakeReceivedPtr;
            } else {
                ::std::cerr << "[" << m_id << "] Write handshake failed: " << errorCode.message() << ".\n";
                this->disconnect();
            }
        }
    );
}
