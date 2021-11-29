#pragma once

// ------------------------------------------------------------------ *structors

template <
    ::detail::constraint::isEnum UserMessageType
> ::network::OwnedMessage<UserMessageType>::OwnedMessage(
    ::network::Message<UserMessageType> message,
    ::std::shared_ptr<::network::Connection<UserMessageType>> remote
)
    : ::network::Message<UserMessageType>{ ::std::move(message) }
    , m_remote{ remote }
{}

template <
    ::detail::constraint::isEnum UserMessageType
> ::network::OwnedMessage<UserMessageType>::~OwnedMessage() = default;



// ------------------------------------------------------------------ informations

template <
    ::detail::constraint::isEnum UserMessageType
> auto ::network::OwnedMessage<UserMessageType>::getRemote()
    -> ::std::shared_ptr<::network::Connection<UserMessageType>>
{
    return m_remote;
}
