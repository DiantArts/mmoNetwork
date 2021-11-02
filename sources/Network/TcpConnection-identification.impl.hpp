#pragma once

// ------------------------------------------------------------------ async - Common identification

template <
    ::detail::isEnum UserMessageType
> void ::network::TcpConnection<UserMessageType>::identification()
{
#ifdef ENABLE_ENCRYPTION
    // send public key
    this->sendRawData<
        [](...){},
        [](::network::TcpConnection<UserMessageType>& self, const ::std::error_code& errorCode){
            ::std::cerr << "[ERROR:Identification:TCP:" << self.m_id << "] Write header failed: "
                << errorCode.message() << ".\n";
            self.disconnect();
        }
    >(m_cipher.getPublicKeyAddr(), m_cipher.getPublicKeySize());

    // read public key
    this->receiveToRawData<
        [](::network::TcpConnection<UserMessageType>& self){
            if (self.m_owner.getType() == ::network::ANode<UserMessageType>::Type::server) {
                self.serverHandshake();
            } else {
                self.clientHandshake();
            }
        },
        [](::network::TcpConnection<UserMessageType>& self, const ::std::error_code& errorCode){
            ::std::cerr << "[ERROR:Identification:TCP:" << self.m_id << "] Read public key failed: "
                << errorCode.message() << ".\n";
            self.disconnect();
        }
    >(m_cipher.getTargetPublicKeyAddr(), m_cipher.getPublicKeySize());
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
> void ::network::TcpConnection<UserMessageType>::sendIdentificationDenial()
{
    this->sendMessage<
        [](::network::TcpConnection<UserMessageType>& self){
            self.disconnect();
        },
        [](::network::TcpConnection<UserMessageType>& self, const ::std::error_code& errorCode){
            ::std::cerr << "[ERROR:Identification:TCP:" << self.m_id
                << "] Send identificaion denied header failed: " << errorCode.message() << ".\n";
            self.disconnect();
        },
        [](::network::TcpConnection<UserMessageType>& self, const ::std::error_code& errorCode){
            ::std::cerr << "[ERROR:Identification:TCP:" << self.m_id
                << "] Send identificaion denied body failed: " << errorCode.message() << ".\n";
            self.disconnect();
        }
    >(::network::Message<UserMessageType>{
        ::network::Message<UserMessageType>::SystemType::identificationAccepted, m_id
    });
}



#ifdef ENABLE_ENCRYPTION



// ------------------------------------------------------------------ async - Server identification

template <
    ::detail::isEnum UserMessageType
> void ::network::TcpConnection<UserMessageType>::serverHandshake()
{
    // TODO generating random data through sodium
    auto baseValue{ static_cast<::std::uint64_t>(
        ::std::chrono::system_clock::now().time_since_epoch().count()
    ) };
    this->serverSendHandshake(m_cipher.encrypt(&baseValue, sizeof(::std::uint64_t)));
    this->serverReadHandshake(
        baseValue,
        new ::std::array<::std::byte, ::security::Cipher::getEncryptedSize(sizeof(::std::uint64_t))>
    );
}

template <
    ::detail::isEnum UserMessageType
> void ::network::TcpConnection<UserMessageType>::serverSendHandshake(
    ::std::vector<::std::byte>&& encryptedBaseValue
)
{
    this->sendRawData<
        [](...){},
        [](::network::TcpConnection<UserMessageType>& self, const ::std::error_code& errorCode){
            ::std::cerr << "[ERROR:Identification:TCP:" << self.m_id << "] Write handshake failed: "
                << errorCode.message() << ".\n";
            self.disconnect();
        }
    >(encryptedBaseValue.data(), encryptedBaseValue.size());
}

template <
    ::detail::isEnum UserMessageType
> void ::network::TcpConnection<UserMessageType>::serverReadHandshake(
    ::std::uint64_t& baseValue,
    ::std::array<
        ::std::byte,
        ::security::Cipher::getEncryptedSize(sizeof(::std::uint64_t))
    >* receivedValue
)
{
    ::asio::async_read(
        m_socket,
        ::asio::buffer(receivedValue->data(), receivedValue->size()),
        [
            this,
            baseValue,
            receivedValue
        ](
            const ::std::error_code& errorCode,
            const ::std::size_t length
        ) {
            if (errorCode) {
                if (errorCode == ::asio::error::operation_aborted) {
                    ::std::cerr << "[Connection:TCP] Operation canceled\n";
                } else {
                    ::std::cerr << "[ERROR:Identification:TCP:" << m_id << "] Read handshake failed: "
                        << errorCode.message() << ".\n";
                    this->disconnect();
                }
            } else {
                auto decryptedValue{
                    *reinterpret_cast<::std::uint64_t*>(
                        m_cipher.decrypt(receivedValue->data(), receivedValue->size()).data()
                    )
                };
                if (decryptedValue == m_cipher.scramble(baseValue)) {
                    if (m_owner.onIdentification(this->shared_from_this())) {
                        this->serverSendIdentificationAcceptance();
                        ::std::cerr << "[Connection:TCP:" << m_id << "] Identification successful.\n";
                    } else {
                        m_owner.onIdentificationDenial(this->shared_from_this());
                        this->sendIdentificationDenial();
                    }
                } else {
                    ::std::cerr << "[ERROR:Identification:TCP:" << m_id
                        << "] Handshake failed, incorrect value\n";
                    m_owner.onIdentificationDenial(this->shared_from_this());
                    this->sendIdentificationDenial();
                }
            }
            delete receivedValue;
        }
    );
}



// ------------------------------------------------------------------ async - Client identification

template <
    ::detail::isEnum UserMessageType
> void ::network::TcpConnection<UserMessageType>::clientHandshake()
{
    this->clientReadHandshake(
        new ::std::array<::std::byte, ::security::Cipher::getEncryptedSize(sizeof(::std::uint64_t))>
    );
}

template <
    ::detail::isEnum UserMessageType
> void ::network::TcpConnection<UserMessageType>::clientReadHandshake(
    ::std::array<
        ::std::byte,
        ::security::Cipher::getEncryptedSize(sizeof(::std::uint64_t))
    >* receivedValue
)
{
    ::asio::async_read(
        m_socket,
        ::asio::buffer(receivedValue->data(), receivedValue->size()),
        [
            this,
            receivedValue
        ](
            const ::std::error_code& errorCode,
            const ::std::size_t length
        ) {
            if (errorCode) {
                if (errorCode == ::asio::error::operation_aborted) {
                    ::std::cerr << "[Connection:TCP] Operation canceled\n";
                } else {
                    ::std::cerr << "[ERROR:Identification:TCP:" << m_id << "] Read handshake failed: "
                        << errorCode.message() << ".\n";
                    this->disconnect();
                }
            } else {
                this->clientResolveHandshake(receivedValue);
            }
        }
    );
}

template <
    ::detail::isEnum UserMessageType
> void ::network::TcpConnection<UserMessageType>::clientResolveHandshake(
    ::std::array<
        ::std::byte,
        ::security::Cipher::getEncryptedSize(sizeof(::std::uint64_t))
    >* receivedValue
)
{
    auto handshakeReceived{ m_cipher.decrypt(receivedValue->data(), receivedValue->size()) };
    auto handshakeResolved{ m_cipher.scramble(*reinterpret_cast<::std::uint64_t*>(handshakeReceived.data())) };
    auto handshakeResolvedEncrypted{ m_cipher.encrypt(&handshakeResolved, sizeof(handshakeResolved)) };
    delete receivedValue;
    ::asio::async_write(
        m_socket,
        ::asio::buffer(handshakeResolvedEncrypted.data(), handshakeResolvedEncrypted.size()),
        [this](
            const ::std::error_code& errorCode,
            const ::std::size_t length
        ) {
            if (errorCode) {
                if (errorCode == ::asio::error::operation_aborted) {
                    ::std::cerr << "[Connection:TCP] Operation canceled\n";
                } else {
                    ::std::cerr << "[ERROR:Identification:TCP:" << m_id << "] Write handshake failed: "
                        << errorCode.message() << ".\n";
                    this->disconnect();
                }
            } else {
                if (m_owner.onIdentification(this->shared_from_this())) {
                    this->clientWaitIdentificationAcceptance();
                } else {
                    this->disconnect();
                }
            }
        }
    );
}



#endif // ENABLE_ENCRYPTION



// ------------------------------------------------------------------ async - Server identificationAcceptance

template <
    ::detail::isEnum UserMessageType
> void ::network::TcpConnection<UserMessageType>::serverSendIdentificationAcceptance()
{

    this->sendMessage<
        [](::network::TcpConnection<UserMessageType>& self){
            self.serverAuthentification();
        },
        [](::network::TcpConnection<UserMessageType>& self, const ::std::error_code& errorCode){
            ::std::cerr << "[ERROR:Identification:TCP:" << self.m_id
                << "] Send identificaion acceptance header failed: " << errorCode.message() << ".\n";
            self.disconnect();
        },
        [](::network::TcpConnection<UserMessageType>& self, const ::std::error_code& errorCode){
            ::std::cerr << "[ERROR:Identification:TCP:" << self.m_id
                << "] Send identificaion acceptance body failed: " << errorCode.message() << ".\n";
            self.disconnect();
        }
    >(::network::Message<UserMessageType>{
        ::network::Message<UserMessageType>::SystemType::identificationAccepted, m_id
    });
}



// ------------------------------------------------------------------ async - Client identificationAcceptance

template <
    ::detail::isEnum UserMessageType
> void ::network::TcpConnection<UserMessageType>::clientWaitIdentificationAcceptance()
{
    this->receiveMessage<
        [](::network::TcpConnection<UserMessageType>& self){
            if (
                self.m_bufferIn.getTypeAsSystemType() ==
                ::network::Message<UserMessageType>::SystemType::identificationAccepted
            ) {
                self.m_bufferIn.extract(self.m_id);
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
                ::std::cerr << "[ERROR:Identification:TCP:" << self.m_id << "] Identification failed, "
                    << "unexpected message received.\n";
                self.disconnect();
            }
        },
        [](::network::TcpConnection<UserMessageType>& self, const ::std::error_code& errorCode){
            ::std::cerr << "[ERROR:Identification:TCP:" << self.m_id
                << "] Identification acceptance failed: " << errorCode.message() << ".\n";
            self.disconnect();
        },
        [](::network::TcpConnection<UserMessageType>& self, const ::std::error_code& errorCode){
            ::std::cerr << "[ERROR:Identification:TCP:" << self.m_id
                << "] Identification acceptance failed: " << errorCode.message() << ".\n";
            self.disconnect();
        }
    >();
}
