#pragma once

#include <Network/Tcp/Connection.hpp>
#include <Network/Udp/Connection.hpp>
#include <Network/Informations.hpp>

namespace network::client { template <::detail::constraint::isEnum UserMessageType> class AClient; }
namespace network::server { template <::detail::constraint::isEnum UserMessageType> class AServer; }




namespace network {



template <
    ::detail::constraint::isEnum UserMessageType
> class Connection
    : public ::std::enable_shared_from_this<Connection<UserMessageType>>
{

public:

    // ------------------------------------------------------------------ *structors

    ~Connection();



    // ------------------------------------------------------------------ shared_ptr

    static auto create(
        ::network::client::AClient<UserMessageType>& owner,
        const ::std::string& host,
        const ::std::uint16_t port
    ) -> ::std::shared_ptr<Connection<UserMessageType>>;

    static auto create(
        ::network::server::AServer<UserMessageType>& owner,
        ::asio::ip::tcp::socket&& socket,
        ::detail::Id id
    ) -> ::std::shared_ptr<Connection<UserMessageType>>;

    auto getPtr()
        -> ::std::shared_ptr<Connection<UserMessageType>>;

    void disconnect();




private:

    ::network::ANode<UserMessageType>& m_owner;

#ifdef ENABLE_ENCRYPTION
    ::security::Cipher m_cipher;
#endif // ENABLE_ENCRYPTION



public:

    ::network::Informations informations;

    ::network::udp::Connection<UserMessageType> udp;
    ::network::tcp::Connection<UserMessageType> tcp;



private:

    // ------------------------------------------------------------------ private constructors

    friend class ::network::tcp::Connection<UserMessageType>;
    friend class ::network::udp::Connection<UserMessageType>;

    // called by client
    Connection(
        ::network::client::AClient<UserMessageType>& owner,
        const ::std::string& host,
        const ::std::uint16_t port
    );

    // called by server
    Connection(
        ::network::server::AServer<UserMessageType>& owner,
        ::asio::ip::tcp::socket&& socket,
        ::detail::Id id
    );

};



} // namespace network

#include <Network/Connection.impl.hpp>
