#pragma once

#include <Network/Message.hpp>

namespace network { template <::detail::isEnum UserMessageType> class TcpConnection; }



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
        ::std::shared_ptr<::network::TcpConnection<UserMessageType>> remote
    );

    ~OwnedMessage();



    // ------------------------------------------------------------------ informations

    auto getRemote() const
        -> ::std::shared_ptr<::network::TcpConnection<UserMessageType>>;



private:

    ::std::shared_ptr<::network::TcpConnection<UserMessageType>> m_remote{ nullptr };

};



} // namespace network

#include <Network/OwnedMessage.impl.hpp>
