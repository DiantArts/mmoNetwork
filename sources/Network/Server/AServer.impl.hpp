#pragma once

// ------------------------------------------------------------------ *structors

template <
    ::detail::constraint::isEnum UserMessageType
> ::network::server::AServer<UserMessageType>::AServer(
    const ::std::uint16_t port
)
    : ::network::ANode<UserMessageType>{ ::network::ANode<UserMessageType>::Type::server }
    , m_asioAcceptor{
        this->getAsioContext(),
        ::asio::ip::tcp::endpoint{ ::asio::ip::tcp::v4(), port }
    }
{
    ::std::cout << "[Server] Ready to listen port " << port << ".\n";
}

template <
    ::detail::constraint::isEnum UserMessageType
> ::network::server::AServer<UserMessageType>::~AServer()
{
    this->stop();
}



// ------------------------------------------------------------------ running

template <
    ::detail::constraint::isEnum UserMessageType
> auto ::network::server::AServer<UserMessageType>::start()
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
    ::detail::constraint::isEnum UserMessageType
> void ::network::server::AServer<UserMessageType>::stop()
{
    if (this->isRunning()) {
        this->stopThread();
        this->getIncommingMessages().notify();
        ::std::cout << "[Server] Stopped" << '\n';
    }
}

template <
    ::detail::constraint::isEnum UserMessageType
> auto ::network::server::AServer<UserMessageType>::isRunning()
    -> bool
{
    // TODO: isRunning implemetation
    return !this->getAsioContext().stopped();
}



// ------------------------------------------------------------------ in

template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::server::AServer<UserMessageType>::startReceivingConnections()
{
    m_asioAcceptor.async_accept(
        [this](
            const ::std::error_code& errorCode,
            ::asio::ip::tcp::socket socket
        ) {
            if (!errorCode) {
                ::std::cout << "[Server] New incomming connection: " << socket.remote_endpoint() << ".\n";
                m_incommingConnections.push_back(::network::Connection<UserMessageType>::create(
                    *this, ::std::move(socket), ++m_idCounter
                ));
            } else {
                ::std::cerr << "[ERROR:Server] New connection error: " << errorCode.message() << ".\n";
            }
            this->startReceivingConnections();
        }
    );
}



// ------------------------------------------------------------------ tcpSend

template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::server::AServer<UserMessageType>::tcpSendToClient(
    ::network::Message<UserMessageType>& message,
    ::std::shared_ptr<::network::Connection<UserMessageType>> client
)
{
    client->tcpSendToClient(message);
}

template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::server::AServer<UserMessageType>::tcpSendToClient(
    ::network::Message<UserMessageType>&& message,
    ::std::shared_ptr<::network::Connection<UserMessageType>> client
)
{
    client->tcpSendToClient(::std::move(message));
}

template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::server::AServer<UserMessageType>::tcpSendToClient(
    ::network::Message<UserMessageType>& message,
    ::detail::Id clientId
)
{
    this->getConnection(clientId)->tcpSendToClient(message);
}

template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::server::AServer<UserMessageType>::tcpSendToClient(
    ::network::Message<UserMessageType>&& message,
    ::detail::Id clientId
)
{
    this->getConnection(clientId)->tcpSendToClient(::std::move(message));
}

template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::server::AServer<UserMessageType>::tcpSendToClient(
    const ::network::Message<UserMessageType>& message,
    ::detail::constraint::sameAs<::std::shared_ptr<::network::Connection<UserMessageType>>> auto... clients
)
{
    for (auto& client : {clients...}) {
        client->tcp.send(::network::Message<UserMessageType>{ message });
    }
}

template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::server::AServer<UserMessageType>::tcpSendToAllClients(
    ::network::Message<UserMessageType>::SystemType messageType,
    auto&&... args
)
{
    ::network::Message<UserMessageType> message{
        ::std::forward<decltype(messageType)>(messageType),
        ::std::forward<decltype(args)>(args)...
    };
    for (auto& client : m_connections) {
        client->tcp.send(message);
    }
}

template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::server::AServer<UserMessageType>::tcpSendToAllClients(
    UserMessageType messageType,
    auto&&... args
)
{
    ::network::Message<UserMessageType> message{
        ::std::forward<decltype(messageType)>(messageType),
        ::std::forward<decltype(args)>(args)...
    };
    for (auto& client : m_connections) {
        client->tcp.send(message);
    }
}

template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::server::AServer<UserMessageType>::tcpSendToAllClients(
    const ::network::Message<UserMessageType>& message
)
{
    for (auto& client : m_connections) {
        client->tcp.send(message);
    }
}

template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::server::AServer<UserMessageType>::tcpSendToAllClients(
    const ::network::Message<UserMessageType>& message,
    ::detail::constraint::sameAs<::std::shared_ptr<::network::Connection<UserMessageType>>> auto... ignoredClients
)
{
    for (auto& client : m_connections) {
        if (((client != ignoredClients) && ...)) {
            client->tcp.send(message);
        }
    }
}

template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::server::AServer<UserMessageType>::tcpSendToAllClients(
    const ::network::Message<UserMessageType>& message,
    ::detail::constraint::sameAs<::detail::Id> auto... ignoredClientIds
)
{
    for (auto& client : m_connections) {
        if (((client.getId()() != ignoredClientIds) && ...)) {
            client->tcp.send(message);
        }
    }
}



// ------------------------------------------------------------------ udpSend

template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::server::AServer<UserMessageType>::udpSendToClient(
    ::network::Message<UserMessageType>& message,
    ::std::shared_ptr<::network::Connection<UserMessageType>> client
)
{
    client->udpSendToClient(message);
}

template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::server::AServer<UserMessageType>::udpSendToClient(
    ::network::Message<UserMessageType>&& message,
    ::std::shared_ptr<::network::Connection<UserMessageType>> client
)
{
    client->udpSendToClient(::std::move(message));
}

template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::server::AServer<UserMessageType>::udpSendToClient(
    ::network::Message<UserMessageType>& message,
    ::detail::Id clientId
)
{
    this->getConnection(clientId)->udpSendToClient(message);
}

template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::server::AServer<UserMessageType>::udpSendToClient(
    ::network::Message<UserMessageType>&& message,
    ::detail::Id clientId
)
{
    this->getConnection(clientId)->udpSendToClient(::std::move(message));
}

template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::server::AServer<UserMessageType>::udpSendToClient(
    const ::network::Message<UserMessageType>& message,
    ::detail::constraint::sameAs<::std::shared_ptr<::network::Connection<UserMessageType>>> auto... clients
)
{
    for (auto& client : {clients...}) {
        client->udp.send(::network::Message<UserMessageType>{ message });
    }
}

template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::server::AServer<UserMessageType>::udpSendToAllClients(
    ::network::Message<UserMessageType>::SystemType messageType,
    auto&&... args
)
{
    ::network::Message<UserMessageType> message{
        ::std::forward<decltype(messageType)>(messageType),
        ::std::forward<decltype(args)>(args)...
    };
    for (auto& client : m_connections) {
        client->udp.send(message);
    }
}

template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::server::AServer<UserMessageType>::udpSendToAllClients(
    UserMessageType messageType,
    auto&&... args
)
{
    ::network::Message<UserMessageType> message{
        ::std::forward<decltype(messageType)>(messageType),
        ::std::forward<decltype(args)>(args)...
    };
    for (auto& client : m_connections) {
        client->udp.send(message);
    }
}

template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::server::AServer<UserMessageType>::udpSendToAllClients(
    const ::network::Message<UserMessageType>& message
)
{
    for (auto& client : m_connections) {
        client->udp.send(message);
    }
}

template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::server::AServer<UserMessageType>::udpSendToAllClients(
    const ::network::Message<UserMessageType>& message,
    ::detail::constraint::sameAs<::std::shared_ptr<::network::Connection<UserMessageType>>> auto... ignoredClients
)
{
    for (auto& client : m_connections) {
        if (((client != ignoredClients) && ...)) {
            client->udp.send(message);
        }
    }
}

template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::server::AServer<UserMessageType>::udpSendToAllClients(
    const ::network::Message<UserMessageType>& message,
    ::detail::constraint::sameAs<::detail::Id> auto... ignoredClientIds
)
{
    for (auto& client : m_connections) {
        if (((client.getId()() != ignoredClientIds) && ...)) {
            client->udp.send(message);
        }
    }
}



// ------------------------------------------------------------------ receive behaviour

template <
    ::detail::constraint::isEnum UserMessageType
> auto ::network::server::AServer<UserMessageType>::defaultReceiveBehaviour(
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
        case ::network::Message<UserMessageType>::SystemType::sharableInformations: {
            auto informationsIndex{ message.template pull<::network::SharableInformations::Index>() };
            switch (informationsIndex) {
            case ::network::SharableInformations::Index::name:
                this->setInformation<::network::SharableInformations::Index::name>(
                    connection,
                    message.template pull<::std::string>()
                );
                break;
            default:
                ::std::cerr << "[ERROR:Server:TCP:" << connection->getId() << "]: "
                    << "Unwnown informations index.";
                connection->disconnect();
                break;
            }
            return true;
        } case ::network::Message<UserMessageType>::SystemType::allSharableInformations:
            message.push(this->getSharableInformations());
            connection->tcp.send(::std::move(message));
            return true;
        case ::network::Message<UserMessageType>::SystemType::ping:
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



// ------------------------------------------------------------------ user methods

template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::server::AServer<UserMessageType>::onDisconnect(
    ::std::shared_ptr<::network::Connection<UserMessageType>> disconnectedConnection
)
{
    ::std::cout << '[' << disconnectedConnection->getId() << "] Disconnected.\n";
    ::std::erase_if(
        m_connections,
        [disconnectedConnection](const auto& connection){ return connection == disconnectedConnection; }
    );
}

template <
    ::detail::constraint::isEnum UserMessageType
> auto ::network::server::AServer<UserMessageType>::onAuthentification(
    ::std::shared_ptr<::network::Connection<UserMessageType>> connection
) -> bool
{
    // TODO: onAuthentification give reson why it failed
    // TODO: onIdentification and onAuthentification max number fail attempts
    ::std::cout << "[Server:TCP:" << connection->getId() << "] onAuthentification: \""
        << connection->getName() << "\".\n";
    if (connection->getName().size() < 3) {
        ::std::cerr << "[ERROR:Server:TCP:" << connection->getName()
            << ":onAuthentification] User name too short\n";
        return false;
    }
    for (const auto& validatedConnection : m_connections) {
        if (validatedConnection->getName() == connection->getName()) {
            ::std::cerr << "[ERROR:Server:TCP:" << connection->getId()
                << ":onAuthentification] User name already taken\n";
            return false;
        }
    }
    return true;
}

template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::server::AServer<UserMessageType>::onConnectionValidated(
    ::std::shared_ptr<::network::Connection<UserMessageType>> connection
)
{
    ::std::cout << "[Server:TCP:" << connection->getId() << "] onConnectionValidated.\n";

    connection->tcp.startReceivingMessage();
    connection->udp.startReceivingMessage();
    m_incommingConnections.erase(::std::ranges::find(m_incommingConnections, connection));
    m_connections.push_back(connection);

    connection->tcp.send(
        ::network::Message<UserMessageType>::SystemType::allSharableInformations,
        this->getSharableInformations()
    );
    this->tcpSendToAllClients(
        ::network::Message<UserMessageType>{
            ::network::Message<UserMessageType>::SystemType::newConnection,
            connection->getSharableInformations(),
            connection->getId()
        },
        connection
    );
}



template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::server::AServer<UserMessageType>::onAuthentificationDenial(
    ::std::shared_ptr<::network::Connection<UserMessageType>> connection
)
{
    ::std::cout << "[Server:TCP:" << connection->getId() << "] onAuthentificationDenial.\n";
}



// ------------------------------------------------------------------ others

template <
    ::detail::constraint::isEnum UserMessageType
> auto ::network::server::AServer<UserMessageType>::getConnection(
    ::detail::Id id
) -> ::std::shared_ptr<::network::Connection<UserMessageType>>
{
    return *::std::ranges::find_if(
        m_connections,
        [id](const auto& connection){ return connection->getId()() == id; }
    );
}

template <
    ::detail::constraint::isEnum UserMessageType
> auto ::network::server::AServer<UserMessageType>::getSharableInformations()
    -> ::std::map<::detail::Id, ::network::SharableInformations>
{
    ::std::map<::detail::Id, ::network::SharableInformations> map;
    for (const auto& connection : m_connections) {
        map.emplace(::std::make_pair(connection->getId(), connection->getSharableInformations()));
    }
    return map;
}

template <
    ::detail::constraint::isEnum UserMessageType
> template <
    ::network::SharableInformations::Index indexValue
> void ::network::server::AServer<UserMessageType>::setInformation(
    ::detail::Id id,
    auto&&... args
)
{
    this->setInformation<indexValue>(
        this->getConnection(id),
        ::std::forward<decltype(args)>(args)...
    );
}

template <
    ::detail::constraint::isEnum UserMessageType
> template <
    ::network::SharableInformations::Index indexValue
> void ::network::server::AServer<UserMessageType>::setInformation(
    ::std::shared_ptr<::network::Connection<UserMessageType>> connection,
    auto&&... args
)
{
    connection->template setSharableInformation<indexValue>(::std::forward<decltype(args)>(args)...);
    this->tcpSendToAllClients(
        ::network::Message<UserMessageType>{
            ::network::Message<UserMessageType>::SystemType::sharableInformations,
            ::std::forward<decltype(args)>(args)...,
            connection->getId(),
            indexValue
        },
        connection
    );
}
