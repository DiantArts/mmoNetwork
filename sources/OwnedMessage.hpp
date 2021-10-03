#pragma once

#include <Message.hpp>

// forward decleration
namespace network { template <::network::detail::IsEnum MessageEnumType> class Connection; }



namespace network {



template <
    ::network::detail::IsEnum MessageEnumType
> class OwnedMessage
    : public ::network::Message<MessageEnumType>
{

public:

    // ------------------------------------------------------------------ *structors

    inline OwnedMessage(
        ::std::shared_ptr<::network::Connection<MessageEnumType>> remote,
        ::network::Message<MessageEnumType> message
    )
        : ::network::Message<MessageEnumType>{ ::std::move(message) }
        , m_remote{ remote }
    {}

    inline ~OwnedMessage() = default;



    // ------------------------------------------------------------------ informations

    auto getRemote() const
        -> ::std::shared_ptr<::network::Connection<MessageEnumType>>
    {
        return m_remote;
    }



private:

    ::std::shared_ptr<::network::Connection<MessageEnumType>> m_remote{ nullptr };

};



} // namespace network
