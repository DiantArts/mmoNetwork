#pragma once

// ------------------------------------------------------------------ *structors

template <
    ::detail::constraint::isEnum UserMessageType
> network::Connection<UserMessageType>::~Connection()
{
    ::std::cout << "[" << this->informations.getId() << "] Connetion destroyed\n";
}



// ------------------------------------------------------------------ shared_ptr

template <
    ::detail::constraint::isEnum UserMessageType
> auto network::Connection<UserMessageType>::create(
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
    ::std::cout << "[Connection:TCP:" << ptr->informations.getId() << "] "
        << "Connection request sent to " << host << ":" << port << ".\n";
    ptr->tcp.waitNotification();
    // TODO: setup udp

    return ptr;
}

template <
    ::detail::constraint::isEnum UserMessageType
> auto network::Connection<UserMessageType>::create(
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

template <
    ::detail::constraint::isEnum UserMessageType
> auto network::Connection<UserMessageType>::getPtr()
    -> ::std::shared_ptr<Connection<UserMessageType>>
{
    return this->shared_from_this();
}

template <
    ::detail::constraint::isEnum UserMessageType
> void network::Connection<UserMessageType>::disconnect()
{
    this->udp.close();
    this->tcp.disconnect();
}



// ------------------------------------------------------------------ private constructors

// called by client
template <
    ::detail::constraint::isEnum UserMessageType
> network::Connection<UserMessageType>::Connection(
    ::network::client::AClient<UserMessageType>& owner,
    const ::std::string& host,
    const ::std::uint16_t port
)
    : m_owner{ owner }
    , udp{ m_owner.getAsioContext() }
    , tcp{ ::asio::ip::tcp::socket(owner.getAsioContext()) }
{}

// called by server
template <
    ::detail::constraint::isEnum UserMessageType
> network::Connection<UserMessageType>::Connection(
    ::network::server::AServer<UserMessageType>& owner,
    ::asio::ip::tcp::socket&& socket,
    ::detail::Id id
)
    : m_owner{ owner }
    , informations{ id }
    , udp{ m_owner.getAsioContext() }
    , tcp{ ::std::move(socket) }
{}
