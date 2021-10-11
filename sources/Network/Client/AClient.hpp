#pragma once

#include <Network/Connection.hpp>
#include <Network/MessageType.hpp>
#include <Detail/Queue.hpp>
#include <Network/Message.hpp>
#include <Network/OwnedMessage.hpp>
#include <Network/ANode.hpp>



namespace network {



template <
    ::detail::isEnum MessageType
> class AClient
    : public ::network::ANode<MessageType>
{

public:

    // ------------------------------------------------------------------ *structors

    explicit AClient();

    virtual ~AClient() = 0;



    // ------------------------------------------------------------------ connection

    [[ nodiscard ]] auto connect(
        const ::std::string& host,
        ::std::uint16_t port
    ) -> bool;

    void disconnect();

    void stop();

    [[ nodiscard ]] auto isConnected()
        -> bool;



    // ------------------------------------------------------------------ async - tcpOut

    void tcpSend(
        ::network::Message<MessageType>& message
    );

    void tcpSend(
        ::network::Message<MessageType>&& message
    );

    // construct and tcpSend
    void tcpSend(
        MessageType messageType,
        auto&&... args
    )
    {
        m_connection->tcpSend(
            ::std::forward<decltype(messageType)>(messageType),
            ::std::forward<decltype(args)>(args)...
        );
    }



    // ------------------------------------------------------------------ async - udpOut

    void udpSend(
        ::network::Message<MessageType>& message
    );

    void udpSend(
        ::network::Message<MessageType>&& message
    );

    // construct and udpSend
    void udpSend(
        MessageType messageType,
        auto&&... args
    )
    {
        m_connection->udpSend(
            ::std::forward<decltype(messageType)>(messageType),
            ::std::forward<decltype(args)>(args)...
        );
    }



    // ------------------------------------------------------------------ receive behaviour

    virtual auto defaultReceiveBehaviour(
        ::network::Message<MessageType>& message,
        ::std::shared_ptr<::network::Connection<MessageType>> connection
    ) -> bool
        override final;


private:

    // hardware connection to the server
    ::std::shared_ptr<::network::Connection<MessageType>> m_connection;

};



} // namespace network
