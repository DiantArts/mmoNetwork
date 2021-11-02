#pragma once

#include <Detail/Queue.hpp>
#include <Detail/Id.hpp>
#include <Detail/Concepts.hpp>
#include <Network/Message.hpp>
#include <Network/OwnedMessage.hpp>

#ifdef ENABLE_ENCRYPTION
#include <Security/Cipher.hpp>
#endif

namespace network { template <::detail::isEnum UserMessageType> class ANode; }



namespace network {



template <
    ::detail::isEnum UserMessageType
> class TcpConnection
    : public ::std::enable_shared_from_this<TcpConnection<UserMessageType>>
{

public:

    // ------------------------------------------------------------------ *structors

    TcpConnection(
        ::network::ANode<UserMessageType>& owner,
        ::asio::ip::tcp::socket socket
    );

    // doesnt call onDisconnect
    ~TcpConnection();



    // ------------------------------------------------------------------ async - connection

    auto connectToClient(
        ::detail::Id id
    ) -> bool;

    void connect(
        const ::std::string& host,
        ::std::uint16_t port
    );

    void disconnect();

    [[ nodiscard ]] auto isConnected() const
        -> bool;



    // ------------------------------------------------------------------ async - out

    void send(
        UserMessageType messageType,
        auto&&... args
    );

    void send(
        ::network::Message<UserMessageType> message
    );

    bool hasSendingMessagesAwaiting() const;

    void writeAwaitingMessages();



    // ------------------------------------------------------------------ async - in

    void startReadMessage();

    void pullIncommingMessage();

    void pullIncommingMessages();

    void blockingPullIncommingMessages();



    // ------------------------------------------------------------------ other

    // TODO: create struct
    [[ nodiscard ]] auto getSharableInformations() const
        -> ::std::pair<::std::string, ::detail::Id>;

    [[ nodiscard ]] auto getId() const
        -> ::detail::Id;

    [[ nodiscard ]] auto getOwner() const
        -> const ::network::ANode<UserMessageType>&;

    [[ nodiscard ]] auto getPort() const
        -> ::std::uint16_t;

    [[ nodiscard ]] auto getAddress() const
        -> ::std::string;

    [[ nodiscard ]] auto getUserName() const
        -> const ::std::string&;

    void setUserName(
        ::std::string newName
    );



private:

    // ------------------------------------------------------------------ async - out

    template <
        auto successCallback,
        auto failureHeaderCallback,
        auto failureBodyCallback
    > void sendMessage(
        ::network::Message<UserMessageType> message
    );

    template <
        typename Type,
        auto successCallback,
        auto failureCallback
    > void sendRawData(
        auto&&... args
    );

    template <
        auto successCallback,
        auto failureCallback
    > void sendRawData(
        ::detail::isPointer auto pointerToData,
        ::std::size_t dataSize
    );



    // ------------------------------------------------------------------ async - in

    template <
        auto successCallback,
        auto failureHeaderCallback,
        auto failureBodyCallback
    > void receiveMessage();

    template <
        typename Type,
        auto successCallback,
        auto failureCallback
    > void receiveRawData();

    template <
        auto successCallback,
        auto failureCallback
    > void receiveToRawData(
        ::detail::isPointer auto pointerToData,
        ::std::size_t dataSize
    );

    void readBody();

    void transferBufferToInQueue();



    // ------------------------------------------------------------------ async - identification
    // Identification (Client claiming to identify as a client of the protocol):
    //     1. Both send the public key
    //     2. The server sends an handshake encrypted
    //     3. The client resolves and sends the handshake back encrypted

    void identification();

    void sendIdentificationDenied();


    void sendPublicKey();

    void readPublicKey();



#ifdef ENABLE_ENCRYPTION

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

#endif // ENABLE_ENCRYPTION



    void serverSendIdentificationAcceptance();

    void clientWaitIdentificationAcceptance();



    void sendIdentificationDenial();



    // ------------------------------------------------------------------ async - Authentification
    // Authentification (Client registering with some provable way that they are who the claim to be):
    //     1. Username
    //     2. TODO: password

    void serverAuthentification();

    void serverReceiveAuthentification();

    void serverSendAuthentificationAcceptance();



    void clientAuthentification();

    void clientSendAuthentification();

    void clientReceiveAuthentificationAcceptance();



    void sendAuthentificationDenial();



private:

    ::network::ANode<UserMessageType>& m_owner;

    // tcp
    ::asio::ip::tcp::socket m_socket;
    ::network::Message<UserMessageType> m_bufferIn;
    ::detail::Queue<::network::Message<UserMessageType>> m_messagesOut;

    // security module
#ifdef ENABLE_ENCRYPTION
    ::security::Cipher m_cipher;
#endif // ENABLE_ENCRYPTION

    ::detail::Id m_id{ 1 };

    bool m_isValid{ false };

    ::std::string m_userName;

};



} // namespace network

#include <Network/TcpConnection.impl.hpp>
