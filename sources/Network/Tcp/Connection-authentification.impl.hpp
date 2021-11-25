#pragma once

// ------------------------------------------------------------------ async - Server Authentification

template <
    ::detail::isEnum UserMessageType
> void ::network::tcp::Connection<UserMessageType>::serverAuthentification()
{
    this->serverReceiveAuthentification();
}

template <
    ::detail::isEnum UserMessageType
> void ::network::tcp::Connection<UserMessageType>::serverReceiveAuthentification()
{
    this->receiveMessage<[](::std::shared_ptr<::network::Connection<UserMessageType>> connection){
        if (
            connection->tcp.m_bufferIn.getTypeAsSystemType() !=
            ::network::Message<UserMessageType>::SystemType::authentification
        ) {
            ::std::cerr << "[ERROR:TCP:" << connection->informations.id << "] Authentification failed, "
                << "unexpected message received.\n";
            return connection->disconnect();
        }
        auto password{ connection->tcp.m_bufferIn.template pull<::std::string>() };
        connection->setUserName(connection->tcp.m_bufferIn.template pull<::std::string>());
        if (!connection->m_owner.onAuthentification(connection)) {
            ::std::cerr << "[ERROR:TCP:" << connection->informations.id << "] Authentification failed, "
                << "onAuthentification returned false.\n";
            connection->m_owner.onAuthentificationDenial(connection);
            connection->tcp.sendAuthentificationDenial();
            return connection->tcp.serverAuthentification();
        }
        ::std::cerr << "[Connection:TCP:" << connection->informations.id << "] Authentification successful.\n";
        connection->tcp.serverSendAuthentificationAcceptance();
    }>();
}

template <
    ::detail::isEnum UserMessageType
> void ::network::tcp::Connection<UserMessageType>::serverSendAuthentificationAcceptance()
{
    this->sendMessage<[](::std::shared_ptr<::network::Connection<UserMessageType>> connection){
        ::std::cerr << "[Connection:TCP:" << connection->informations.id << "] Authentification successful.\n";
        connection->tcp.m_isSendAllowed = true;
        connection->m_owner.onConnectionValidated(connection);
        connection->tcp.m_blocker.notify_all();
    }>(::network::Message<UserMessageType>{
        ::network::Message<UserMessageType>::SystemType::authentificationAccepted
    });
}



// ------------------------------------------------------------------ async - Client Authentification
// TODO: mem error when closing the client after authentification denial

template <
    ::detail::isEnum UserMessageType
> void ::network::tcp::Connection<UserMessageType>::clientAuthentification()
{
    m_connection->m_owner.onAuthentification(m_connection);
    this->clientSendAuthentification();
}

template <
    ::detail::isEnum UserMessageType
> void ::network::tcp::Connection<UserMessageType>::clientSendAuthentification()
{
    this->sendMessage<[](::std::shared_ptr<::network::Connection<UserMessageType>> connection){
        connection->tcp.clientReceiveAuthentificationAcceptance();
    }>(::network::Message<UserMessageType>{
        ::network::Message<UserMessageType>::SystemType::authentification,
        m_connection->informations.userName,
        "password"s
    });
}

template <
    ::detail::isEnum UserMessageType
> void ::network::tcp::Connection<UserMessageType>::clientReceiveAuthentificationAcceptance()
{
    this->receiveMessage<[](::std::shared_ptr<::network::Connection<UserMessageType>> connection){
        if (
            connection->tcp.m_bufferIn.getTypeAsSystemType() ==
            ::network::Message<UserMessageType>::SystemType::authentificationDenied
        ) {
            connection->m_owner.onAuthentificationDenial(connection);
            connection->tcp.clientSendAuthentification();
        } else if (
            connection->tcp.m_bufferIn.getTypeAsSystemType() ==
            ::network::Message<UserMessageType>::SystemType::authentificationAccepted
        ) {
            ::std::cerr << "[Connection:TCP:" << connection->informations.id << "] Authentification successful.\n";
            connection->tcp.m_isSendAllowed = true;
            connection->m_owner.onConnectionValidated(connection);
            connection->tcp.m_blocker.notify_all();
        } else {
            ::std::cerr << "[Connection:TCP:" << connection->informations.id << "] invalid authentification acceptance\n";
            connection->disconnect();
        }
    }>();
}



// ------------------------------------------------------------------ async - Error Authentification

template <
    ::detail::isEnum UserMessageType
> void ::network::tcp::Connection<UserMessageType>::sendAuthentificationDenial()
{
    this->sendMessage<[](...){}>(::network::Message<UserMessageType>{
        ::network::Message<UserMessageType>::SystemType::authentificationDenied
    });
}
