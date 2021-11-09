#pragma once

// ------------------------------------------------------------------ *structors

template <
    typename UserMessageType
> ::network::OwnedMessage<UserMessageType>::OwnedMessage(
    ::network::Message<UserMessageType> message,
    ::std::shared_ptr<::network::AConnection<UserMessageType>> remote
)
    : ::network::Message<UserMessageType>{ ::std::move(message) }
    , m_remote{ remote }
{}

template <
    typename UserMessageType
> ::network::OwnedMessage<UserMessageType>::~OwnedMessage() = default;



// ------------------------------------------------------------------ informations

template <
    class T,
    class U
> static inline auto my_static_pointer_cast(
    const std::shared_ptr<U>& r
) noexcept
    -> std::shared_ptr<T>
{
    auto p = static_cast<typename std::shared_ptr<T>::element_type*>(r.get());
    return std::shared_ptr<T>{r, p};
}

template <
    typename UserMessageType
> auto ::network::OwnedMessage<UserMessageType>::getRemoteAsTcp()
    -> ::std::shared_ptr<::network::tcp::Connection<UserMessageType>>
{
    if (this->getTransmissionProtocol() == ::network::Message<UserMessageType>::TransmissionProtocol::udp) {
        throw ::std::runtime_error("requesting a get remote as Tcp while the remote is a Udp remote");
    }
    return my_static_pointer_cast<::network::tcp::Connection<UserMessageType>>(m_remote);
}

template <
    typename UserMessageType
> auto ::network::OwnedMessage<UserMessageType>::getRemoteAsUdp()
    -> ::std::shared_ptr<::network::udp::Connection<UserMessageType>>
{
    if (this->getTransmissionProtocol() == ::network::Message<UserMessageType>::TransmissionProtocol::tcp) {
        throw ::std::runtime_error("requesting a get remote as Udp while the remote is a Tcp remote");
    }
    return my_static_pointer_cast<::network::udp::Connection<UserMessageType>>(m_remote);
}
