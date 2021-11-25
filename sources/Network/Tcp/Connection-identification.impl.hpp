#pragma once

// ------------------------------------------------------------------ async - Common identification

// identification-etape:1
template <
    ::detail::isEnum UserMessageType
> void ::network::tcp::Connection<UserMessageType>::identification()
{
#ifdef ENABLE_ENCRYPTION
    // send public key
    this->sendMessage<[](...){}>(::network::Message<UserMessageType>{
        ::network::Message<UserMessageType>::SystemType::publicKey, m_connection->m_cipher.getPublicKey()
    });

    // read public key
    this->receiveMessage<[](::std::shared_ptr<::network::Connection<UserMessageType>> connection){
        if (
            connection->tcp.m_bufferIn.getTypeAsSystemType() ==
            ::network::Message<UserMessageType>::SystemType::publicKey
        ) {
            connection->m_cipher.setTargetPublicKey(connection->tcp.m_bufferIn.template pull<::security::Cipher::PublicKey>());
            if (connection->m_owner.getType() == ::network::ANode<UserMessageType>::Type::server) {
                connection->tcp.serverHandshake();
            } else {
                connection->tcp.clientHandshake();
            }
        } else {
            ::std::cerr << "[ERROR:Identification:TCP:" << connection->informations.id << "] Identification failed, "
                << "unexpected message received: " << connection->tcp.m_bufferIn.getTypeAsInt() << ".\n";
            connection->disconnect();
        }
    }>();

#else // ENABLE_ENCRYPTION

    if (m_owner.getType() == ::network::ANode<UserMessageType>::Type::server) {
        this->serverSendIdentificationAcceptance();
    } else {
        this->clientWaitIdentificationAcceptance();
    }
    ::std::cerr << "[Connection:TCP:" << m_connection->informations.id << "] Identification ignored.\n";

#endif // ENABLE_ENCRYPTION
}

template <
    ::detail::isEnum UserMessageType
> void ::network::tcp::Connection<UserMessageType>::sendIdentificationDenial()
{
    this->sendMessage<[](::std::shared_ptr<::network::Connection<UserMessageType>> connection){
        connection->disconnect();
    }>(::network::Message<UserMessageType>{
        ::network::Message<UserMessageType>::SystemType::identificationAccepted, m_connection->informations.id
    });
}



#ifdef ENABLE_ENCRYPTION



// ------------------------------------------------------------------ async - Server identification

template <
    ::detail::isEnum UserMessageType
> void ::network::tcp::Connection<UserMessageType>::serverHandshake()
{
    auto baseValue{ m_connection->m_cipher.generateRandomData(1024) };
    this->serverSendHandshake(::std::vector{ baseValue });
    this->serverReadHandshake(::std::move(baseValue));
}

// identification-etape:2
template <
    ::detail::isEnum UserMessageType
> void ::network::tcp::Connection<UserMessageType>::serverSendHandshake(
    ::std::vector<::std::byte>&& baseValue
)
{
    // m_connection->m_cipher.encrypt(baseValue);
    this->sendMessage<[](...){}>(::network::Message<UserMessageType>{
        ::network::Message<UserMessageType>::SystemType::proposeHandshake, ::std::move(baseValue)
    });
}

// identification-etape:5
template <
    ::detail::isEnum UserMessageType
> void ::network::tcp::Connection<UserMessageType>::serverReadHandshake(
    ::std::vector<::std::byte>&& baseValue
)
{
    m_connection->m_cipher.scramble(baseValue);
    this->receiveMessage<[](
        ::std::shared_ptr<::network::Connection<UserMessageType>> connection,
        ::std::vector<::std::byte> baseValue
    ){
        if (
            connection->tcp.m_bufferIn.getTypeAsSystemType() ==
            ::network::Message<UserMessageType>::SystemType::resolveHandshake
        ) {
            auto receivedValue{ connection->tcp.m_bufferIn.template pull<::std::vector<::std::byte>>() };
            // connection->m_cipher.decrypt(receivedValue);
            if (receivedValue == baseValue) {
                if (connection->m_owner.onIdentification(connection)) {
                    connection->tcp.serverSendIdentificationAcceptance();
                    ::std::cerr << "[Connection:TCP:" << connection->informations.id << "] Identification successful.\n";
                } else {
                    connection->m_owner.onIdentificationDenial(connection);
                    connection->tcp.sendIdentificationDenial();
                }
            } else {
                ::std::cerr << "[ERROR:Identification:TCP:" << connection->informations.id
                    << "] Handshake failed, incorrect value\n";
                connection->m_owner.onIdentificationDenial(connection);
                connection->tcp.sendIdentificationDenial();
            }
        } else {
            ::std::cerr << "[ERROR:Identification:TCP:" << connection->informations.id << "] Handshake failed, "
                << "unexpected message received: " << connection->tcp.m_bufferIn.getTypeAsInt() << ".\n";
            connection->disconnect();
        }
    }>(::std::move(baseValue));
}



// ------------------------------------------------------------------ async - Client identification

template <
    ::detail::isEnum UserMessageType
> void ::network::tcp::Connection<UserMessageType>::clientHandshake()
{
    this->clientReadHandshake();
}

// identification-etape:3
template <
    ::detail::isEnum UserMessageType
> void ::network::tcp::Connection<UserMessageType>::clientReadHandshake()
{
    this->receiveMessage<[](::std::shared_ptr<::network::Connection<UserMessageType>> connection){
        if (
            connection->tcp.m_bufferIn.getTypeAsSystemType() ==
            ::network::Message<UserMessageType>::SystemType::proposeHandshake
        ) {
            auto receivedValue{ connection->tcp.m_bufferIn.template pull<::std::vector<::std::byte>>() };
            // connection->m_cipher.decrypt(receivedValue);
            connection->tcp.clientResolveHandshake(::std::move(receivedValue));
        } else {
            ::std::cerr << "[ERROR:Identification:TCP:" << connection->informations.id << "] Handshake failed, "
                << "unexpected message received: " << connection->tcp.m_bufferIn.getTypeAsInt() << ".\n";

            connection->disconnect();
        }
    }>();
}

// identification-etape:4
template <
    ::detail::isEnum UserMessageType
> void ::network::tcp::Connection<UserMessageType>::clientResolveHandshake(
    ::std::vector<::std::byte>&& receivedValue
)
{
    m_connection->m_cipher.scramble(receivedValue);
    // m_cipher.encrypt(receivedValue);

    this->sendMessage<[](::std::shared_ptr<::network::Connection<UserMessageType>> connection){
        if (connection->m_owner.onIdentification(connection)) {
            connection->tcp.clientWaitIdentificationAcceptance();
        } else {
            connection->disconnect();
        }
    }>(::network::Message<UserMessageType>{
        ::network::Message<UserMessageType>::SystemType::resolveHandshake,
        ::std::move(receivedValue)
    });
}



#endif // ENABLE_ENCRYPTION



// ------------------------------------------------------------------ async - Server identificationAcceptance

// identification-etape:6
template <
    ::detail::isEnum UserMessageType
> void ::network::tcp::Connection<UserMessageType>::serverSendIdentificationAcceptance()
{

    this->sendMessage<[](::std::shared_ptr<::network::Connection<UserMessageType>> connection){
        connection->tcp.serverAuthentification();
    }>(::network::Message<UserMessageType>{
        ::network::Message<UserMessageType>::SystemType::identificationAccepted, m_connection->informations.id
    });
}



// ------------------------------------------------------------------ async - Client identificationAcceptance

// identification-etape:7
template <
    ::detail::isEnum UserMessageType
> void ::network::tcp::Connection<UserMessageType>::clientWaitIdentificationAcceptance()
{
    this->receiveMessage<[](::std::shared_ptr<::network::Connection<UserMessageType>> connection){
        if (
            connection->tcp.m_bufferIn.getTypeAsSystemType() ==
            ::network::Message<UserMessageType>::SystemType::identificationAccepted
        ) {
            connection->tcp.m_bufferIn.pull(connection->informations.id);
#ifdef ENABLE_ENCRYPTION
            ::std::cerr << "[Connection:TCP:" << connection->informations.id << "] Identification successful.\n";
#endif // ENABLE_ENCRYPTION
            connection->tcp.clientAuthentification();
        } else if (
            connection->tcp.m_bufferIn.getTypeAsSystemType() ==
            ::network::Message<UserMessageType>::SystemType::identificationDenied
        ) {
            connection->m_owner.onIdentificationDenial(connection);
            connection->disconnect();
        } else {
            ::std::cerr << "[ERROR:Identification:TCP:" << connection->informations.id << "] Identification acceptance failed"
                << ", unexpected message received: " << connection->tcp.m_bufferIn.getTypeAsInt() << ".\n";
            connection->disconnect();
        }
    }>();
}
