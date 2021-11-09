#pragma once

// ------------------------------------------------------------------ *structors

template <
    typename UserMessageType
> ::network::client::AClient<UserMessageType>::AClient()
    : ::network::ANode<UserMessageType>{ ::network::ANode<UserMessageType>::Type::client }
{}

template <
    typename UserMessageType
> ::network::client::AClient<UserMessageType>::~AClient()
{
    this->stopThread();
    this->disconnect();
}



// ------------------------------------------------------------------ connection

template <
    typename UserMessageType
> void ::network::client::AClient<UserMessageType>::disconnect()
{

    this->disconnectFromServer();
    this->closePeerConnection();
    this->getIncommingMessages().notify();
}

template <
    typename UserMessageType
> auto ::network::client::AClient<UserMessageType>::isConnected() const
    -> bool
{
    return this->isConnectedToServer() || this->isConnectedToPeer();
}

template <
    typename UserMessageType
> auto ::network::client::AClient<UserMessageType>::getUdpPort() const
    -> ::std::uint16_t
{
    return m_connectionToPeer->getPort();
}



// ------------------------------------------------------------------ server

template <
    typename UserMessageType
> auto ::network::client::AClient<UserMessageType>::startConnectingToServer(
    const ::std::string& host,
    const ::std::uint16_t port
) -> bool
{
    try {
        m_tcpConnectionToServer = ::std::make_shared<::network::tcp::Connection<UserMessageType>>(
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
    typename UserMessageType
> auto ::network::client::AClient<UserMessageType>::connectToServer(
    const ::std::string& host,
    const ::std::uint16_t port
) -> bool
{
    // TODO: Client block until connected
    return this->startConnectingToServer(host, port);
}

template <
    typename UserMessageType
> void ::network::client::AClient<UserMessageType>::disconnectFromServer()
{
    if (m_tcpConnectionToServer) {
        if (this->isConnectedToServer()) {
            m_tcpConnectionToServer->disconnect();
        }
        m_tcpConnectionToServer.reset();
    }
}

template <
    typename UserMessageType
> void ::network::client::AClient<UserMessageType>::sendToServer(
    ::network::Message<UserMessageType>& message
)
{
    m_tcpConnectionToServer->send(message);
}

template <
    typename UserMessageType
> void ::network::client::AClient<UserMessageType>::sendToServer(
    ::network::Message<UserMessageType>&& message
)
{
    m_tcpConnectionToServer->send(::std::move(message));
}



template <
    typename UserMessageType
> auto ::network::client::AClient<UserMessageType>::isConnectedToServer() const
    -> bool
{
    return m_tcpConnectionToServer && m_tcpConnectionToServer->isConnected();
}



// ------------------------------------------------------------------ peer

template <
    typename UserMessageType
> void ::network::client::AClient<UserMessageType>::openUdpConnection()
{
    m_connectionToPeer = ::std::make_shared<::network::udp::Connection<UserMessageType>>(*this);
}

template <
    typename UserMessageType
> auto ::network::client::AClient<UserMessageType>::targetPeer(
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
    typename UserMessageType
> void ::network::client::AClient<UserMessageType>::closePeerConnection()
{
    if (this->isConnectedToPeer()) {
        m_connectionToPeer->close();
        ::std::cout << "[Client:UDP] Disconnected.\n";
    }
    m_connectionToPeer.reset();
}

template <
    typename UserMessageType
> void ::network::client::AClient<UserMessageType>::sendToPeer(
    ::network::Message<UserMessageType>&& message
)
{
    m_connectionToPeer->send(::std::move(message));
}

template <
    typename UserMessageType
> auto ::network::client::AClient<UserMessageType>::isConnectedToPeer() const
    -> bool
{
    return m_connectionToPeer && m_connectionToPeer->isOpen();
}



// ------------------------------------------------------------------ default behaviours

template <
    typename UserMessageType
> auto ::network::client::AClient<UserMessageType>::defaultReceiveBehaviour(
    ::network::Message<UserMessageType>& message,
    ::std::shared_ptr<::network::tcp::Connection<UserMessageType>> connection
) -> bool
{
    switch (message.getType()) {
    default:
        return false;
    }
    return true;
}

template <
    typename UserMessageType
> auto ::network::client::AClient<UserMessageType>::defaultReceiveBehaviour(
    ::network::Message<UserMessageType>& message,
    ::std::shared_ptr<::network::udp::Connection<UserMessageType>> connection
) -> bool
{
    switch (message.getType()) {
    default:
        return false;
    }
    return true;
}



// ------------------------------------------------------------------ user behaviours

template <
    typename UserMessageType
> void ::network::client::AClient<UserMessageType>::onDisconnect(
    ::std::shared_ptr<::network::tcp::Connection<UserMessageType>> connection
)
{
    if (this->isConnectedToPeer()) {
        m_connectionToPeer->close();
    }
    ANode<UserMessageType>::onDisconnect(connection);
}

template <
    typename UserMessageType
> auto ::network::client::AClient<UserMessageType>::onAuthentification(
    ::std::shared_ptr<::network::tcp::Connection<UserMessageType>> connection
) -> bool

{
    ::std::cout << "[Client:TCP:" << connection->getId() << "] onAuthentification.\n";
    connection->setUserName(::std::string("user") + ::std::to_string(connection->getId()));
    return true;
}

template <
    typename UserMessageType
> void ::network::client::AClient<UserMessageType>::onConnectionValidated(
    ::std::shared_ptr<::network::tcp::Connection<UserMessageType>> connection
)
{
    ::std::cout << "[Client:TCP:" << connection->getId() << "] onConnectionValidated.\n";
    // start reading/writing tcp
    connection->startReadMessage();
    if (connection->hasSendingMessagesAwaiting()) {
        connection->writeAwaitingMessages();
    }
}



template <
    typename UserMessageType
> void ::network::client::AClient<UserMessageType>::onDisconnect(
    ::std::shared_ptr<::network::udp::Connection<UserMessageType>> connection
)
{
    ::std::cout << "[Client:UDP] OnDisconnect.\n";
}
