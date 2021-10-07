#include <pch.hpp>
#include <Network/Server/AServer.hpp>
#include <Network/MessageType.hpp>



// ------------------------------------------------------------------ explicit instantiations

template class ::network::AServer<::network::MessageType>;



// ------------------------------------------------------------------ *structors

template <
    ::detail::IsEnum MessageType
> ::network::AServer<MessageType>::AServer(
    const ::std::uint16_t port
)
    : ::network::ANode<MessageType>{ ::network::ANode<MessageType>::Type::server }
    , m_asioAcceptor{
        this->getAsioContext(),
        ::boost::asio::ip::tcp::endpoint{ ::boost::asio::ip::tcp::v4(), port }
    }
{
    ::std::cout << "[Server] Ready to listen port " << port << ".\n";
}

template <
    ::detail::IsEnum MessageType
> ::network::AServer<MessageType>::~AServer()
{
    this->stop();
}



// ------------------------------------------------------------------ running

template <
    ::detail::IsEnum MessageType
> auto ::network::AServer<MessageType>::start()
    -> bool
{
    try {
        this->startReceivingConnections();

        // start the routine
        this->getThreadContext() = ::std::thread([this](){ this->getAsioContext().run(); });

    } catch (::std::exception& e) {
        ::std::cerr << "[ERROR:Server] " << e.what() << ".\n";
        return false;
    }
    ::std::cout << "[Server] Started" << '\n';
    return true;
}

template <
    ::detail::IsEnum MessageType
> void ::network::AServer<MessageType>::stop()
{
    if (this->isRunning()) {
        // stop everything running in parallele
        this->getAsioContext().stop();
        if(this->getThreadContext().joinable()) {
            this->getThreadContext().join();
        }

        this->getIncommingMessages().notify();
        ::std::cout << "[Server] Stopped" << '\n';
    }
}

template <
    ::detail::IsEnum MessageType
> auto ::network::AServer<MessageType>::isRunning()
    -> bool
{
    // TODO: isRunning implemetation
    return !this->getAsioContext().stopped();
}



// ------------------------------------------------------------------ in - async

template <
    ::detail::IsEnum MessageType
> void ::network::AServer<MessageType>::startReceivingConnections()
{
    m_asioAcceptor.async_accept(
        [this](
            const boost::system::error_code& errorCode,
            ::boost::asio::ip::tcp::socket socket
        ) {
            if (!errorCode) {
                ::std::cout << "[Server] New incomming connection: " << socket.remote_endpoint() << ".\n";
                auto newConnection{ ::std::make_shared<::network::Connection<MessageType>>(
                    *this,
                    ::std::move(socket),
                    ::boost::asio::ip::udp::socket{
                        this->getAsioContext(),
                        ::boost::asio::ip::udp::endpoint(::boost::asio::ip::udp::v4(), 0)
                    }
                ) };
                if (newConnection->connectToClient(++m_idCounter)) {
                    m_incommingConnections.push_back(::std::move(newConnection));
                    ::std::cout << '[' << m_incommingConnections.back()->getId()
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
    ::detail::IsEnum MessageType
> void ::network::AServer<MessageType>::pullIncommingMessage()
{
    auto message{ this->getIncommingMessages().pop_front() };
    this->onTcpReceive(message, message.getRemote());
}


template <
    ::detail::IsEnum MessageType
> void ::network::AServer<MessageType>::pullIncommingMessages()
{
    while (!this->getIncommingMessages().empty()) {
        this->pullIncommingMessage();
    }
}

template <
    ::detail::IsEnum MessageType
> void ::network::AServer<MessageType>::blockingPullIncommingMessages()
{
    this->getIncommingMessages().wait();
    this->pullIncommingMessages();
}



// ------------------------------------------------------------------ async - autoProtocol

template <
    ::detail::IsEnum MessageType
> void ::network::AServer<MessageType>::send(
    ::network::Message<MessageType>&& message,
    ::std::shared_ptr<::network::Connection<MessageType>>&& client
)
{
    if (message.getTransmissionProtocol() == ::network::TransmissionProtocol::tcp) {
        this->tcpSend(
            ::std::forward<decltype(message)>(message),
            ::std::forward<decltype(client)>(client)
        );
    } else {
        this->udpSend(
            ::std::forward<decltype(message)>(message),
            ::std::forward<decltype(client)>(client)
        );
    }
}



// ------------------------------------------------------------------ async - tcpOut

template <
    ::detail::IsEnum MessageType
> void ::network::AServer<MessageType>::tcpSend(
    ::network::Message<MessageType>& message,
    ::std::shared_ptr<::network::Connection<MessageType>> client
)
{
    this->onTcpSend(message, client);
    client->tcpSend(message);
}

template <
    ::detail::IsEnum MessageType
> void ::network::AServer<MessageType>::tcpSend(
    ::network::Message<MessageType>&& message,
    ::std::shared_ptr<::network::Connection<MessageType>> client
)
{
    this->onTcpSend(message, client);
    client->tcpSend(::std::move(message));
}



// ------------------------------------------------------------------ async - udpOut

template <
    ::detail::IsEnum MessageType
> void ::network::AServer<MessageType>::udpSend(
    ::network::Message<MessageType>& message,
    ::std::shared_ptr<::network::Connection<MessageType>> client
)
{
    this->onUdpSend(message, client);
    client->udpSend(message);
}

template <
    ::detail::IsEnum MessageType
> void ::network::AServer<MessageType>::udpSend(
    ::network::Message<MessageType>&& message,
    ::std::shared_ptr<::network::Connection<MessageType>> client
)
{
    this->onUdpSend(message, client);
    client->udpSend(::std::move(message));
}



// ------------------------------------------------------------------ receive behaviour

template <
    ::detail::IsEnum MessageType
> auto ::network::AServer<MessageType>::defaultReceiveBehaviour(
    ::network::Message<MessageType>& message,
    ::std::shared_ptr<::network::Connection<MessageType>> connection
) -> bool
{
    switch (message.getType()) {
    default:
        return false;
    }
    return true;
}



// ------------------------------------------------------------------ user methods

// handle the disconnection
template <
    ::detail::IsEnum MessageType
> void ::network::AServer<MessageType>::onDisconnect(
    ::std::shared_ptr<::network::Connection<MessageType>> connection
)
{
    ::std::cout << '[' << m_connections.back()->getId() << "] Disconnected.\n";
    m_connections.erase(::std::ranges::find(m_connections, connection));
}

// refuses the connection by returning false, when connection is done
template <
    ::detail::IsEnum MessageType
> auto ::network::AServer<MessageType>::onClientConnect(
    ::std::shared_ptr<::network::Connection<MessageType>> connection
) -> bool
{
    return true;
}

template <
    ::detail::IsEnum MessageType
> auto ::network::AServer<MessageType>::onClientIdentificate(
    ::std::shared_ptr<::network::Connection<MessageType>> connection
) -> bool
{
    return true;
}
