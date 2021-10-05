#include <pch.hpp>
#include <Server/AServer.hpp>
#include <MessageType.hpp>



// ------------------------------------------------------------------ explicit instantiations

template class ::network::AServer<::network::MessageType>;



// ------------------------------------------------------------------ *structors

template <
    ::network::detail::IsEnum MessageType
> ::network::AServer<MessageType>::AServer(
    const ::std::uint16_t port
)
    : ::network::ANode<MessageType>{ ::network::ANode<MessageType>::Type::server }
    , m_asioAcceptor{
        this->getAsioContext(),
        ::boost::asio::ip::tcp::endpoint{ ::boost::asio::ip::tcp::v4(), port }
    }
{
    ::std::cout << "Ready to listen port " << port << ".\n";
}

template <
    ::network::detail::IsEnum MessageType
> ::network::AServer<MessageType>::~AServer()
{
    this->stop();
}



// ------------------------------------------------------------------ running

template <
    ::network::detail::IsEnum MessageType
> auto ::network::AServer<MessageType>::start()
    -> bool
{
    try {
        this->startReceivingConnections();

        // start the routine
        this->getThreadContext() = ::std::thread([this](){ this->getAsioContext().run(); });

    } catch (::std::exception& e) {
        ::std::cerr << "Exception: " << e.what() << ".\n";
        return false;
    }
    ::std::cout << "Started" << '\n';
    return true;
}

template <
    ::network::detail::IsEnum MessageType
> void ::network::AServer<MessageType>::stop()
{
    // stop everything running in parallele
    this->getAsioContext().stop();
    if(this->getThreadContext().joinable()) {
        this->getThreadContext().join();
    }

    ::std::cout << "Stopped" << '\n';
}

template <
    ::network::detail::IsEnum MessageType
> auto ::network::AServer<MessageType>::isRunning()
    -> bool
{
    // TODO: isRunning implemetation
    return true;
}



// ------------------------------------------------------------------ in - async

template <
    ::network::detail::IsEnum MessageType
> void ::network::AServer<MessageType>::startReceivingConnections()
{
    m_asioAcceptor.async_accept(
        [this](
            const boost::system::error_code& errorCode,
            ::boost::asio::ip::tcp::socket socket
        ) {
            if (!errorCode) {
                ::std::cout << "New incomming connection: " << socket.remote_endpoint() << ".\n";
                auto newConnection{ ::std::make_shared<::network::Connection<MessageType>>(
                    ::std::move(socket),
                    *this
                ) };
                if (newConnection->connectToClient(++m_idCounter)) {
                    m_connections.push_back(::std::move(newConnection));
                    ::std::cout << "Connection ["
                        << m_connections.back()->getId() << "] approved.\n";
                } else {
                    ::std::cerr << "Connection failed.\n";
                }
            } else {
                ::std::cerr << "New connection error: " << errorCode.message() << ".\n";
            }
            this->startReceivingConnections();
        }
    );
}



// ------------------------------------------------------------------ async - in

template <
    ::network::detail::IsEnum MessageType
> void ::network::AServer<MessageType>::pullIncommingMessage()
{
    auto message{ this->getIncommingMessages().pop_front() };
    this->onReceive(message, message.getRemote());
}


template <
    ::network::detail::IsEnum MessageType
> void ::network::AServer<MessageType>::pullIncommingMessages()
{
    while (!this->getIncommingMessages().empty()) {
        this->pullIncommingMessage();
    }
}

template <
    ::network::detail::IsEnum MessageType
> void ::network::AServer<MessageType>::blockingPullIncommingMessages()
{
    this->getIncommingMessages().wait();
    this->pullIncommingMessages();
}



// ------------------------------------------------------------------ async - out

template <
    ::network::detail::IsEnum MessageType
> void ::network::AServer<MessageType>::send(
    const ::network::Message<MessageType>& message,
    ::std::shared_ptr<::network::Connection<MessageType>> client
)
{
    if (client->isConnected()) {
        this->onSend(message, client);
        client->send(message);
    } else {
        this->onDisconnect(client);
        client.reset();
        m_connections.resize(m_connections.size() - ::std::ranges::remove(m_connections, nullptr).size());
    }
}



// ------------------------------------------------------------------ user methods

// refuses the connection by returning false, when connection is done
template <
    ::network::detail::IsEnum MessageType
> auto ::network::AServer<MessageType>::onClientConnect(
    ::std::shared_ptr<::network::Connection<MessageType>> connection
) -> bool
{
    return true;
}

template <
    ::network::detail::IsEnum MessageType
> auto ::network::AServer<MessageType>::onClientIdentificate(
    ::std::shared_ptr<::network::Connection<MessageType>> connection
) -> bool
{
    return true;
}

