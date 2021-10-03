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

    inline OwnedMessage(
        ::std::shared_ptr<::network::Connection<MessageType>> remote,
        ::network::Message<MessageType> message
    )
        : ::network::Message<MessageType>{ ::std::move(message) }
        , m_remote{ remote }
    {}

    inline ~OwnedMessage() = default;



    // ------------------------------------------------------------------ informations

    auto getRemote() const
        -> ::std::shared_ptr<::network::Connection<MessageType>>
    {
        return m_remote;
    }



private:

    ::std::shared_ptr<::network::Connection<MessageType>> m_remote{ nullptr };

};



} // namespace network
