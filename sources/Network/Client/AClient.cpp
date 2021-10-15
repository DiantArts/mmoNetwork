#include <pch.hpp>
#include <Network/Client/AClient.hpp>
#include <Network/MessageType.hpp>



// ------------------------------------------------------------------ explicit instantiations

template class ::network::AClient<::network::MessageType>;



// ------------------------------------------------------------------ *structors

template <
    ::detail::isEnum MessageType
> ::network::AClient<MessageType>::AClient()
    : ::network::ANode<MessageType>{ ::network::ANode<MessageType>::Type::client }
{}

template <
    ::detail::isEnum MessageType
> ::network::AClient<MessageType>::~AClient()
{
    this->stopThread();
    this->disconnect();
}



// ------------------------------------------------------------------ connection

template <
    ::detail::isEnum MessageType
> void ::network::AClient<MessageType>::disconnect()
{

    this->disconnectFromServer();
    this->closePeerConnection();
    this->getIncommingMessages().notify();
}

template <
    ::detail::isEnum MessageType
> auto ::network::AClient<MessageType>::isConnected()
    -> bool
{
    return this->isConnectedToServer() || this->isConnectedToPeer();
}

template <
    ::detail::isEnum MessageType
> auto ::network::AClient<MessageType>::getUdpPort()
    -> ::std::uint16_t
{
    return m_connectionToPeer->getPort();
}



// ------------------------------------------------------------------ server

template <
    ::detail::isEnum MessageType
> auto ::network::AClient<MessageType>::connectToServer(
    const ::std::string& host,
    const ::std::uint16_t port
) -> bool
{
    try {
        m_connectionToServer = ::std::make_shared<::network::TcpConnection<MessageType>>(
            *this,
            ::boost::asio::ip::tcp::socket(this->getAsioContext())
        );
        m_connectionToServer->connect(host, port);
        ::std::cout << "[Client:TCP:" << m_connectionToServer->getId()
            << "] Connection request sent to " << host << ":" << port << ".\n";
        if (!this->getThreadContext().joinable()) {
            this->getThreadContext() = ::std::thread([this](){ this->getAsioContext().run(); });
        }
    } catch (::std::exception& e) {
        ::std::cerr << "Exception: " << e.what() << '\n';
        return false;
    }
    return true;
}

template <
    ::detail::isEnum MessageType
> void ::network::AClient<MessageType>::disconnectFromServer()
{
    if (m_connectionToServer) {
        if (this->isConnectedToServer()) {
            m_connectionToServer->disconnect();
            ::std::cout << "[Client:TCP:" << m_connectionToServer->getId() << "] Disconnected.\n";
        }
        m_connectionToServer.reset();
    }
}

template <
    ::detail::isEnum MessageType
> void ::network::AClient<MessageType>::sendToServer(
    ::network::Message<MessageType>& message
)
{
    m_connectionToServer->send(message);
}

template <
    ::detail::isEnum MessageType
> void ::network::AClient<MessageType>::sendToServer(
    ::network::Message<MessageType>&& message
)
{
    m_connectionToServer->send(::std::move(message));
}



template <
    ::detail::isEnum MessageType
> auto ::network::AClient<MessageType>::isConnectedToServer()
    -> bool
{
    return m_connectionToServer && m_connectionToServer->isConnected();
}



// ------------------------------------------------------------------ peer

template <
    ::detail::isEnum MessageType
> void ::network::AClient<MessageType>::openUdpConnection()
{
    m_connectionToPeer = ::std::make_shared<::network::UdpConnection<MessageType>>(*this);
}

template <
    ::detail::isEnum MessageType
> auto ::network::AClient<MessageType>::targetPeer(
    const ::std::string& host,
    const ::std::uint16_t port
) -> bool
{
    try {
        m_connectionToPeer->target(host, port);
        if (!this->getThreadContext().joinable()) {
            this->getThreadContext() = ::std::thread([this](){ this->getAsioContext().run(); });
        }
    } catch (::std::exception& e) {
        ::std::cerr << "Exception: " << e.what() << '\n';
        return false;
    }
    return true;
}

template <
    ::detail::isEnum MessageType
> void ::network::AClient<MessageType>::closePeerConnection()
{
    if (this->isConnectedToPeer()) {
        m_connectionToPeer->close();
        ::std::cout << "[Client:UDP] Disconnected.\n";
    }
    m_connectionToPeer.reset();
}

template <
    ::detail::isEnum MessageType
> void ::network::AClient<MessageType>::sendToPeer(
    ::network::Message<MessageType>&& message
)
{
    m_connectionToPeer->send(::std::move(message));
}

template <
    ::detail::isEnum MessageType
> auto ::network::AClient<MessageType>::isConnectedToPeer()
    -> bool
{
    return m_connectionToPeer && m_connectionToPeer->isOpen();
}



// ------------------------------------------------------------------ default behaviours

template <
    ::detail::isEnum MessageType
> auto ::network::AClient<MessageType>::defaultReceiveBehaviour(
    ::network::Message<MessageType>& message,
    ::std::shared_ptr<::network::TcpConnection<MessageType>> connection
) -> bool
{
    switch (message.getType()) {
    default:
        return false;
    }
    return true;
}

template <
    ::detail::isEnum MessageType
> void ::network::AClient<MessageType>::onDisconnect(
    ::std::shared_ptr<::network::TcpConnection<MessageType>> connection
)
{
    if (this->isConnectedToPeer()) {
        m_connectionToPeer->close();
    }
    ANode<MessageType>::onDisconnect(connection);
}

template <
    ::detail::isEnum MessageType
> void ::network::AClient<MessageType>::onUdpDisconnect(
    ::std::shared_ptr<::network::UdpConnection<MessageType>> connection
)
{
    ::std::cout << "[Client:UDP] OnDisconnect.\n";
}
