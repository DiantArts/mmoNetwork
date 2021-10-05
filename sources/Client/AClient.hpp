#pragma once

#include <Connection.hpp>
#include <MessageType.hpp>
#include <Queue.hpp>
#include <Message.hpp>
#include <OwnedMessage.hpp>
#include <ANode.hpp>



namespace network {



template <
    ::network::detail::IsEnum MessageType
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
        ::network::detail::IsEnum auto&& messageType,
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
