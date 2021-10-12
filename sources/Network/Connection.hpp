#pragma once

#include <Detail/Queue.hpp>
#include <Security/Cipher.hpp>
#include <Detail/Id.hpp>
#include <Detail/Concepts.hpp>
#include <Network/Message.hpp>
#include <Network/OwnedMessage.hpp>
// #include <Network/Identifier.hpp>

namespace network { template <::detail::isEnum MessageType> class ANode; }



namespace network {



template <
    ::detail::isEnum MessageType
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

    auto connectToClient(
        ::detail::Id id
    ) -> bool;

    void connectToServer(
        const ::std::string& host,
        ::std::uint16_t port
    );

    void targetServerUdpPort(
        ::std::uint16_t port
    );

    void disconnect();

    auto isConnected() const
        -> bool;



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
                if (m_isValid && wasOutQueueEmpty) {
                    this->tcpWriteAwaitingMessages();
                }
            }
        );
    }

    void tcpSend(
        ::network::Message<MessageType> message
    );



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
                if (m_isValid && wasOutQueueEmpty) {
                    this->udpWriteHeader();
                }
            }
        );
    }

    void udpSend(
        ::network::Message<MessageType> message
    );



    // ------------------------------------------------------------------ other

    [[ nodiscard ]] auto getId() const
        -> ::detail::Id;

    [[ nodiscard ]] auto getOwner() const
        -> const ::network::ANode<MessageType>&;



private:

    // ------------------------------------------------------------------ async - tcpOut

    template <
        auto successCallback,
        auto failureHeaderCallback,
        auto failureBodyCallback
    > void tcpSendMessage(
        ::network::Message<MessageType> message
    )
    {
        ::boost::asio::async_write(
            m_tcpSocket,
            ::boost::asio::buffer(message.getHeaderAddr(), message.getHeaderSize()),
            ::std::bind_front(
                [this](
                    ::network::Message<MessageType> message,
                    const boost::system::error_code& errorCode,
                    const ::std::size_t length
                ) {
                    if (errorCode) {
                        failureHeaderCallback(::std::ref(*this), errorCode);
                    } else {
                        if (!message.isBodyEmpty()) {
                            ::boost::asio::async_write(
                                m_tcpSocket,
                                ::boost::asio::buffer(message.getBodyAddr(), message.getBodySize()),
                                ::std::bind_front(
                                    [this](
                                        ::network::Message<MessageType> message,
                                        const boost::system::error_code& errorCode,
                                        const ::std::size_t length
                                    ) {
                                        if (errorCode) {
                                            failureBodyCallback(::std::ref(*this), errorCode);
                                        } else {
                                            successCallback(::std::ref(*this));
                                        }
                                    },
                                    ::std::move(message)
                                )
                            );
                        } else {
                            successCallback(::std::ref(*this));
                        }
                    }
                },
                ::std::move(message)
            )
        );
    }

    template <
        typename Type,
        auto successCallback,
        auto failureCallback
    > void tcpSendRawData(
        auto&&... args
    )
    {
        auto pointerToData{ new Type{ ::std::forward<decltype(args)>(args)... } };
        ::boost::asio::async_write(
            m_tcpSocket,
            ::boost::asio::buffer(pointerToData, sizeof(Type)),
            [this, pointerToData](
                const boost::system::error_code& errorCode,
                const ::std::size_t length
            ) {
                if (errorCode) {
                    failureCallback(::std::ref(*this), errorCode);
                } else {
                    successCallback(::std::ref(*this));
                }
                delete pointerToData;
            }
        );
    }

    template <
        auto successCallback,
        auto failureCallback
    > void tcpSendRawData(
        ::detail::isPointer auto pointerToData,
        ::std::size_t dataSize
    )
    {
        ::boost::asio::async_write(
            m_tcpSocket,
            ::boost::asio::buffer(pointerToData, dataSize),
            [this](
                const boost::system::error_code& errorCode,
                const ::std::size_t length
            ) {
                if (errorCode) {
                    failureCallback(::std::ref(*this), errorCode);
                } else {
                    successCallback(::std::ref(*this));
                }
            }
        );
    }

    void tcpWriteAwaitingMessages();



    // ------------------------------------------------------------------ async - udpOut

    void udpWriteHeader(
        ::std::size_t bytesAlreadySent = 0
    );

    void udpWriteBody(
        ::std::size_t bytesAlreadySent = 0
    );



    // ------------------------------------------------------------------ async - tcpIn

    template <
        auto successCallback,
        auto failureHeaderCallback,
        auto failureBodyCallback
    > void tcpReceiveMessage()
    {
        ::boost::asio::async_read(
            m_tcpSocket,
            ::boost::asio::buffer(m_tcpBufferIn.getHeaderAddr(), m_tcpBufferIn.getHeaderSize()),
            [this](
                const boost::system::error_code& errorCode,
                const ::std::size_t length
            ) {
                if (errorCode) {
                    failureHeaderCallback(::std::ref(*this), errorCode);
                } else {
                    if (!m_tcpBufferIn.isBodyEmpty()) {
                        m_tcpBufferIn.updateBodySize();
                        ::boost::asio::async_read(
                            m_tcpSocket,
                            ::boost::asio::buffer(m_tcpBufferIn.getBodyAddr(), m_tcpBufferIn.getBodySize()),
                            [this](
                                const boost::system::error_code& errorCode,
                                const ::std::size_t length
                            ) {
                                if (errorCode) {
                                    failureBodyCallback(::std::ref(*this), errorCode);
                                } else {
                                    successCallback(::std::ref(*this));
                                }
                            }
                        );
                    } else {
                        successCallback(::std::ref(*this));
                    }
                }
            }
        );
    }

    template <
        typename Type,
        auto successCallback,
        auto failureCallback
    > void tcpReceiveRawData()
    {
        auto pointerToData{ new ::std::array<::std::byte, sizeof(Type)> };
        ::boost::asio::async_read(
            m_tcpSocket,
            ::boost::asio::buffer(pointerToData->data(), sizeof(Type)),
            [this, pointerToData](
                const boost::system::error_code& errorCode,
                const ::std::size_t length
            ) {
                if (errorCode) {
                    failureCallback(::std::ref(*this), errorCode);
                } else {
                    successCallback(::std::ref(*this), *static_cast<Type*>(pointerToData->data()));
                }
                delete pointerToData;
            }
        );
    }

    template <
        auto successCallback,
        auto failureCallback
    > void tcpReceiveToRawData(
        ::detail::isPointer auto pointerToData,
        ::std::size_t dataSize
    )
    {
        ::boost::asio::async_read(
            m_tcpSocket,
            ::boost::asio::buffer(pointerToData, dataSize),
            [this, pointerToData](
                const boost::system::error_code& errorCode,
                const ::std::size_t length
            ) {
                if (errorCode) {
                    failureCallback(::std::ref(*this), errorCode);
                } else {
                    successCallback(::std::ref(*this));
                }
            }
        );
    }

    void startReadMessage();

    void tcpReadBody();

    void tcpTransferBufferToInQueue();




    // ------------------------------------------------------------------ async - udpIn

    void udpReadHeader(
        ::std::size_t bytesAlreadyRead = 0
    );

    void udpReadBody(
        ::std::size_t bytesAlreadyRead = 0
    );

    void udpTransferBufferToInQueue();



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

    void sendIdentificationAcceptanceHeader();

    void sendIdentificationAcceptanceBody(
        ::network::Message<MessageType>* message
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

    void clientWaitIdentificationAcceptanceHeader();

    void clientWaitIdentificationAcceptanceBody();



    // ------------------------------------------------------------------ async - securityProtocol
    // TODO: Authentification
    // Authentification (Client registering with some provable way that they are who the claim to be):
    //     1. Username
    //     2. password




private:

    ::network::ANode<MessageType>& m_owner;

    // tcp
    ::boost::asio::ip::tcp::socket m_tcpSocket;
    ::network::Message<MessageType> m_tcpBufferIn;
    ::detail::Queue<::network::Message<MessageType>> m_tcpMessagesOut;

    // udp
    ::boost::asio::ip::udp::socket m_udpSocket;
    ::network::Message<MessageType> m_udpBufferIn;
    ::detail::Queue<::network::Message<MessageType>> m_udpMessagesOut;

    // security
    ::security::Cipher m_cipher;

    ::detail::Id m_id{ 1 };

    bool m_isValid{ false };

};



} // namespace network
