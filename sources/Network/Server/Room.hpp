#pragma once

#include <Network/TcpConnection.hpp>



namespace network::server {



template <
    ::detail::isEnum UserMessageType
> class Room
    : public ::network::ANode<UserMessageType>
{

public:

    // ------------------------------------------------------------------ *structors

    explicit Room();

    virtual ~Room() = 0;



    // ------------------------------------------------------------------ managment

    void add(
        ::std::shared_ptr<::network::tcp::Connection<UserMessageType>> connection
    );

    void remove(
        ::detail::Id id
    );

    auto extract(
        ::detail::Id id
    ) -> ::std::shared_ptr<::network::tcp::Connection<UserMessageType>> connection;



    // ------------------------------------------------------------------ out

    void send(
        const ::network::Message<UserMessageType>& message,
        ::detail::sameAs<::std::shared_ptr<::network::tcp::Connection<UserMessageType>>> auto... ignoredClients
    );




private:

    ::std::deque<::std::shared_ptr<::network::tcp::Connection<UserMessageType>>> m_connections;

};



} // namespace network::server

#include <Network/Server/Room.impl.hpp>
