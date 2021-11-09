#pragma once

#include <Network/Message.hpp>



namespace network { template <::detail::isEnum UserMessageType> class AConnection; }
namespace network::tcp { template <::detail::isEnum UserMessageType> class Connection; }
namespace network::udp { template <::detail::isEnum UserMessageType> class Connection; }



namespace network {



template <
    ::detail::isEnum UserMessageType
> class OwnedMessage
    : public ::network::Message<UserMessageType>
{

public:

    // ------------------------------------------------------------------ *structors

    OwnedMessage(
        ::network::Message<UserMessageType> message,
        ::std::shared_ptr<::network::AConnection<UserMessageType>> remote
    );

    ~OwnedMessage();



    // ------------------------------------------------------------------ informations

    auto getRemoteAsTcp()
        -> ::std::shared_ptr<::network::tcp::Connection<UserMessageType>>;

    auto getRemoteAsUdp()
        -> ::std::shared_ptr<::network::udp::Connection<UserMessageType>>;



private:

    ::std::shared_ptr<::network::AConnection<UserMessageType>> m_remote;

};



} // namespace network

#include <Network/OwnedMessage.impl.hpp>
