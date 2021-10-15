#pragma once

#include <Detail/Queue.hpp>
#include <Security/Cipher.hpp>
#include <Detail/Id.hpp>
#include <Detail/Concepts.hpp>
#include <Network/Message.hpp>
#include <Network/OwnedMessage.hpp>

namespace network { template <::detail::isEnum MessageType> class ANode; }



namespace network {



template <
    ::detail::isEnum MessageType
> class TcpConnection
    : public ::std::enable_shared_from_this<TcpConnection<MessageType>>
{

public:

    // ------------------------------------------------------------------ *structors

    TcpConnection(
        ::network::ANode<MessageType>& owner,
        ::boost::asio::ip::tcp::socket socket
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
                auto wasOutQueueEmpty{ m_messagesOut.empty() };
                m_messagesOut.push_back(::std::move(message));
                if (m_isValid && wasOutQueueEmpty) {
                    this->writeAwaitingMessages();
                }
            }
        );
    }

    void send(
        ::network::Message<MessageType> message
    );



    // ------------------------------------------------------------------ other

    [[ nodiscard ]] auto getId() const
        -> ::detail::Id;

    [[ nodiscard ]] auto getOwner() const
        -> const ::network::ANode<MessageType>&;

    [[ nodiscard ]] auto getPort() const
        -> ::std::uint16_t;

    [[ nodiscard ]] auto getAddress() const
        -> ::std::string;



private:

    // ------------------------------------------------------------------ async - out

    template <
        auto successCallback,
        auto failureHeaderCallback,
        auto failureBodyCallback
    > void sendMessage(
        ::network::Message<MessageType> message
    )
    {
        ::boost::asio::async_write(
            m_socket,
            ::boost::asio::buffer(message.getHeaderAddr(), message.getHeaderSize()),
            ::std::bind_front(
                [this](
                    ::network::Message<MessageType> message,
                    const boost::system::error_code& errorCode,
                    const ::std::size_t length
                ) {
                    if (errorCode) {
                        if (errorCode == ::boost::asio::error::operation_aborted) {
                            ::std::cerr << "[Connection:TCP] Operation canceled\n";
                        } else {
                            failureHeaderCallback(::std::ref(*this), errorCode);
                        }
                    } else {
                        if (!message.isBodyEmpty()) {
                            ::boost::asio::async_write(
                                m_socket,
                                ::boost::asio::buffer(message.getBodyAddr(), message.getBodySize()),
                                ::std::bind_front(
                                    [this](
                                        ::network::Message<MessageType> message,
                                        const boost::system::error_code& errorCode,
                                        const ::std::size_t length
                                    ) {
                                        if (errorCode) {
                                            if (errorCode == ::boost::asio::error::operation_aborted) {
                                                ::std::cerr << "[Connection:TCP] Operation canceled\n";
                                            } else {
                                                failureBodyCallback(::std::ref(*this), errorCode);
                                            }
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
    > void sendRawData(
        auto&&... args
    )
    {
        auto pointerToData{ new Type{ ::std::forward<decltype(args)>(args)... } };
        ::boost::asio::async_write(
            m_socket,
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
    > void sendRawData(
        ::detail::isPointer auto pointerToData,
        ::std::size_t dataSize
    )
    {
        ::boost::asio::async_write(
            m_socket,
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

    void writeAwaitingMessages();



    // ------------------------------------------------------------------ async - in

    template <
        auto successCallback,
        auto failureHeaderCallback,
        auto failureBodyCallback
    > void receiveMessage()
    {
        ::boost::asio::async_read(
            m_socket,
            ::boost::asio::buffer(m_bufferIn.getHeaderAddr(), m_bufferIn.getHeaderSize()),
            [this](
                const boost::system::error_code& errorCode,
                const ::std::size_t length
            ) {
                if (errorCode) {
                    if (errorCode == ::boost::asio::error::operation_aborted) {
                        ::std::cerr << "[Connection:TCP] Operation canceled\n";
                    } else {
                        failureHeaderCallback(::std::ref(*this), errorCode);
                    }
                } else {
                    if (!m_bufferIn.isBodyEmpty()) {
                        m_bufferIn.updateBodySize();
                        ::boost::asio::async_read(
                            m_socket,
                            ::boost::asio::buffer(m_bufferIn.getBodyAddr(), m_bufferIn.getBodySize()),
                            [this](
                                const boost::system::error_code& errorCode,
                                const ::std::size_t length
                            ) {
                                if (errorCode) {
                                    if (errorCode == ::boost::asio::error::operation_aborted) {
                                        ::std::cerr << "[Connection:TCP] Operation canceled\n";
                                    } else {
                                        failureBodyCallback(::std::ref(*this), errorCode);
                                    }
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
    > void receiveRawData()
    {
        auto pointerToData{ new ::std::array<::std::byte, sizeof(Type)> };
        ::boost::asio::async_read(
            m_socket,
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
    > void receiveToRawData(
        ::detail::isPointer auto pointerToData,
        ::std::size_t dataSize
    )
    {
        ::boost::asio::async_read(
            m_socket,
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

    void readBody();

    void transferBufferToInQueue();



private:

    // ------------------------------------------------------------------ async - securityProtocol
    // Identification (Client claiming to identify as a client of the protocol):
    //     1. Both send the public key
    //     2. The server sends an handshake encrypted
    //     3. The client resolves and sends the handshake back encrypted

    void identificate();


    void sendPublicKey();

    void readPublicKey();



#if ENABLE_ENCRYPTION

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



    void sendIdentificationAcceptance();

    void clientWaitIdentificationAcceptance();



    // ------------------------------------------------------------------ async - securityProtocol
    // TODO: Authentification
    // Authentification (Client registering with some provable way that they are who the claim to be):
    //     1. Username
    //     2. password




private:

    ::network::ANode<MessageType>& m_owner;

    // tcp
    ::boost::asio::ip::tcp::socket m_socket;
    ::network::Message<MessageType> m_bufferIn;
    ::detail::Queue<::network::Message<MessageType>> m_messagesOut;

    // security module
#if ENABLE_ENCRYPTION
    ::security::Cipher m_cipher;
#endif // ENABLE_ENCRYPTION

    ::detail::Id m_id{ 1 };

    bool m_isValid{ false };

};



} // namespace network
