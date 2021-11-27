#pragma once

// ------------------------------------------------------------------ *structors

template <
    ::detail::isEnum UserMessageType
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
    ::detail::isEnum UserMessageType
> ::network::server::AServer<UserMessageType>::~AServer()
{
    this->stop();
}



// ------------------------------------------------------------------ running

template <
    ::detail::isEnum UserMessageType
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
    ::detail::isEnum UserMessageType
> void ::network::server::AServer<UserMessageType>::stop()
{
    if (this->isRunning()) {
        this->stopThread();
        this->getIncommingMessages().notify();
        ::std::cout << "[Server] Stopped" << '\n';
    }
}

template <
    ::detail::isEnum UserMessageType
> auto ::network::server::AServer<UserMessageType>::isRunning()
    -> bool
{
    // TODO: isRunning implemetation
    return !this->getAsioContext().stopped();
}



// ------------------------------------------------------------------ in

template <
    ::detail::isEnum UserMessageType
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



// ------------------------------------------------------------------ out

template <
    ::detail::isEnum UserMessageType
> void ::network::server::AServer<UserMessageType>::send(
    ::network::Message<UserMessageType>& message,
    ::std::shared_ptr<::network::Connection<UserMessageType>> client
)
{
    client->send(message);
}

template <
    ::detail::isEnum UserMessageType
> void ::network::server::AServer<UserMessageType>::send(
    ::network::Message<UserMessageType>&& message,
    ::std::shared_ptr<::network::Connection<UserMessageType>> client
)
{
    client->send(::std::move(message));
}

template <
    ::detail::isEnum UserMessageType
> void ::network::server::AServer<UserMessageType>::send(
    ::network::Message<UserMessageType>& message,
    ::detail::Id clientId
)
{
    this->getConnection(clientId)->send(message);
}

template <
    ::detail::isEnum UserMessageType
> void ::network::server::AServer<UserMessageType>::send(
    ::network::Message<UserMessageType>&& message,
    ::detail::Id clientId
)
{
    this->getConnection(clientId)->send(::std::move(message));
}

template <
    ::detail::isEnum UserMessageType
> void ::network::server::AServer<UserMessageType>::send(
    const ::network::Message<UserMessageType>& message,
    ::detail::sameAs<::std::shared_ptr<::network::Connection<UserMessageType>>> auto... clients
)
{
    for (auto& client : {clients...}) {
        client->tcp.send(::network::Message<UserMessageType>{ message });
    }
}

template <
    ::detail::isEnum UserMessageType
> void ::network::server::AServer<UserMessageType>::sendToAllClients(
    const ::network::Message<UserMessageType>& message,
    ::detail::sameAs<::std::shared_ptr<::network::Connection<UserMessageType>>> auto... ignoredClients
)
{
    for (auto& client : m_connections) {
        if (((client != ignoredClients) && ...)) {
            client->tcp.send(::network::Message<UserMessageType>{ message });
        }
    }
}

template <
    ::detail::isEnum UserMessageType
> void ::network::server::AServer<UserMessageType>::sendToAllClients(
    const ::network::Message<UserMessageType>& message,
    ::detail::sameAs<::detail::Id> auto... ignoredClientIds
)
{
    for (auto& client : m_connections) {
        if (((client.informations.id() != ignoredClientIds) && ...)) {
            client->tcp.send(::network::Message<UserMessageType>{ message });
        }
    }
}



// ------------------------------------------------------------------ receive behaviour

template <
    ::detail::isEnum UserMessageType
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
        case ::network::Message<UserMessageType>::SystemType::ping:
            connection->tcp.send(::std::move(message));
            break;
        default: return false;
        }
    } else {
        switch (message.getTypeAsSystemType()) {
        case ::network::Message<UserMessageType>::SystemType::ping:
            connection->udp.send(::std::move(message));
            break;
        default: return false;
        }
    }
    return true;
}



// ------------------------------------------------------------------ user methods

template <
    ::detail::isEnum UserMessageType
> void ::network::server::AServer<UserMessageType>::onDisconnect(
    ::std::shared_ptr<::network::Connection<UserMessageType>> disconnectedConnection
)
{
    ::std::cout << '[' << disconnectedConnection->informations.userName << "] Disconnected.\n";
    ::std::erase_if(
        m_connections,
        [disconnectedConnection](const auto& connection){ return connection == disconnectedConnection; }
    );
}

template <
    ::detail::isEnum UserMessageType
> auto ::network::server::AServer<UserMessageType>::onAuthentification(
    ::std::shared_ptr<::network::Connection<UserMessageType>> connection
) -> bool
{
    // TODO: onAuthentification give reson why it failed
    // TODO: onIdentification and onAuthentification max number fail attempts
    ::std::cout << "[Server:TCP:" << connection->informations.userName << "] onAuthentification: \""
        << connection->informations.userName << "\".\n";
    if (connection->informations.userName.size() < 3) {
        ::std::cerr << "[ERROR:Server:TCP:" << connection->informations.userName
            << ":onAuthentification] User name too short\n";
        return false;
    }
    for (const auto& validatedConnection : m_connections) {
        if (validatedConnection->informations.userName == connection->informations.userName) {
            ::std::cerr << "[ERROR:Server:TCP:" << connection->informations.userName
                << ":onAuthentification] User name already taken\n";
            return false;
        }
    }
    return true;
}

template <
    ::detail::isEnum UserMessageType
> void ::network::server::AServer<UserMessageType>::onConnectionValidated(
    ::std::shared_ptr<::network::Connection<UserMessageType>> connection
)
{
    ::std::cout << "[Server:TCP:" << connection->informations.userName << "] onConnectionValidated.\n";
    // start reading/writing tcp
    connection->tcp.startReceivingMessage();
    connection->udp.startReceivingMessage();
    m_incommingConnections.erase(::std::ranges::find(m_incommingConnections, connection));
    m_connections.push_back(::std::move(connection));
}



template <
    ::detail::isEnum UserMessageType
> void ::network::server::AServer<UserMessageType>::onAuthentificationDenial(
    ::std::shared_ptr<::network::Connection<UserMessageType>> connection
)
{
    ::std::cout << "[Server:TCP:" << connection->informations.userName << "] onAuthentificationDenial.\n";
}



// ------------------------------------------------------------------ other

template <
    ::detail::isEnum UserMessageType
> [[ nodiscard ]] auto ::network::server::AServer<UserMessageType>::getConnection(
    ::detail::Id id
) -> ::std::shared_ptr<::network::Connection<UserMessageType>>
{
    return *::std::ranges::find_if(
        m_connections,
        [id](const auto& connection){ return connection->informations.id() == id; }
    );
}
