#pragma once

// ------------------------------------------------------------------ *structors

template <
    typename UserMessageType
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
    typename UserMessageType
> ::network::server::AServer<UserMessageType>::~AServer()
{
    this->stop();
}



// ------------------------------------------------------------------ running

template <
    typename UserMessageType
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
    typename UserMessageType
> void ::network::server::AServer<UserMessageType>::stop()
{
    if (this->isRunning()) {
        this->stopThread();
        this->getIncommingMessages().notify();
        ::std::cout << "[Server] Stopped" << '\n';
    }
}

template <
    typename UserMessageType
> auto ::network::server::AServer<UserMessageType>::isRunning()
    -> bool
{
    // TODO: isRunning implemetation
    return !this->getAsioContext().stopped();
}



// ------------------------------------------------------------------ in

template <
    typename UserMessageType
> void ::network::server::AServer<UserMessageType>::startReceivingConnections()
{
    m_asioAcceptor.async_accept(
        [this](
            const ::std::error_code& errorCode,
            ::asio::ip::tcp::socket socket
        ) {
            if (!errorCode) {
                ::std::cout << "[Server] New incomming connection: " << socket.remote_endpoint() << ".\n";
                auto newConnection{ ::std::make_shared<::network::tcp::Connection<UserMessageType>>(
                    *this,
                    ::std::move(socket)
                ) };
                if (newConnection->connectToClient(++m_idCounter)) {
                    m_incommingConnections.push_back(::std::move(newConnection));
                    ::std::cout << "[Server:TCP:" << m_incommingConnections.back()->getId()
                        << "] Connection started.\n";
                } else {
                    ::std::cerr << "[ERROR:Server] Connection failed.\n";
                }
            } else {
                ::std::cerr << "[ERROR:Server] New connection error: " << errorCode.message() << ".\n";
            }
            this->startReceivingConnections();
        }
    );
}



// ------------------------------------------------------------------ out

template <
    typename UserMessageType
> void ::network::server::AServer<UserMessageType>::send(
    ::network::Message<UserMessageType>& message,
    ::std::shared_ptr<::network::tcp::Connection<UserMessageType>> client
)
{
    client->send(message);
}

template <
    typename UserMessageType
> void ::network::server::AServer<UserMessageType>::send(
    ::network::Message<UserMessageType>&& message,
    ::std::shared_ptr<::network::tcp::Connection<UserMessageType>> client
)
{
    client->send(::std::move(message));
}

template <
    typename UserMessageType
> void ::network::server::AServer<UserMessageType>::send(
    ::network::Message<UserMessageType>& message,
    ::detail::Id clientId
)
{
    this->getConnection(clientId)->send(message);
}

template <
    typename UserMessageType
> void ::network::server::AServer<UserMessageType>::send(
    ::network::Message<UserMessageType>&& message,
    ::detail::Id clientId
)
{
    this->getConnection(clientId)->send(::std::move(message));
}

template <
    typename UserMessageType
> template <
    typename... Args
> void ::network::server::AServer<UserMessageType>::send(
    const ::network::Message<UserMessageType>& message,
    Args... clients
)
{
    for (auto& client : {clients...}) {
        client->send(message);
    }
}

template <
    typename UserMessageType
> template <
    typename... Args
> void ::network::server::AServer<UserMessageType>::sendToAllClients(
    const ::network::Message<UserMessageType>& message,
    Args... ignoredClients
)
{
    for (auto& client : m_connections) {
        if (((client != ignoredClients) && ...)) {
            client->send(message);
        }
    }
}



// ------------------------------------------------------------------ receive behaviour

template <
    typename UserMessageType
> auto ::network::server::AServer<UserMessageType>::defaultReceiveBehaviour(
    ::network::Message<UserMessageType>& message,
    ::std::shared_ptr<::network::tcp::Connection<UserMessageType>> connection
) -> bool
{
    switch (message.getTypeAsSystemType()) {
    case ::network::Message<UserMessageType>::SystemType::ping:
        connection->send(message);
        break;
    default:
        return false;
    }
    return true;
}

template <
    typename UserMessageType
> auto ::network::server::AServer<UserMessageType>::defaultReceiveBehaviour(
    ::network::Message<UserMessageType>& message,
    ::std::shared_ptr<::network::udp::Connection<UserMessageType>> connection
) -> bool
{
    switch (message.getTypeAsSystemType()) {
    case ::network::Message<UserMessageType>::SystemType::ping:
        connection->send(message);
        break;
    default:
        return false;
    }
    return true;
}



// ------------------------------------------------------------------ user methods

template <
    typename UserMessageType
> void ::network::server::AServer<UserMessageType>::onDisconnect(
    ::std::shared_ptr<::network::tcp::Connection<UserMessageType>> disconnectedConnection
)
{
    ::std::cout << '[' << disconnectedConnection->getId() << "] Disconnected.\n";
    m_connections.erase(::std::remove_if(
        m_connections.begin(),
        m_connections.end(),
        [
            disconnectedConnection
        ](
            const ::std::shared_ptr<::network::tcp::Connection<UserMessageType>>& connection
        ){
            return connection == disconnectedConnection;
        }
    ));
}

template <
    typename UserMessageType
> auto ::network::server::AServer<UserMessageType>::onAuthentification(
    ::std::shared_ptr<::network::tcp::Connection<UserMessageType>> connection
) -> bool
{
    // TODO: onAuthentification give reson why it failed
    // TODO: onIdentification and onAuthentification max number fail attempts
    ::std::cout << "[Server:TCP:" << connection->getId() << "] onAuthentification: \""
        << connection->getUserName() << "\".\n";
    if (connection->getUserName().size() < 3) {
        ::std::cerr << "[ERROR:Server:TCP:" << connection->getId()
            << ":onAuthentification] User name too short\n";
        return false;
    }
    for (const auto& validatedConnection : m_connections) {
        if (validatedConnection->getUserName() == connection->getUserName()) {
            ::std::cerr << "[ERROR:Server:TCP:" << connection->getId()
                << ":onAuthentification] User name already taken\n";
            return false;
        }
    }
    return true;
}

template <
    typename UserMessageType
> void ::network::server::AServer<UserMessageType>::onConnectionValidated(
    ::std::shared_ptr<::network::tcp::Connection<UserMessageType>> connection
)
{
    ::std::cout << "[Server:TCP:" << connection->getId() << "] onConnectionValidated.\n";
    // start reading/writing tcp
    connection->startReadMessage();
    // m_incommingConnections.erase(::std::ranges::find(m_incommingConnections, connection)); TODO
    m_connections.push_back(::std::move(connection));
}



template <
    typename UserMessageType
> void ::network::server::AServer<UserMessageType>::onAuthentificationDenial(
    ::std::shared_ptr<::network::tcp::Connection<UserMessageType>> connection
)
{
    ::std::cout << "[Server:TCP:" << connection->getId() << "] onAuthentificationDenial.\n";
}



// ------------------------------------------------------------------ other

template <
    typename UserMessageType
> [[ nodiscard ]] auto ::network::server::AServer<UserMessageType>::getConnection(
    ::detail::Id id
) -> ::std::shared_ptr<::network::tcp::Connection<UserMessageType>>
{
    return *::std::find_if(
        m_connections.begin(),
        m_connections.end(),
        [id](const auto& connection){ return connection->getId() == id; }
    );
}

template <
    typename UserMessageType
> auto ::network::server::AServer<UserMessageType>::getConnectionsSharableInformations() const
    -> ::std::vector<::std::pair<::std::string, ::detail::Id>>
{
    ::std::vector<::std::pair<::std::string, ::detail::Id>> sharableInformations{ m_connections.size() };
    for (const auto& connection : m_connections) {
        sharableInformations.push_back(connection->getSharableInformations());
    }
    return sharableInformations;
}
