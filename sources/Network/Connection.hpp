#pragma once

#include <Detail/Queue.hpp>
#include <Security/Cipher.hpp>
#include <Detail/Id.hpp>
#include <Detail/Concepts.hpp>
#include <Network/Message.hpp>
#include <Network/OwnedMessage.hpp>

namespace network { template <::detail::IsEnum MessageType> class ANode; }



namespace network {



template <
    ::detail::IsEnum MessageType
> class Connection
    : public ::std::enable_shared_from_this<Connection<MessageType>>
{

public:

    // ------------------------------------------------------------------ *structors

    Connection(
        ::network::ANode<MessageType>& owner,
        ::boost::asio::ip::tcp::socket tcpSocket,
        ::boost::asio::ip::udp::socket udpSocket
    );

    ~Connection();




    // ------------------------------------------------------------------ async - connection

    // server
    auto connectToClient(
        ::detail::Id id
    ) -> bool;



    // client
    void connectToServer(
        const ::std::string& host,
        ::std::uint16_t port
    );

    void targetServerUdpPort(
        ::std::uint16_t port
    );



    // common
    void disconnect();

    auto isConnected() const
        -> bool;




    // ------------------------------------------------------------------ async - tcpIn

    void tcpReadHeader();

    void tcpReadBody();

    void tcpTransferBufferToInQueue();



    // ------------------------------------------------------------------ async - tcpOut

    void tcpSend(
        MessageType messageType,
        auto&&... args
    )
    {
        ::network::Message message{
            ::std::forward<decltype(messageType)>(messageType),
            ::network::TransmissionProtocol::tcp,
            ::std::forward<decltype(args)>(args)...
        };
        ::boost::asio::post(
            m_owner.getAsioContext(),
            [this, message]()
            {
                auto wasOutQueueEmpty{ m_tcpMessagesOut.empty() };
                m_tcpMessagesOut.push_back(::std::move(message));
                if (wasOutQueueEmpty) {
                    this->tcpWriteHeader();
                }
            }
        );
    }

    void tcpSend(
        ::network::Message<MessageType> message
    );

    void tcpWriteHeader();

    void tcpWriteBody();




    // ------------------------------------------------------------------ async - udpIn

    void udpReadHeader(
        ::std::size_t bytesAlreadyRead = 0
    );

    void udpReadBody(
        ::std::size_t bytesAlreadyRead = 0
    );

    void udpTransferBufferToInQueue();



    // ------------------------------------------------------------------ async - udpOut

    void udpSend(
        MessageType messageType,
        auto&&... args
    )
    {
        ::network::Message message{
            ::std::forward<decltype(messageType)>(messageType),
            ::network::TransmissionProtocol::udp,
            ::std::forward<decltype(args)>(args)...
        };
        ::boost::asio::post(
            m_owner.getAsioContext(),
            [this, message]()
            {
                auto wasOutQueueEmpty{ m_udpMessagesOut.empty() };
                m_udpMessagesOut.push_back(::std::move(message));
                if (wasOutQueueEmpty) {
                    this->udpWriteHeader();
                }
            }
        );
    }

    void udpSend(
        ::network::Message<MessageType> message
    );

    void udpWriteHeader(
        ::std::size_t bytesAlreadySent = 0
    );

    void udpWriteBody(
        ::std::size_t bytesAlreadySent = 0
    );



    // ------------------------------------------------------------------ other

    [[ nodiscard ]] auto getId() const
        -> ::detail::Id;



private:

    // ------------------------------------------------------------------ async - securityProtocol
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

    ::boost::asio::ip::tcp::socket m_tcpSocket;
    ::boost::asio::ip::udp::socket m_udpSocket;

    // in
    ::network::Message<MessageType> m_bufferIn;

    // out
    ::detail::Queue<::network::Message<MessageType>> m_tcpMessagesOut;
    ::detail::Queue<::network::Message<MessageType>> m_udpMessagesOut;

    // security
    ::security::Cipher m_cipher;

    ::detail::Id m_id{ 1 };

};



} // namespace network
