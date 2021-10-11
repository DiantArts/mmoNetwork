// Identification (Client claiming to identify as a client of the protocol):
//     1. Both send the public key
//     2. The server sends an handshake encrypted
//     3. The client resolves and sends the handshake back encrypted
// TODO: Authentification
// Authentification (Client registering with some provable way that they are who the claim to be):
//     1. Username
//     2. password

#pragma once

#include <Detail/Queue.hpp>
#include <Security/Cipher.hpp>
#include <Detail/Id.hpp>
#include <Detail/Concepts.hpp>
#include <Network/Message.hpp>
#include <Network/OwnedMessage.hpp>

namespace network { template <::detail::isEnum MessageType> class Connection; }



namespace network {



template <
    ::detail::isEnum MessageType
> class Identifier
{

public:

    // ------------------------------------------------------------------ caller

    static void identificate(
        ::network::Connection<MessageType>& connection
    );




private:

    // ------------------------------------------------------------------ *structors

    Identifier(
        ::network::Connection<MessageType>& connection
    );

    ~Identifier();



    // ------------------------------------------------------------------ public keys

    void sendPublicKey();

    void receivePublicKey();



    // ------------------------------------------------------------------ handshake - Server

    void serverHandshake();

    void serverSendHandshake(
        ::std::vector<::std::byte>&& encryptedHandshakeBaseValue
    );

    void serverReadHandshake(
        ::std::uint64_t& handshakeBaseValue,
        ::std::array<
            ::std::byte,
            ::security::Cipher::getEncryptedSize(sizeof(::std::uint64_t))
        >* handshakeReceivedPtr
    );

    void sendIdentificationAcceptanceHeader();

    void sendIdentificationAcceptanceBody(
        ::network::Message<MessageType>* message
    );



    // ------------------------------------------------------------------ handshake - Client

    void clientHandshake();

    void clientReadHandshake(
        ::std::array<
            ::std::byte,
            ::security::Cipher::getEncryptedSize(sizeof(::std::uint64_t))
        >* handshakeReceivedPtr
    );

    void clientResolveHandshake(
        ::std::array<
            ::std::byte,
            ::security::Cipher::getEncryptedSize(sizeof(::std::uint64_t))
        >* handshakeReceivedPtr
    );

    void clientWaitIdentificationAcceptanceHeader();

    void clientWaitIdentificationAcceptanceBody();



private:

    ::network::Connection<MessageType>& m_connection;

    ::security::Cipher m_cipher;

};



} // namespace network
