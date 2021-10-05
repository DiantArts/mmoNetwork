#pragma once

#include <Queue.hpp>
#include <Cipher.hpp>
#include <Id.hpp>
#include <Detail/Concepts.hpp>
#include <Message.hpp>
#include <OwnedMessage.hpp>

namespace network{ template <::network::detail::IsEnum MessageType> class ANode; }



namespace network {



template <
    ::network::detail::IsEnum MessageType
> class Connection
    : public ::std::enable_shared_from_this<Connection<MessageType>>
{

public:

    // ------------------------------------------------------------------ *structors

    Connection(
        ::boost::asio::ip::tcp::socket socket,
        ::network::ANode<MessageType>& connectionOwner
    );

    ~Connection();




    // ------------------------------------------------------------------ async - connection

    auto connectToClient(
        ::network::Id id
    ) -> bool;

    void connectToServer(
        const ::std::string& host,
        const ::std::uint16_t port
    );

    void disconnect();

    auto isConnected() const
        -> bool;




    // ------------------------------------------------------------------ async - in

    void readHeader();

    void readBody();

    void transferBufferToInQueue();



    // ------------------------------------------------------------------ async - out

    void send(
        ::network::detail::IsEnum auto&& messageType,
        auto&&... args
    )
    {
        ::network::Message message{
            ::std::forward<decltype(messageType)>(messageType),
            ::std::forward<decltype(args)>(args)...
        };
        ::boost::asio::post(
            m_owner.getAsioContext(),
            [this, message]()
            {
                auto wasOutQueueEmpty{ m_messagesOut.empty() };
                m_messagesOut.push_back(::std::move(message));
                if (wasOutQueueEmpty) {
                    this->writeHeader();
                }
            }
        );
    }

    void send(
        ::network::Message<MessageType> message
    );

    // TODO: client memory error
    void writeHeader();

    void writeBody();



    // ------------------------------------------------------------------ other

    [[ nodiscard ]] auto getId() const
        -> ::network::Id;



private:

    // ------------------------------------------------------------------ async - securityProtocol
    // TODO: encrypt handshake
    // Identification (Client claiming to identify as a client of the protocol):
    //     1. Both send the public key
    //     2. The server sends an handshake encrypted
    //     3. The client resolves and sends the handshake back encrypted

    void identificate();


    void sendPublicKey();

    void readPublicKey();



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



    // ------------------------------------------------------------------ async - securityProtocol
    // TODO: Authentification
    // Authentification (Client registering with some provable way that they are who the claim to be):
    //     1. Username
    //     2. password




private:

    ::network::ANode<MessageType>& m_owner;

    // context shared by the whole asio instance
    ::boost::asio::ip::tcp::socket m_socket;

    // in
    ::network::Message<MessageType> m_bufferIn;

    // out
    ::network::Queue<::network::Message<MessageType>> m_messagesOut;

    // security
    ::security::Cipher m_cipher;

    ::network::Id m_id{ 1 };

};



} // namespace network
