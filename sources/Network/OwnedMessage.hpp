#pragma once

#include <Network/Message.hpp>



namespace network { template <::detail::constraint::isEnum UserMessageType> class Connection; }



namespace network {



template <
    ::detail::constraint::isEnum UserMessageType
> class OwnedMessage
    : public ::network::Message<UserMessageType>
{

public:

    // ------------------------------------------------------------------ *structors

    OwnedMessage(
        ::network::Message<UserMessageType> message,
        ::std::shared_ptr<::network::Connection<UserMessageType>> remote
    );

    ~OwnedMessage();



    // ------------------------------------------------------------------ informations

    auto getRemote()
        -> ::std::shared_ptr<::network::Connection<UserMessageType>>;



private:

    ::std::shared_ptr<::network::Connection<UserMessageType>> m_remote;

};



} // namespace network

#include <Network/OwnedMessage.impl.hpp>
