#pragma once

#include <Detail/Concepts.hpp>
#include <Detail/Queue.hpp>
#include <Network/Message.hpp>
#include <Network/OwnedMessage.hpp>

#ifdef ENABLE_ENCRYPTION
#include <Security/Cipher.hpp>
#endif

namespace network { template <::detail::constraint::isEnum UserMessageType> class ANode; }
namespace network { template <::detail::constraint::isEnum UserMessageType> class Connection; }



namespace network::udp {



template <
    ::detail::constraint::isEnum UserMessageType
> class Connection {

public:

    ///////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // *structors
    //
    ///////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////
    /// \brief Construct a new udp connection.
    ///
    /// This contructor creates basic udp connection.
    ///
    /// \warning The class is imcomlete with just the contructor called.
    ///          In order to work, the user must call assignConnection().
    ///
    /// Once the class is fully initialized, the user must call target()
    /// either call:
    /// \arg \c startConnectingToClient()
    /// \arg \c startConnectingToServer()
    ///
    /// depending on what they plan to connect to.
    /// The call must match the socket sent to the constructor.
    ///
    /// \param socket The udp socket from asio, generated beforehand.
    ///
    ////////////////////////////////////////////////////////////
    Connection(
        ::asio::io_context& asioContext
    );

    // doesnt call onDisconnect
    ~Connection();



    // ------------------------------------------------------------------ async - connection

    void target(
        const ::std::string& host,
        const ::std::uint16_t port
    );

    void close();

    [[ nodiscard ]] auto isOpen() const
        -> bool;



    // ------------------------------------------------------------------ async - out

    void send(
        ::network::Message<UserMessageType>::SystemType messageType,
        auto&&... args
    );

    void send(
        UserMessageType messageType,
        auto&&... args
    );

    void send(
        const ::network::Message<UserMessageType>& message
    );

    void send(
        ::network::Message<UserMessageType>&& message
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
    > void sendMessageHeader(
        ::network::Message<UserMessageType> message,
        ::std::size_t bytesAlreadySent,
        auto&&... args
    );

    template <
        auto successCallback
    > void sendMessageBody(
        ::network::Message<UserMessageType> message,
        ::std::size_t bytesAlreadySent,
        auto&&... args
    );


    template <
        auto successCallback
    > void sendQueueMessage(
        auto&&... args
    );

    template <
        auto successCallback
    > void sendQueueMessageHeader(
        ::std::size_t bytesAlreadySent,
        auto&&... args
    );

    template <
        auto successCallback
    > void sendQueueMessageBody(
        ::std::size_t bytesAlreadySent,
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
    > void receiveMessageHeader(
        ::std::size_t bytesAlreadySent,
        auto&&... args
    );

    template <
        auto successCallback
    > void receiveMessageBody(
        ::std::size_t bytesAlreadySent,
        auto&&... args
    );

    void transferBufferToInQueue();



private:

    ::std::shared_ptr<::network::Connection<UserMessageType>> m_connection;

    ::asio::ip::udp::socket m_socket;
    ::network::Message<UserMessageType> m_bufferIn;
    ::detail::Queue<::network::Message<UserMessageType>> m_messagesOut;

    bool m_isSendAllowed{ false };

};



} // namespace network::udp

#include <Network/Udp/Connection.impl.hpp>
