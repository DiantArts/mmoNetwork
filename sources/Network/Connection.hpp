#pragma once

#include <Network/Tcp/Connection.hpp>
#include <Network/Udp/Connection.hpp>
#include <Network/Informations.hpp>

namespace network::client { template <::detail::isEnum UserMessageType> class AClient; }
namespace network::server { template <::detail::isEnum UserMessageType> class AServer; }




namespace network {



template <
    ::detail::isEnum UserMessageType
> class Connection
    : public ::std::enable_shared_from_this<Connection<UserMessageType>>
{

public:

    // ------------------------------------------------------------------ *structors

    ~Connection()
    {
        ::std::cout << "[" << this->informations.id << "] Connetion destroyed\n";
    }



    // ------------------------------------------------------------------ shared_ptr

    static auto create(
        ::network::client::AClient<UserMessageType>& owner,
        const ::std::string& host,
        const ::std::uint16_t port
    ) -> ::std::shared_ptr<Connection<UserMessageType>>
    {
        auto ptr{ ::std::shared_ptr<Connection<UserMessageType>>{
            new Connection<UserMessageType>(owner, host, port)
        } };
        ptr->tcp.assignConnection(ptr->getPtr());
        ptr->udp.assignConnection(ptr->getPtr());

        // TODO: setup tcp
        ptr->tcp.startConnectingToServer(host, port);
        ::std::cout << "[Connection:TCP:" << ptr->informations.id << "] "
            << "Connection request sent to " << host << ":" << port << ".\n";
        ptr->tcp.waitNotification();
        // TODO: setup udp

        return ptr;
    }

    static auto create(
        ::network::server::AServer<UserMessageType>& owner,
        ::asio::ip::tcp::socket&& socket,
        ::detail::Id id
    ) -> ::std::shared_ptr<Connection<UserMessageType>>
    {
        auto ptr{ ::std::shared_ptr<Connection<UserMessageType>>{
            new Connection<UserMessageType>(owner, ::std::move(socket), id)
        } };
        ptr->tcp.assignConnection(ptr->getPtr());
        ptr->udp.assignConnection(ptr->getPtr());

        // TODO: add a ownership to avoid instant dustruction
        if (!ptr->tcp.startConnectingToClient()) {
            throw ::std::runtime_error("[ERROR:Server] Connection failed.");
        }

        return ptr;
    }

    auto getPtr()
        -> ::std::shared_ptr<Connection<UserMessageType>>
    {
        return this->shared_from_this();
    }



    // ------------------------------------------------------------------ others

    void disconnect()
    {
        this->udp.close();
        this->tcp.disconnect();
    }



    // ------------------------------------------------------------------ others

    void setUserName(
        ::std::string str
    )
    {
        this->informations.userName = ::std::move(str);
    }




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
    )
        : m_owner{ owner }
        , informations{ 0, "" }
        , udp{ m_owner.getAsioContext() }
        , tcp{ ::asio::ip::tcp::socket(owner.getAsioContext()) }
    {}

    // called by server
    Connection(
        ::network::server::AServer<UserMessageType>& owner,
        ::asio::ip::tcp::socket&& socket,
        ::detail::Id id
    )
        : m_owner{ owner }
        , informations{ id, "" }
        , udp{ m_owner.getAsioContext() }
        , tcp{ ::std::move(socket) }
    {}

};



} // namespace network
