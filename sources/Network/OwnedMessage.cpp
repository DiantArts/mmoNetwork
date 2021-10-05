#include <pch.hpp>
#include <Network/OwnedMessage.hpp>
#include <Network/MessageType.hpp>



// ------------------------------------------------------------------ explicit instantiations

template class ::network::OwnedMessage<::network::MessageType>;



// ------------------------------------------------------------------ *structors

template <
    ::detail::IsEnum MessageType
> ::network::OwnedMessage<MessageType>::OwnedMessage(
    ::std::shared_ptr<::network::Connection<MessageType>> remote,
    ::network::Message<MessageType> message
)
    : ::network::Message<MessageType>{ ::std::move(message) }
    , m_remote{ remote }
{}

template <
    ::detail::IsEnum MessageType
> ::network::OwnedMessage<MessageType>::~OwnedMessage() = default;



// ------------------------------------------------------------------ informations

template <
    ::detail::IsEnum MessageType
> auto ::network::OwnedMessage<MessageType>::getRemote() const
    -> ::std::shared_ptr<::network::Connection<MessageType>>
{
    return m_remote;
}
