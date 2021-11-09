#pragma once



// ------------------------------------------------------------------ *structors

template <
    typename UserMessageType
> ::network::AConnection<UserMessageType>::AConnection(
    ::network::ANode<UserMessageType>& owner
)
    : m_owner{ owner }
{}


template <
    typename UserMessageType
> ::network::AConnection<UserMessageType>::~AConnection() = default;
