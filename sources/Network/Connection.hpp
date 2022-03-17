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



    // ------------------------------------------------------------------ informations

    auto getSharableInformations() const
        -> const ::network::SharableInformations&;

    void setSharableInformations(
        ::network::SharableInformations newInformations
    );



    template <
        ::network::SharableInformations::Index informationIndex
    > void setSharableInformation(
        auto&&... args
    );



    auto getId() const
        -> ::detail::Id;

    void setId(
        ::detail::Id newId
    );



    auto getName() const
        -> const ::std::string&;

    void setName(
        ::std::string newName
    );



private:

    ::network::ANode<UserMessageType>& m_owner;

#ifdef ENABLE_ENCRYPTION
    ::security::Cipher m_cipher;
#endif // ENABLE_ENCRYPTION

    ::detail::Id m_id{ 0 };

    ::network::SharableInformations m_sharableInformations;



public:

    // Subclass composition
    ::network::udp::Connection<UserMessageType> udp;
    ::network::tcp::Connection<UserMessageType> tcp;



private:

    // ------------------------------------------------------------------ private constructors

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

    friend class ::network::tcp::Connection<UserMessageType>;
    friend class ::network::udp::Connection<UserMessageType>;

};



} // namespace network

#include <Network/Connection.impl.hpp>
