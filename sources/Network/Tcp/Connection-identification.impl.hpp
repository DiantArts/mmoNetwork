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
        ::network::Message<UserMessageType>::SystemType::publicKey, m_cipher.getPublicKey()
    });

    // read public key
    this->receiveMessage<[](::network::tcp::Connection<UserMessageType>& self){
        if (
            self.m_bufferIn.getTypeAsSystemType() ==
            ::network::Message<UserMessageType>::SystemType::publicKey
        ) {
            self.m_cipher.setTargetPublicKey(self.m_bufferIn.template pull<::security::Cipher::PublicKey>());
            if (self.m_owner.getType() == ::network::ANode<UserMessageType>::Type::server) {
                self.serverHandshake();
            } else {
                self.clientHandshake();
            }
        } else {
            ::std::cerr << "[ERROR:Identification:TCP:" << self.m_id << "] Identification failed, "
                << "unexpected message received: " << self.m_bufferIn.getTypeAsInt() << ".\n";
            self.disconnect();
        }
    }>();

#else // ENABLE_ENCRYPTION

    if (m_owner.getType() == ::network::ANode<UserMessageType>::Type::server) {
        this->serverSendIdentificationAcceptance();
    } else {
        this->clientWaitIdentificationAcceptance();
    }
    ::std::cerr << "[Connection:TCP:" << m_id << "] Identification ignored.\n";

#endif // ENABLE_ENCRYPTION
}

template <
    ::detail::isEnum UserMessageType
> void ::network::tcp::Connection<UserMessageType>::sendIdentificationDenial()
{
    this->sendMessage<[](::network::tcp::Connection<UserMessageType>& self){
        self.disconnect();
    }>(::network::Message<UserMessageType>{
        ::network::Message<UserMessageType>::SystemType::identificationAccepted, m_id
    });
}



#ifdef ENABLE_ENCRYPTION



// ------------------------------------------------------------------ async - Server identification

template <
    ::detail::isEnum UserMessageType
> void ::network::tcp::Connection<UserMessageType>::serverHandshake()
{
    auto baseValue{ m_cipher.generateRandomData(1024) };
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
    // m_cipher.encrypt(baseValue);
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
    m_cipher.scramble(baseValue);
    this->receiveMessage<[](
        ::network::tcp::Connection<UserMessageType>& self,
        ::std::vector<::std::byte> baseValue
    ){
        if (
            self.m_bufferIn.getTypeAsSystemType() ==
            ::network::Message<UserMessageType>::SystemType::resolveHandshake
        ) {
            auto receivedValue{ self.m_bufferIn.template pull<::std::vector<::std::byte>>() };
            // self.m_cipher.decrypt(receivedValue);
            if (receivedValue == baseValue) {
                if (self.m_owner.onIdentification(self.shared_from_this())) {
                    self.serverSendIdentificationAcceptance();
                    ::std::cerr << "[Connection:TCP:" << self.m_id << "] Identification successful.\n";
                } else {
                    self.m_owner.onIdentificationDenial(self.shared_from_this());
                    self.sendIdentificationDenial();
                }
            } else {
                ::std::cerr << "[ERROR:Identification:TCP:" << self.m_id
                    << "] Handshake failed, incorrect value\n";
                self.m_owner.onIdentificationDenial(self.shared_from_this());
                self.sendIdentificationDenial();
            }
        } else {
            ::std::cerr << "[ERROR:Identification:TCP:" << self.m_id << "] Handshake failed, "
                << "unexpected message received: " << self.m_bufferIn.getTypeAsInt() << ".\n";
            self.disconnect();
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
    this->receiveMessage<[](::network::tcp::Connection<UserMessageType>& self){
        if (
            self.m_bufferIn.getTypeAsSystemType() ==
            ::network::Message<UserMessageType>::SystemType::proposeHandshake
        ) {
            auto receivedValue{ self.m_bufferIn.template pull<::std::vector<::std::byte>>() };
            // self.m_cipher.decrypt(receivedValue);
            self.clientResolveHandshake(::std::move(receivedValue));
        } else {
            ::std::cerr << "[ERROR:Identification:TCP:" << self.m_id << "] Handshake failed, "
                << "unexpected message received: " << self.m_bufferIn.getTypeAsInt() << ".\n";

            self.disconnect();
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
    m_cipher.scramble(receivedValue);
    // m_cipher.encrypt(receivedValue);

    this->sendMessage<[](::network::tcp::Connection<UserMessageType>& self){
        if (self.m_owner.onIdentification(self.shared_from_this())) {
            self.clientWaitIdentificationAcceptance();
        } else {
            self.disconnect();
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

    this->sendMessage<[](::network::tcp::Connection<UserMessageType>& self){
        self.serverAuthentification();
    }>(::network::Message<UserMessageType>{
        ::network::Message<UserMessageType>::SystemType::identificationAccepted, m_id
    });
}



// ------------------------------------------------------------------ async - Client identificationAcceptance

// identification-etape:7
template <
    ::detail::isEnum UserMessageType
> void ::network::tcp::Connection<UserMessageType>::clientWaitIdentificationAcceptance()
{
    this->receiveMessage<[](::network::tcp::Connection<UserMessageType>& self){
        if (
            self.m_bufferIn.getTypeAsSystemType() ==
            ::network::Message<UserMessageType>::SystemType::identificationAccepted
        ) {
            self.m_bufferIn.pull(self.m_id);
#ifdef ENABLE_ENCRYPTION
            ::std::cerr << "[Connection:TCP:" << self.m_id << "] Identification successful.\n";
#endif // ENABLE_ENCRYPTION
            self.clientAuthentification();
        } else if (
            self.m_bufferIn.getTypeAsSystemType() ==
            ::network::Message<UserMessageType>::SystemType::identificationDenied
        ) {
            self.m_owner.onIdentificationDenial(self.shared_from_this());
            self.disconnect();
        } else {
            ::std::cerr << "[ERROR:Identification:TCP:" << self.m_id << "] Identification acceptance failed"
                << ", unexpected message received: " << self.m_bufferIn.getTypeAsInt() << ".\n";
            self.disconnect();
        }
    }>();
}
