#pragma once

#include <Network/Message.hpp>

namespace network { template <::detail::isEnum MessageType> class TcpConnection; }



namespace network {



template <
    ::detail::isEnum MessageType
> class OwnedMessage
    : public ::network::Message<MessageType>
{

public:

    // ------------------------------------------------------------------ *structors

    OwnedMessage(
        ::network::Message<MessageType> message,
        ::std::shared_ptr<::network::TcpConnection<MessageType>> remote
    );

    ~OwnedMessage();



    // ------------------------------------------------------------------ informations

    auto getRemote() const
        -> ::std::shared_ptr<::network::TcpConnection<MessageType>>;



private:

    ::std::shared_ptr<::network::TcpConnection<MessageType>> m_remote{ nullptr };

};



} // namespace network
