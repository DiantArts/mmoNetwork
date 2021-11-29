#pragma once

// ------------------------------------------------------------------ *structors

template <
    ::detail::constraint::isEnum UserMessageType
> ::network::client::AClient<UserMessageType>::AClient()
    : ::network::ANode<UserMessageType>{ ::network::ANode<UserMessageType>::Type::client }
{}

template <
    ::detail::constraint::isEnum UserMessageType
> ::network::client::AClient<UserMessageType>::~AClient()
{
    this->stopThread();
    this->disconnect();
}



// ------------------------------------------------------------------ connection

template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::client::AClient<UserMessageType>::disconnect()
{

    this->disconnectFromServer();
    this->getIncommingMessages().notify();
}

template <
    ::detail::constraint::isEnum UserMessageType
> auto ::network::client::AClient<UserMessageType>::isConnected() const
    -> bool
{
    return this->isConnectedToServer();
}

template <
    ::detail::constraint::isEnum UserMessageType
> auto ::network::client::AClient<UserMessageType>::getUdpPort() const
    -> ::std::uint16_t
{
    return m_connectionToServer->udp.getPort();
}



// ------------------------------------------------------------------ server

template <
    ::detail::constraint::isEnum UserMessageType
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
    ::detail::constraint::isEnum UserMessageType
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
    ::detail::constraint::isEnum UserMessageType
> auto ::network::client::AClient<UserMessageType>::isConnectedToServer() const
    -> bool
{
    return m_connectionToServer && m_connectionToServer->tcp.isConnected();
}



// ------------------------------------------------------------------ tcp

template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::client::AClient<UserMessageType>::tcpSendToServer(
    ::network::Message<UserMessageType>::SystemType messageType,
    auto&&... args
)
{
    m_connectionToServer->tcp.send(
        ::network::Message<UserMessageType>{ messageType, ::std::forward<decltype(args)>(args)... }
    );
}

template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::client::AClient<UserMessageType>::tcpSendToServer(
    UserMessageType messageType,
    auto&&... args
)
{
    m_connectionToServer->tcp.send(
        ::network::Message{ messageType, ::std::forward<decltype(args)>(args)... }
    );
}

template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::client::AClient<UserMessageType>::tcpSendToServer(
    ::network::Message<UserMessageType>& message
)
{
    m_connectionToServer->tcp.send(message);
}

template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::client::AClient<UserMessageType>::tcpSendToServer(
    ::network::Message<UserMessageType>&& message
)
{
    m_connectionToServer->tcp.send(::std::move(message));
}



// ------------------------------------------------------------------ udp

template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::client::AClient<UserMessageType>::udpSendToServer(
    ::network::Message<UserMessageType>::SystemType messageType,
    auto&&... args
)
{
    m_connectionToServer->udp.send(
        ::network::Message<UserMessageType>{ messageType, ::std::forward<decltype(args)>(args)... }
    );
}

template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::client::AClient<UserMessageType>::udpSendToServer(
    UserMessageType messageType,
    auto&&... args
)
{
    m_connectionToServer->udp.send(
        ::network::Message{ messageType, ::std::forward<decltype(args)>(args)... }
    );
}

template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::client::AClient<UserMessageType>::udpSendToServer(
    ::network::Message<UserMessageType>& message
)
{
    m_connectionToServer->udp.send(message);
}

template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::client::AClient<UserMessageType>::udpSendToServer(
    ::network::Message<UserMessageType>&& message
)
{
    m_connectionToServer->udp.send(::std::move(message));
}



// ------------------------------------------------------------------ default behaviours

template <
    ::detail::constraint::isEnum UserMessageType
> auto ::network::client::AClient<UserMessageType>::defaultReceiveBehaviour(
    ::network::Message<UserMessageType>& message,
    ::std::shared_ptr<::network::Connection<UserMessageType>> connection
) -> bool
{
    switch (message.getTypeAsSystemType()) {
    default: break;
    }
    if (message.getTransmissionProtocol() == ::network::Message<UserMessageType>::Protocol::tcp) {
        switch (message.getTypeAsSystemType()) {
        case ::network::Message<UserMessageType>::SystemType::sharableInformations: {
            switch (message.template pull<::network::Informations::Index>()) {
            case ::network::Informations::Index::name: {
                auto id{ message.template pull<::detail::Id>() };
                m_connectedClientsInformations.at(id).name = message.template pull<::std::string>();
                break;
            } default:
                ::std::cerr << "[ERROR:Server:TCP:" << connection->informations.getName() << "]: "
                    << "Unwnown informations index.";
                connection->disconnect();
                break;
            }
            return true;
        } case ::network::Message<UserMessageType>::SystemType::allSharableInformations: {
            message.pull(m_connectedClientsInformations);
            m_connectedClientsInformations.erase(m_connectionToServer->informations.getId());
            return true;
        } case ::network::Message<UserMessageType>::SystemType::newConnection: {
            auto id{ message.template pull<::detail::Id>() };
            auto sharable{ message.template pull<::network::Informations::Sharable>() };
            m_connectedClientsInformations.emplace(::std::make_pair(::std::move(id), ::std::move(sharable)));
            return true;
        } case ::network::Message<UserMessageType>::SystemType::ping:
            connection->tcp.send(::std::move(message));
            return true;
        default: break;
        }
    } else {
        switch (message.getTypeAsSystemType()) {
        case ::network::Message<UserMessageType>::SystemType::ping:
            connection->udp.send(::std::move(message));
            return true;
        default: break;
        }
    }
    return false;
}



// ------------------------------------------------------------------ user behaviours

template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::client::AClient<UserMessageType>::onDisconnect(
    ::std::shared_ptr<::network::Connection<UserMessageType>> connection
)
{
    ANode<UserMessageType>::onDisconnect(connection);
}

template <
    ::detail::constraint::isEnum UserMessageType
> auto ::network::client::AClient<UserMessageType>::onAuthentification(
    ::std::shared_ptr<::network::Connection<UserMessageType>> connection
) -> bool

{
    ::std::cout << "[Client:TCP:" << connection->informations.getId() << "] onAuthentification.\n";
    connection->informations.setName("user"s + ::std::to_string(connection->informations.getId()));
    return true;
}

template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::client::AClient<UserMessageType>::onConnectionValidated(
    ::std::shared_ptr<::network::Connection<UserMessageType>> connection
)
{
    ::std::cout << "[Client:TCP:" << connection->informations.getId() << "] onConnectionValidated.\n";
    connection->tcp.startReceivingMessage();
    connection->udp.startReceivingMessage();
}



// ------------------------------------------------------------------ others

template <
    ::detail::constraint::isEnum UserMessageType
> template <
    ::network::Informations::Index indexValue
> void ::network::client::AClient<UserMessageType>::setInformation(
    auto&&... args
)
{
    m_connectionToServer->informations.template set<indexValue>(::std::forward<decltype(args)>(args)...);
    this->tcpSendToServer(
        ::network::Message<UserMessageType>::SystemType::sharableInformations,
        ::std::forward<decltype(args)>(args)...,
        indexValue
    );
}
