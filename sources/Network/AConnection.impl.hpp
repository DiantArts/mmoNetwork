#pragma once



// ------------------------------------------------------------------ *structors

template <
    ::detail::isEnum UserMessageType
> ::network::AConnection<UserMessageType>::AConnection(
    ::network::ANode<UserMessageType>& owner
)
    : m_owner{ owner }
{}


template <
    ::detail::isEnum UserMessageType
> ::network::AConnection<UserMessageType>::~AConnection() = default;
