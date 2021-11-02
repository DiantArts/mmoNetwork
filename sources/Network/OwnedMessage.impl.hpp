#pragma once

// ------------------------------------------------------------------ *structors

template <
    ::detail::isEnum UserMessageType
> ::network::OwnedMessage<UserMessageType>::OwnedMessage(
    ::network::Message<UserMessageType> message,
    ::std::shared_ptr<::network::TcpConnection<UserMessageType>> remote
)
    : ::network::Message<UserMessageType>{ ::std::move(message) }
    , m_remote{ remote }
{}

template <
    ::detail::isEnum UserMessageType
> ::network::OwnedMessage<UserMessageType>::~OwnedMessage() = default;



// ------------------------------------------------------------------ informations

template <
    ::detail::isEnum UserMessageType
> auto ::network::OwnedMessage<UserMessageType>::getRemote() const
    -> ::std::shared_ptr<::network::TcpConnection<UserMessageType>>
{
    return m_remote;
}
