#pragma once

#include <Message.hpp>

// forward decleration
namespace network { template <::network::detail::IsEnum MessageType> class Connection; }



namespace network {



template <
    ::network::detail::IsEnum MessageType
> class OwnedMessage
    : public ::network::Message<MessageType>
{

public:

    // ------------------------------------------------------------------ *structors

    OwnedMessage(
        ::std::shared_ptr<::network::Connection<MessageType>> remote,
        ::network::Message<MessageType> message
    );

    ~OwnedMessage();



    // ------------------------------------------------------------------ informations

    auto getRemote() const
        -> ::std::shared_ptr<::network::Connection<MessageType>>;



private:

    ::std::shared_ptr<::network::Connection<MessageType>> m_remote{ nullptr };

};



} // namespace network
