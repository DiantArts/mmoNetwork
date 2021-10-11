#pragma once

#include <Network/Message.hpp>

namespace network { template <::detail::isEnum MessageType> class Connection; }



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
        ::std::shared_ptr<::network::Connection<MessageType>> remote
    );

    ~OwnedMessage();



    // ------------------------------------------------------------------ informations

    auto getRemote() const
        -> ::std::shared_ptr<::network::Connection<MessageType>>;



private:

    ::std::shared_ptr<::network::Connection<MessageType>> m_remote{ nullptr };

};



} // namespace network
