#include <pch.hpp>
#include <Network/Client/AClient.hpp>
#include <Network/MessageType.hpp>



// ------------------------------------------------------------------ explicit instantiations

template class ::network::AClient<::network::MessageType>;



// ------------------------------------------------------------------ *structors

template <
    ::detail::IsEnum MessageType
> ::network::AClient<MessageType>::AClient()
    : ::network::ANode<MessageType>{ ::network::ANode<MessageType>::Type::client }
{}

template <
    ::detail::IsEnum MessageType
> ::network::AClient<MessageType>::~AClient()
{
    this->disconnect();
    this->stop();
}



// ------------------------------------------------------------------ connection

template <
    ::detail::IsEnum MessageType
> auto ::network::AClient<MessageType>::connect(
    const ::std::string& host,
    const ::std::uint16_t port
) -> bool
{
    try {
        m_connection = ::std::make_shared<::network::Connection<MessageType>>(
            *this,
            ::boost::asio::ip::tcp::socket(this->getAsioContext()),
            ::boost::asio::ip::udp::socket(this->getAsioContext())
        );
        m_connection->connectToServer(host, port);
        this->getThreadContext() = ::std::thread([this](){ this->getAsioContext().run(); });
    } catch (::std::exception& e) {
        ::std::cerr << "Exception: " << e.what() << '\n';
        return false;
    }
    return true;
}

template <
    ::detail::IsEnum MessageType
> void ::network::AClient<MessageType>::disconnect()
{
    if (this->isConnected()) {
        this->onDisconnect(m_connection);

        m_connection->disconnect();

        // destroy the connection
        m_connection.reset();
        ::std::cout << "Disconnected" << '\n';
    }
}

template <
    ::detail::IsEnum MessageType
> void ::network::AClient<MessageType>::stop()
{
    this->getIncommingMessages().notify();
    this->getAsioContext().stop();
    if(this->getThreadContext().joinable()) {
        this->getThreadContext().join();
    }
}

template <
    ::detail::IsEnum MessageType
> auto ::network::AClient<MessageType>::isConnected()
    -> bool
{
    return m_connection && m_connection->isConnected();
}



// ------------------------------------------------------------------ async - tcpOut

template <
    ::detail::IsEnum MessageType
> void ::network::AClient<MessageType>::tcpSend(
    ::network::Message<MessageType>& message
)
{
    this->onTcpSend(message, m_connection);
    m_connection->tcpSend(message);
}

template <
    ::detail::IsEnum MessageType
> void ::network::AClient<MessageType>::tcpSend(
    ::network::Message<MessageType>&& message
)
{
    this->onTcpSend(message, m_connection);
    m_connection->tcpSend(::std::move(message));
}



// ------------------------------------------------------------------ async - udpOut

template <
    ::detail::IsEnum MessageType
> void ::network::AClient<MessageType>::udpSend(
    ::network::Message<MessageType>& message
)
{
    this->onTcpSend(message, m_connection);
    m_connection->udpSend(message);
}

template <
    ::detail::IsEnum MessageType
> void ::network::AClient<MessageType>::udpSend(
    ::network::Message<MessageType>&& message
)
{
    this->onTcpSend(message, m_connection);
    m_connection->udpSend(::std::move(message));
}



// ------------------------------------------------------------------ receive behaviour

template <
    ::detail::IsEnum MessageType
> auto ::network::AClient<MessageType>::defaultReceiveBehaviour(
    ::network::Message<MessageType>& message,
    ::std::shared_ptr<::network::Connection<MessageType>> connection
) -> bool
{
    switch (message.getType()) {
    case MessageType::udpPort:
        ::std::uint16_t udpPort;
        message >> udpPort;
        m_connection->targetServerUdpPort(udpPort);
        break;
    default:
        return false;
    }
    return true;
}
