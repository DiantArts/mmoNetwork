#include <pch.hpp>
#include <Network/Server/AServer.hpp>
#include <Network/MessageType.hpp>



// ------------------------------------------------------------------ explicit instantiations

template class ::network::AServer<::network::MessageType>;



// ------------------------------------------------------------------ *structors

template <
    ::detail::isEnum MessageType
> ::network::AServer<MessageType>::AServer(
    const ::std::uint16_t port
)
    : ::network::ANode<MessageType>{ ::network::ANode<MessageType>::Type::server }
    , m_asioAcceptor{
        this->getAsioContext(),
        ::asio::ip::tcp::endpoint{ ::asio::ip::tcp::v4(), port }
    }
{
    ::std::cout << "[Server] Ready to listen port " << port << ".\n";
}

template <
    ::detail::isEnum MessageType
> ::network::AServer<MessageType>::~AServer()
{
    this->stop();
}



// ------------------------------------------------------------------ running

template <
    ::detail::isEnum MessageType
> auto ::network::AServer<MessageType>::start()
    -> bool
{
    try {
        this->startReceivingConnections();
        this->getThreadContext() = ::std::thread([this](){ this->getAsioContext().run(); });
    } catch (::std::exception& e) {
        ::std::cerr << "[ERROR:Server] " << e.what() << ".\n";
        return false;
    }
    ::std::cout << "[Server] Started" << '\n';
    return true;
}

template <
    ::detail::isEnum MessageType
> void ::network::AServer<MessageType>::stop()
{
    if (this->isRunning()) {
        this->stopThread();
        this->getIncommingMessages().notify();
        ::std::cout << "[Server] Stopped" << '\n';
    }
}

template <
    ::detail::isEnum MessageType
> auto ::network::AServer<MessageType>::isRunning()
    -> bool
{
    // TODO: isRunning implemetation
    return !this->getAsioContext().stopped();
}



// ------------------------------------------------------------------ in - async

template <
    ::detail::isEnum MessageType
> void ::network::AServer<MessageType>::startReceivingConnections()
{
    m_asioAcceptor.async_accept(
        [this](
            const ::std::error_code& errorCode,
            ::asio::ip::tcp::socket socket
        ) {
            if (!errorCode) {
                ::std::cout << "[Server] New incomming connection: " << socket.remote_endpoint() << ".\n";
                auto newConnection{ ::std::make_shared<::network::TcpConnection<MessageType>>(
                    *this,
                    ::std::move(socket)
                ) };
                if (newConnection->connectToClient(++m_idCounter)) {
                    m_incommingConnections.push_back(::std::move(newConnection));
                    ::std::cout << "[Server:TCP:" << m_incommingConnections.back()->getId()
                        << "] Connection started.\n";
                } else {
                    ::std::cerr << "[ERROR:Server] Connection failed.\n";
                }
            } else {
                ::std::cerr << "[ERROR:Server] New connection error: " << errorCode.message() << ".\n";
            }
            this->startReceivingConnections();
        }
    );
}



// ------------------------------------------------------------------ async - in

template <
    ::detail::isEnum MessageType
> void ::network::AServer<MessageType>::pullIncommingMessage()
{
    auto message{ this->getIncommingMessages().pop_front() };
    this->onTcpReceive(message, message.getRemote());
}


template <
    ::detail::isEnum MessageType
> void ::network::AServer<MessageType>::pullIncommingMessages()
{
    while (!this->getIncommingMessages().empty()) {
        this->pullIncommingMessage();
    }
}

template <
    ::detail::isEnum MessageType
> void ::network::AServer<MessageType>::blockingPullIncommingMessages()
{
    this->getIncommingMessages().wait();
    this->pullIncommingMessages();
}



// ------------------------------------------------------------------ async - out

template <
    ::detail::isEnum MessageType
> void ::network::AServer<MessageType>::send(
    ::network::Message<MessageType>& message,
    ::std::shared_ptr<::network::TcpConnection<MessageType>> client
)
{
    client->send(message);
}

template <
    ::detail::isEnum MessageType
> void ::network::AServer<MessageType>::send(
    ::network::Message<MessageType>&& message,
    ::std::shared_ptr<::network::TcpConnection<MessageType>> client
)
{
    client->send(::std::move(message));
}



// ------------------------------------------------------------------ receive behaviour

template <
    ::detail::isEnum MessageType
> auto ::network::AServer<MessageType>::defaultReceiveBehaviour(
    ::network::Message<MessageType>& message,
    ::std::shared_ptr<::network::TcpConnection<MessageType>> connection
) -> bool
{
    switch (message.getType()) {
    case MessageType::ping:
        connection->send(message);
        break;
    default:
        return false;
    }
    return true;
}



// ------------------------------------------------------------------ user methods

template <
    ::detail::isEnum MessageType
> void ::network::AServer<MessageType>::onDisconnect(
    ::std::shared_ptr<::network::TcpConnection<MessageType>> disconnectedConnection
)
{
    ::std::cout << '[' << disconnectedConnection->getId() << "] Disconnected.\n";
    ::std::erase_if(
        m_connections,
        [
            disconnectedConnection
        ](
            const ::std::shared_ptr<::network::TcpConnection<MessageType>>& connection
        ){
            return connection == disconnectedConnection;
        }
    );
}

// refuses the connection by returning false, when connection is done
template <
    ::detail::isEnum MessageType
> auto ::network::AServer<MessageType>::onClientConnect(
    ::std::shared_ptr<::network::TcpConnection<MessageType>> connection
) -> bool
{
    return true;
}

template <
    ::detail::isEnum MessageType
> auto ::network::AServer<MessageType>::onClientIdentificate(
    ::std::shared_ptr<::network::TcpConnection<MessageType>> connection
) -> bool
{
    return true;
}



// ------------------------------------------------------------------ other

template <
    ::detail::isEnum MessageType
> [[ nodiscard ]] auto ::network::AServer<MessageType>::getConnection(
    ::detail::Id id
) -> ::std::shared_ptr<::network::TcpConnection<MessageType>>
{
    return *::std::ranges::find_if(
        m_connections,
        [id](const auto& connection){ return connection->getId() == id; }
    );
}
