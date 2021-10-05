#pragma once

#include <Network/Connection.hpp>
#include <Network/MessageType.hpp>
#include <Detail/Queue.hpp>
#include <Network/Message.hpp>
#include <Network/OwnedMessage.hpp>
#include <Network/ANode.hpp>



namespace network {



template <
    ::detail::IsEnum MessageType
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

    [[ nodiscard ]] auto isConnected()
        -> bool;



    // ------------------------------------------------------------------ async - out

    void send(
        ::network::Message<MessageType>& message
    );

    void send(
        ::network::Message<MessageType>&& message
    );

    // construct and send
    void send(
        ::detail::IsEnum auto&& messageType,
        auto&&... args
    )
    {
        m_connection->send(
            ::std::forward<decltype(messageType)>(messageType),
            ::std::forward<decltype(args)>(args)...
        );
    }



private:

    // hardware connection to the server
    ::std::shared_ptr<::network::Connection<MessageType>> m_connection;

};



} // namespace network
