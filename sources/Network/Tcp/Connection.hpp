#pragma once

#include <Detail/Concepts.hpp>
#include <Detail/Queue.hpp>
#include <Network/Message.hpp>
#include <Network/OwnedMessage.hpp>

#ifdef ENABLE_ENCRYPTION
#include <Security/Cipher.hpp>
#endif

namespace network { template <::detail::isEnum UserMessageType> class ANode; }
namespace network { template <::detail::isEnum UserMessageType> class Connection; }



namespace network::tcp {



template <
    ::detail::isEnum UserMessageType
> class Connection {

public:

    // ------------------------------------------------------------------ *structors

    Connection(
        ::asio::ip::tcp::socket socket
    );

    // doesnt call onDisconnect
    ~Connection();



    // ------------------------------------------------------------------ async - connection

    auto startConnectingToClient()
        -> bool;

    void startConnectingToServer(
        const ::std::string& host,
        ::std::uint16_t port
    );

    void disconnect();

    [[ nodiscard ]] auto isConnected() const
        -> bool;

    void notify();

    void waitNotification();



    // ------------------------------------------------------------------ async - out

    void send(
        UserMessageType messageType,
        auto&&... args
    );

    void send(
        ::network::Message<UserMessageType> message
    );

    auto hasSendingMessagesAwaiting() const
        -> bool;

    void sendAwaitingMessages();



    // ------------------------------------------------------------------ async - in

    void startReceivingMessage();



    // ------------------------------------------------------------------ other

    [[ nodiscard ]] auto getPort() const
        -> ::std::uint16_t;

    [[ nodiscard ]] auto getAddress() const
        -> ::std::string;

    void assignConnection(
        ::std::shared_ptr<::network::Connection<UserMessageType>> connection
    );


private:

    // ------------------------------------------------------------------ async - out

    template <
        auto successCallback
    > void sendMessage(
        ::network::Message<UserMessageType> message,
        auto&&... args
    );

    template <
        auto successCallback
    > void syncSendMessage(
        ::network::Message<UserMessageType> message,
        auto&&... args
    );



    // ------------------------------------------------------------------ async - in

    template <
        auto successCallback
    > void receiveMessage(
        auto&&... args
    );

    template <
        auto successCallback
    > void syncReceiveMessage(
        auto&&... args
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



#ifdef ENABLE_ENCRYPTION

    void serverHandshake();

    void serverSendHandshake(
        ::std::vector<::std::byte>&& encryptedBaseValue
    );

    void serverReadHandshake(
        ::std::vector<::std::byte>&& baseValue
    );


    void clientHandshake();

    void clientReadHandshake();

    void clientResolveHandshake(
        ::std::vector<::std::byte>&& receivedValue
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



    // ------------------------------------------------------------------ async - udp setup

    void sendUdpAddr();

    void receiveUdpAddr();




private:

    ::std::shared_ptr<::network::Connection<UserMessageType>> m_connection;

    ::asio::ip::tcp::socket m_socket;
    ::network::Message<UserMessageType> m_bufferIn;
    ::detail::Queue<::network::Message<UserMessageType>> m_messagesOut;

    bool m_isSendAllowed{ false };

    ::std::mutex m_mutex;
    ::std::condition_variable m_blocker;

};



} // namespace network::tcp

#include <Network/Tcp/Connection.impl.hpp>
