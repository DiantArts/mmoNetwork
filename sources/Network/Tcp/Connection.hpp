#pragma once

#include <Network/AConnection.hpp>



namespace network::tcp {



template <
    ::detail::isEnum UserMessageType
> class Connection
    : public ::network::AConnection<UserMessageType>
    , public ::std::enable_shared_from_this<Connection<UserMessageType>>
{

public:

    // ------------------------------------------------------------------ *structors

    Connection(
        ::network::ANode<UserMessageType>& owner,
        ::asio::ip::tcp::socket socket
    );

    // doesnt call onDisconnect
    ~Connection();



    // ------------------------------------------------------------------ async - connection

    auto connectToClient(
        ::detail::Id id
    ) -> bool;

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

    bool hasSendingMessagesAwaiting() const;

    void sendAwaitingMessages();



    // ------------------------------------------------------------------ async - in

    void startReadMessage();

    void pullIncommingMessage();

    void pullIncommingMessages();

    void blockingPullIncommingMessages();



    // ------------------------------------------------------------------ other

    // TODO: create struct sharable info
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


    void sendPublicKey();

    void readPublicKey();



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



private:

    ::asio::ip::tcp::socket m_socket;

    ::detail::Id m_id{ 1 };
    ::std::string m_userName;

    bool m_isSendAllowed{ false };

    using ::network::AConnection<UserMessageType>::m_owner;
    using ::network::AConnection<UserMessageType>::m_bufferIn;
    using ::network::AConnection<UserMessageType>::m_messagesOut;
#ifdef ENABLE_ENCRYPTION
    using ::network::AConnection<UserMessageType>::m_cipher;
#endif // ENABLE_ENCRYPTION

    ::std::mutex m_mutex;
    ::std::condition_variable m_blocker;

};



} // namespace network::tcp

#include <Network/Tcp/Connection.impl.hpp>
