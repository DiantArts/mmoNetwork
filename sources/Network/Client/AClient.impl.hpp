#pragma once

// ------------------------------------------------------------------ *structors

template <
    ::detail::isEnum UserMessageType
> ::network::client::AClient<UserMessageType>::AClient()
    : ::network::ANode<UserMessageType>{ ::network::ANode<UserMessageType>::Type::client }
{}

template <
    ::detail::isEnum UserMessageType
> ::network::client::AClient<UserMessageType>::~AClient()
{
    this->stopThread();
    this->disconnect();
}



// ------------------------------------------------------------------ connection

template <
    ::detail::isEnum UserMessageType
> void ::network::client::AClient<UserMessageType>::disconnect()
{

    this->disconnectFromServer();
    this->getIncommingMessages().notify();
}

template <
    ::detail::isEnum UserMessageType
> auto ::network::client::AClient<UserMessageType>::isConnected() const
    -> bool
{
    return this->isConnectedToServer();
}

template <
    ::detail::isEnum UserMessageType
> auto ::network::client::AClient<UserMessageType>::getUdpPort() const
    -> ::std::uint16_t
{
    return m_connectionToServer->udp.getPort();
}



// ------------------------------------------------------------------ server

template <
    ::detail::isEnum UserMessageType
> auto ::network::client::AClient<UserMessageType>::connectToServer(
    const ::std::string& host,
    const ::std::uint16_t port
) -> bool
{
    if (this->isConnectedToServer()) {
        return false;
    }
    try {
        ::asio::io_context::work idleWork{ this->getAsioContext() };
        this->getThreadContext() = ::std::thread([this](){
            this->getAsioContext().run();
        });
        m_connectionToServer = ::network::Connection<UserMessageType>::create(*this, host, port);
        if (this->isConnectedToServer()) {
            return true;
        }
    } catch (::std::exception& e) {
        ::std::cerr << "Exception: " << e.what() << '\n';
    }
    this->disconnectFromServer();
    return false;

}

template <
    ::detail::isEnum UserMessageType
> void ::network::client::AClient<UserMessageType>::disconnectFromServer()
{
    if (m_connectionToServer) {
        if (this->isConnectedToServer()) {
            m_connectionToServer->disconnect();
        }
        m_connectionToServer.reset();
    }
}

template <
    ::detail::isEnum UserMessageType
> auto ::network::client::AClient<UserMessageType>::isConnectedToServer() const
    -> bool
{
    return m_connectionToServer && m_connectionToServer->tcp.isConnected();
}



// ------------------------------------------------------------------ tcp

template <
    ::detail::isEnum UserMessageType
> void ::network::client::AClient<UserMessageType>::tcpSendToServer(
    ::network::Message<UserMessageType>& message
)
{
    m_connectionToServer->tcp.send(message);
}

template <
    ::detail::isEnum UserMessageType
> void ::network::client::AClient<UserMessageType>::tcpSendToServer(
    ::network::Message<UserMessageType>&& message
)
{
    m_connectionToServer->tcp.send(::std::move(message));
}



// ------------------------------------------------------------------ udp

template <
    ::detail::isEnum UserMessageType
> void ::network::client::AClient<UserMessageType>::udpSendToServer(
    ::network::Message<UserMessageType>& message
)
{
    m_connectionToServer->udp.send(message);
}

template <
    ::detail::isEnum UserMessageType
> void ::network::client::AClient<UserMessageType>::udpSendToServer(
    ::network::Message<UserMessageType>&& message
)
{
    m_connectionToServer->udp.send(::std::move(message));
}



// ------------------------------------------------------------------ default behaviours

template <
    ::detail::isEnum UserMessageType
> auto ::network::client::AClient<UserMessageType>::defaultReceiveBehaviour(
    ::network::Message<UserMessageType>& message,
    ::std::shared_ptr<::network::Connection<UserMessageType>> connection
) -> bool
{
    switch (message.getTypeAsSystemType()) {
    default: break;
    }
    if (
        message.getTransmissionProtocol() ==
        ::network::Message<UserMessageType>::TransmissionProtocol::tcp
    ) {
        switch (message.getTypeAsSystemType()) {
        case ::network::Message<UserMessageType>::SystemType::ping: connection->tcp.send(message); break;
        default: return false;
        }
    } else {
        switch (message.getTypeAsSystemType()) {
        case ::network::Message<UserMessageType>::SystemType::ping: connection->udp.send(message); break;
        default: return false;
        }
    }
    return true;
}



// ------------------------------------------------------------------ user behaviours

template <
    ::detail::isEnum UserMessageType
> void ::network::client::AClient<UserMessageType>::onDisconnect(
    ::std::shared_ptr<::network::Connection<UserMessageType>> connection
)
{
    ANode<UserMessageType>::onDisconnect(connection);
}

template <
    ::detail::isEnum UserMessageType
> auto ::network::client::AClient<UserMessageType>::onAuthentification(
    ::std::shared_ptr<::network::Connection<UserMessageType>> connection
) -> bool

{
    ::std::cout << "[Client:TCP:" << connection->informations.id << "] onAuthentification.\n";
    connection->setUserName("user"s + ::std::to_string(connection->informations.id));
    return true;
}

template <
    ::detail::isEnum UserMessageType
> void ::network::client::AClient<UserMessageType>::onConnectionValidated(
    ::std::shared_ptr<::network::Connection<UserMessageType>> connection
)
{
    ::std::cout << "[Client:TCP:" << connection->informations.id << "] onConnectionValidated.\n";

    // start reading/writing tcp
    connection->tcp.startReceivingMessage();
    if (connection->tcp.hasSendingMessagesAwaiting()) {
        connection->tcp.sendAwaitingMessages();
    }

    // TODO: udp part
    // connection->udp.startReceivingMessage();
    // if (connection->udp.hasSendingMessagesAwaiting()) {
        // connection->udp.sendAwaitingMessages();
    // }
}
