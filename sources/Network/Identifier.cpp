#include <pch.hpp>
#include <Network/Identifier.hpp>
#include <Network/Message.hpp>
#include <Network/OwnedMessage.hpp>
#include <Network/MessageType.hpp>
#include <Network/ANode.hpp>
#include <Network/Server/AServer.hpp>
#include <Network/Connection.hpp>



// ------------------------------------------------------------------ explicit instantiations

template class ::network::Identifier<::network::MessageType>;



// ------------------------------------------------------------------ caller

template <
    ::detail::isEnum MessageType
> void ::network::Identifier<MessageType>::identificate(
    ::network::Connection<MessageType>& connection
)
{
    Identifier<MessageType> identifier{ connection };

    // public keys
    identifier.sendPublicKey();
    identifier.receivePublicKey();
}



// ------------------------------------------------------------------ *structors

template <
    ::detail::isEnum MessageType
> ::network::Identifier<MessageType>::Identifier(
    ::network::Connection<MessageType>& connection
)
    : m_connection{ connection }
{}

template <
    ::detail::isEnum MessageType
> ::network::Identifier<MessageType>::~Identifier() = default;


// ------------------------------------------------------------------ public keys

template <
    ::detail::isEnum MessageType
> void ::network::Identifier<MessageType>::sendPublicKey()
{
    // send public key
    ::boost::asio::async_write(
        m_connection.m_tcpSocket,
        ::boost::asio::buffer(m_cipher.getPublicKeyAddr(), m_cipher.getPublicKeySize()),
        [this](
            const boost::system::error_code& errorCode,
            const ::std::size_t length
        ) {
            if (errorCode) {
                ::std::cerr << "[ERROR:" << m_connection.m_id << "] Write header failed: " << errorCode.message() << ".\n";
                m_connection.disconnect();
            }
        }
    );
}

template <
    ::detail::isEnum MessageType
> void ::network::Identifier<MessageType>::receivePublicKey()
{
    // read public key
    ::boost::asio::async_read(
        m_connection.m_tcpSocket,
        ::boost::asio::buffer(m_cipher.getTargetPublicKeyAddr(), m_cipher.getPublicKeySize()),
        [this](
            const boost::system::error_code& errorCode,
            const ::std::size_t length
        ) {
            if (errorCode) {
                ::std::cerr << "[ERROR:" << m_connection.m_id << "] Read public key failed: " << errorCode.message() << ".\n";
                m_connection.disconnect();
            } else {
                if (m_connection.m_owner.getType() == ::network::ANode<MessageType>::Type::server) {
                    return this->serverHandshake();
                } else {
                    return this->clientHandshake();
                }
                // no return means error
                m_connection.m_owner.onIdentificationDenial(m_connection.shared_from_this());
                m_connection.disconnect();
            }
        }
    );
}



// ------------------------------------------------------------------ handshake - Server

template <
    ::detail::isEnum MessageType
> void ::network::Identifier<MessageType>::serverHandshake()
{
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
    ::detail::isEnum MessageType
> void ::network::Identifier<MessageType>::serverSendHandshake(
    ::std::vector<::std::byte>&& encryptedBaseValue
)
{
    ::boost::asio::async_write(
        m_connection.m_tcpSocket,
        ::boost::asio::buffer(encryptedBaseValue.data(), encryptedBaseValue.size()),
        [this](
            const boost::system::error_code& errorCode,
            const ::std::size_t length
        ) {
            if (errorCode) {
                ::std::cerr << "[ERROR:" << m_connection.m_id << "] Write handshake failed: " << errorCode.message() << ".\n";
                m_connection.disconnect();
            }
        }
    );
}

template <
    ::detail::isEnum MessageType
> void ::network::Identifier<MessageType>::serverReadHandshake(
    ::std::uint64_t& baseValue,
    ::std::array<
        ::std::byte,
        ::security::Cipher::getEncryptedSize(sizeof(::std::uint64_t))
    >* receivedValue
)
{
    ::boost::asio::async_read(
        m_connection.m_tcpSocket,
        ::boost::asio::buffer(receivedValue->data(), receivedValue->size()),
        [
            this,
            baseValue,
            receivedValue
        ](
            const boost::system::error_code& errorCode,
            const ::std::size_t length
        ) {
            if (errorCode) {
                ::std::cerr << "[ERROR:" << m_connection.m_id << "] Read handshake failed: " << errorCode.message() << ".\n";
                m_connection.disconnect();
            } else {
                auto decryptedValue{
                    *reinterpret_cast<::std::uint64_t*>(
                        m_cipher.decrypt(receivedValue->data(), receivedValue->size()).data()
                    )
                };
                if (decryptedValue == m_cipher.scramble(baseValue)) {
                    if (
                        dynamic_cast<::network::AServer<MessageType>&>(m_connection.m_owner)
                            .onClientIdentificate(m_connection.shared_from_this())
                    ) {
                        this->sendIdentificationAcceptanceHeader();
                    } else {
                        m_connection.disconnect();
                    }
                } else {
                    ::std::cerr << "[ERROR:" << m_connection.m_id << "] Handshake failed, incorrect value\n";
                    m_connection.m_owner.onIdentificationDenial(m_connection.shared_from_this());
                    m_connection.disconnect();
                }
            }
            delete receivedValue;
        }
    );
}

template <
    ::detail::isEnum MessageType
> void ::network::Identifier<MessageType>::sendIdentificationAcceptanceHeader()
{
    // TODO: FK THIS ONE
    // m_connection.tcpSendMessage<
        // [](::network::Connection<MessageType>& self){
            // m_connection.m_isValid = true;
            // ::std::cout << "[" << m_connection.m_id << "] Identificated successfully.\n";
            // dynamic_cast<::network::AServer<MessageType>&>(self.m_connection.m_owner)
                // .validateConnection(self.m_connection.shared_from_this());
            // m_connection.startReadMessage();
        // },
        // [](::network::Identifier<MessageType>& self, const boost::system::error_code& errorCode){
            // ::std::cerr << "[ERROR:" << self.m_connection.m_id << "] Send identificaion acceptance header failed: "
                // << errorCode.message() << ".\n";
           // self.m_connection.disconnect();
        // },
        // [](::network::Identifier<MessageType>& self, const boost::system::error_code& errorCode){
            // ::std::cerr << "[ERROR:" << self.m_connection.m_id << "] Send identificaion acceptance body failed: "
                // << errorCode.message() << ".\n";
           // self.m_connection.disconnect();
        // }
    // >(
        // ::network::Message<MessageType>{
            // MessageType::identificationAccepted,
            // ::network::TransmissionProtocol::tcp,
            // ::std::uint16_t(m_connection.m_udpSocket.local_endpoint().port())
        // }
    // );
}



template <
    ::detail::isEnum MessageType
> void ::network::Identifier<MessageType>::clientHandshake()
{
    this->clientReadHandshake(
        new ::std::array<::std::byte, ::security::Cipher::getEncryptedSize(sizeof(::std::uint64_t))>
    );
}

template <
    ::detail::isEnum MessageType
> void ::network::Identifier<MessageType>::clientReadHandshake(
    ::std::array<
        ::std::byte,
        ::security::Cipher::getEncryptedSize(sizeof(::std::uint64_t))
    >* receivedValue
)
{
    ::boost::asio::async_read(
        m_connection.m_tcpSocket,
        ::boost::asio::buffer(receivedValue->data(), receivedValue->size()),
        [
            this,
            receivedValue
        ](
            const boost::system::error_code& errorCode,
            const ::std::size_t length
        ) {
            if (errorCode) {
                ::std::cerr << "[ERROR:" << m_connection.m_id << "] Read handshake failed: " << errorCode.message() << ".\n";
                m_connection.disconnect();
            } else {
                this->clientResolveHandshake(receivedValue);
            }
        }
    );
}

template <
    ::detail::isEnum MessageType
> void ::network::Identifier<MessageType>::clientResolveHandshake(
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
    ::boost::asio::async_write(
        m_connection.m_tcpSocket,
        ::boost::asio::buffer(handshakeResolvedEncrypted.data(), handshakeResolvedEncrypted.size()),
        [this](
            const boost::system::error_code& errorCode,
            const ::std::size_t length
        ) {
            if (errorCode) {
                ::std::cerr << "[ERROR:" << m_connection.m_id << "] Write handshake failed: " << errorCode.message() << ".\n";
                m_connection.disconnect();
            } else {
                this->clientWaitIdentificationAcceptanceHeader();
            }
        }
    );
}

// TODO: add timeout functionnalities
template <
    ::detail::isEnum MessageType
> void ::network::Identifier<MessageType>::clientWaitIdentificationAcceptanceHeader()
{
    ::boost::asio::async_read(
        m_connection.m_tcpSocket,
        ::boost::asio::buffer(m_connection.m_tcpBufferIn.getHeaderAddr(), m_connection.m_tcpBufferIn.getHeaderSize()),
        [this](
            const boost::system::error_code& errorCode,
            const ::std::size_t length
        ) {
            if (errorCode) {
                ::std::cerr << "[ERROR:" << m_connection.m_id << "] Iidentificaion acceptance failed: "
                    << errorCode.message() << ".\n";
                m_connection.disconnect();
            } else {
                if (m_connection.m_tcpBufferIn.getType() == MessageType::identificationAccepted) {
                    this->clientWaitIdentificationAcceptanceBody();
                } else if (m_connection.m_tcpBufferIn.getType() == MessageType::identificationDenied) {
                    ::std::cerr << "[ERROR:" << m_connection.m_id << "] Iidentificaion denied.\n";
                    m_connection.disconnect();
                } else {
                    ::std::cerr << "[ERROR:" << m_connection.m_id << "] Identificaion failed, "
                        << "unexpected message received.\n";
                    m_connection.disconnect();
                }
            }
        }
    );
}

template <
    ::detail::isEnum MessageType
> void ::network::Identifier<MessageType>::clientWaitIdentificationAcceptanceBody()
{
    m_connection.m_tcpBufferIn.resize(sizeof(::std::uint16_t));
    ::boost::asio::async_read(
        m_connection.m_tcpSocket,
        ::boost::asio::buffer(m_connection.m_tcpBufferIn.getBodyAddr(), m_connection.m_tcpBufferIn.getBodySize()),
        [this](
            const boost::system::error_code& errorCode,
            const ::std::size_t length
        ) {
            if (errorCode) {
                ::std::cerr << "[ERROR:" << m_connection.m_id << "] Iidentificaion acceptance failed: "
                    << errorCode.message() << ".\n";
                m_connection.disconnect();
            } else {
                m_connection.m_isValid = true;
                ::std::cerr << "[" << m_connection.m_id << "] Iidentificaion accepted.\n";
                m_connection.startReadMessage();
                if (!m_connection.m_tcpMessagesOut.empty()) {
                    m_connection.tcpWriteAwaitingMessages();
                }

                ::std::uint16_t udpPort;
                m_connection.m_tcpBufferIn >> udpPort;
                m_connection.targetServerUdpPort(udpPort);
                m_connection.udpReadHeader();
                if (!m_connection.m_udpMessagesOut.empty()) {
                    m_connection.udpWriteHeader();
                }
            }
        }
    );
}
