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
            ::boost::asio::ip::tcp::socket(this->getAsioContext()),
            *this
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

        // stop everything running in parallele
        this->getAsioContext().stop();
        if(this->getThreadContext().joinable()) {
            this->getThreadContext().join();
        }

        // destroy the connection
        m_connection.reset();
        this->getIncommingMessages().notify();

        ::std::cout << "Disconnected" << '\n';
    }
}

template <
    ::detail::IsEnum MessageType
> auto ::network::AClient<MessageType>::isConnected()
    -> bool
{
    return m_connection && m_connection->isConnected();
}



// ------------------------------------------------------------------ async - out

template <
    ::detail::IsEnum MessageType
> void ::network::AClient<MessageType>::send(
    ::network::Message<MessageType>& message
)
{
    this->onSend(message, m_connection);
    m_connection->send(message);
}

template <
    ::detail::IsEnum MessageType
> void ::network::AClient<MessageType>::send(
    ::network::Message<MessageType>&& message
)
{
    this->onSend(message, m_connection);
    m_connection->send(::std::move(message));
}
