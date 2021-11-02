#pragma once

// ------------------------------------------------------------------ *structors

template <
    ::detail::isEnum UserMessageType
> ::network::AClient<UserMessageType>::AClient()
    : ::network::ANode<UserMessageType>{ ::network::ANode<UserMessageType>::Type::client }
{}

template <
    ::detail::isEnum UserMessageType
> ::network::AClient<UserMessageType>::~AClient()
{
    this->stopThread();
    this->disconnect();
}



// ------------------------------------------------------------------ connection

template <
    ::detail::isEnum UserMessageType
> void ::network::AClient<UserMessageType>::disconnect()
{

    this->disconnectFromServer();
    this->closePeerConnection();
    this->getIncommingMessages().notify();
}

template <
    ::detail::isEnum UserMessageType
> auto ::network::AClient<UserMessageType>::isConnected() const
    -> bool
{
    return this->isConnectedToServer() || this->isConnectedToPeer();
}

template <
    ::detail::isEnum UserMessageType
> auto ::network::AClient<UserMessageType>::getUdpPort() const
    -> ::std::uint16_t
{
    return m_connectionToPeer->getPort();
}



// ------------------------------------------------------------------ server

template <
    ::detail::isEnum UserMessageType
> auto ::network::AClient<UserMessageType>::connectToServer(
    const ::std::string& host,
    const ::std::uint16_t port
) -> bool
{
    try {
        m_tcpConnectionToServer = ::std::make_shared<::network::TcpConnection<UserMessageType>>(
            *this,
            ::asio::ip::tcp::socket(this->getAsioContext())
        );
        m_tcpConnectionToServer->connect(host, port);
        ::std::cout << "[Client:TCP:" << m_tcpConnectionToServer->getId()
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
    ::detail::isEnum UserMessageType
> void ::network::AClient<UserMessageType>::disconnectFromServer()
{
    if (m_tcpConnectionToServer) {
        if (this->isConnectedToServer()) {
            m_tcpConnectionToServer->disconnect();
            ::std::cout << "[Client:TCP:" << m_tcpConnectionToServer->getId() << "] Disconnected.\n";
        }
        m_tcpConnectionToServer.reset();
    }
}

template <
    ::detail::isEnum UserMessageType
> void ::network::AClient<UserMessageType>::sendToServer(
    ::network::Message<UserMessageType>& message
)
{
    m_tcpConnectionToServer->send(message);
}

template <
    ::detail::isEnum UserMessageType
> void ::network::AClient<UserMessageType>::sendToServer(
    ::network::Message<UserMessageType>&& message
)
{
    m_tcpConnectionToServer->send(::std::move(message));
}



template <
    ::detail::isEnum UserMessageType
> auto ::network::AClient<UserMessageType>::isConnectedToServer() const
    -> bool
{
    return m_tcpConnectionToServer && m_tcpConnectionToServer->isConnected();
}



// ------------------------------------------------------------------ peer

template <
    ::detail::isEnum UserMessageType
> void ::network::AClient<UserMessageType>::openUdpConnection()
{
    m_connectionToPeer = ::std::make_shared<::network::UdpConnection<UserMessageType>>(*this);
}

template <
    ::detail::isEnum UserMessageType
> auto ::network::AClient<UserMessageType>::targetPeer(
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
    ::detail::isEnum UserMessageType
> void ::network::AClient<UserMessageType>::closePeerConnection()
{
    if (this->isConnectedToPeer()) {
        m_connectionToPeer->close();
        ::std::cout << "[Client:UDP] Disconnected.\n";
    }
    m_connectionToPeer.reset();
}

template <
    ::detail::isEnum UserMessageType
> void ::network::AClient<UserMessageType>::sendToPeer(
    ::network::Message<UserMessageType>&& message
)
{
    m_connectionToPeer->send(::std::move(message));
}

template <
    ::detail::isEnum UserMessageType
> auto ::network::AClient<UserMessageType>::isConnectedToPeer() const
    -> bool
{
    return m_connectionToPeer && m_connectionToPeer->isOpen();
}



// ------------------------------------------------------------------ default behaviours

template <
    ::detail::isEnum UserMessageType
> auto ::network::AClient<UserMessageType>::defaultReceiveBehaviour(
    ::network::Message<UserMessageType>& message,
    ::std::shared_ptr<::network::TcpConnection<UserMessageType>> connection
) -> bool
{
    switch (message.getType()) {
    default:
        return false;
    }
    return true;
}

template <
    ::detail::isEnum UserMessageType
> void ::network::AClient<UserMessageType>::onDisconnect(
    ::std::shared_ptr<::network::TcpConnection<UserMessageType>> connection
)
{
    if (this->isConnectedToPeer()) {
        m_connectionToPeer->close();
    }
    ANode<UserMessageType>::onDisconnect(connection);
}

template <
    ::detail::isEnum UserMessageType
> auto ::network::AClient<UserMessageType>::onAuthentification(
    ::std::shared_ptr<::network::TcpConnection<UserMessageType>> connection
) -> bool

{
    ::std::cout << "[AClient:TCP:" << connection->getId() << "] onAuthentification.\n";
    connection->setUserName("user"s + ::std::to_string(connection->getId()));
    return true;
}

template <
    ::detail::isEnum UserMessageType
> void ::network::AClient<UserMessageType>::onConnectionValidated(
    ::std::shared_ptr<::network::TcpConnection<UserMessageType>> connection
)
{
    ::std::cout << "[AClient:TCP:" << connection->getId() << "] onConnectionValidated.\n";
    // start reading/writing tcp
    connection->startReadMessage();
    if (connection->hasSendingMessagesAwaiting()) {
        connection->writeAwaitingMessages();
    }
}



template <
    ::detail::isEnum UserMessageType
> void ::network::AClient<UserMessageType>::onUdpDisconnect(
    ::std::shared_ptr<::network::UdpConnection<UserMessageType>> connection
)
{
    ::std::cout << "[Client:UDP] OnDisconnect.\n";
}
