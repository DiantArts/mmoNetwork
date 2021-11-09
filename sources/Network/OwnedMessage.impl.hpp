#pragma once

// ------------------------------------------------------------------ *structors

template <
    ::detail::isEnum UserMessageType
> ::network::OwnedMessage<UserMessageType>::OwnedMessage(
    ::network::Message<UserMessageType> message,
    ::std::shared_ptr<::network::AConnection<UserMessageType>> remote
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
> auto ::network::OwnedMessage<UserMessageType>::getRemoteAsTcp()
    -> ::std::shared_ptr<::network::tcp::Connection<UserMessageType>>
{
    if (this->getTransmissionProtocol() == ::network::Message<UserMessageType>::TransmissionProtocol::udp) {
        throw ::std::runtime_error("requesting a get remote as Tcp while the remote is a Udp remote");
    }
    return static_pointer_cast<::network::tcp::Connection<UserMessageType>>(m_remote);
}

template <
    ::detail::isEnum UserMessageType
> auto ::network::OwnedMessage<UserMessageType>::getRemoteAsUdp()
    -> ::std::shared_ptr<::network::udp::Connection<UserMessageType>>
{
    if (this->getTransmissionProtocol() == ::network::Message<UserMessageType>::TransmissionProtocol::tcp) {
        throw ::std::runtime_error("requesting a get remote as Udp while the remote is a Tcp remote");
    }
    return static_pointer_cast<::network::udp::Connection<UserMessageType>>(m_remote);
}
