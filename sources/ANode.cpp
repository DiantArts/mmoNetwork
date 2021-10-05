#include <pch.hpp>
#include <ANode.hpp>
#include <MessageType.hpp>



// ------------------------------------------------------------------ explicit instantiations

template class ::network::ANode<::network::MessageType>;



// ------------------------------------------------------------------ *structors

template <
    ::network::detail::IsEnum MessageType
> ::network::ANode<MessageType>::ANode(
    ANode<MessageType>::Type type
)
    : m_type{ type }
{}

template <
    ::network::detail::IsEnum MessageType
> ::network::ANode<MessageType>::~ANode() = default;



// ------------------------------------------------------------------ async - in

template <
    ::network::detail::IsEnum MessageType
> void ::network::ANode<MessageType>::pullIncommingMessage()
{
    auto message{ m_messagesIn.pop_front() };
    this->onReceive(message, message.getRemote());
}

template <
    ::network::detail::IsEnum MessageType
> void ::network::ANode<MessageType>::pullIncommingMessages()
{
    while (!m_messagesIn.empty()) {
        this->pullIncommingMessage();
    }
}

template <
    ::network::detail::IsEnum MessageType
> void ::network::ANode<MessageType>::blockingPullIncommingMessages()
{
    m_messagesIn.wait();
    this->pullIncommingMessages();
}

template <
    ::network::detail::IsEnum MessageType
> auto ::network::ANode<MessageType>::getIncommingMessages()
    -> ::network::Queue<::network::OwnedMessage<MessageType>>&
{
    return m_messagesIn;
}



// ------------------------------------------------------------------ getter

template <
    ::network::detail::IsEnum MessageType
> auto ::network::ANode<MessageType>::getAsioContext()
    -> ::boost::asio::io_context&
{
    return m_asioContext;
}

template <
    ::network::detail::IsEnum MessageType
> auto ::network::ANode<MessageType>::getThreadContext()
    -> ::std::thread&
{
    return m_threadContext;
}

template <
    ::network::detail::IsEnum MessageType
> auto ::network::ANode<MessageType>::getType()
    -> ANode<MessageType>::Type
{
    return m_type;
}



// ------------------------------------------------------------------ user methods

// before the actual disconnection
template <
    ::network::detail::IsEnum MessageType
> void ::network::ANode<MessageType>::onDisconnect(
    ::std::shared_ptr<::network::Connection<MessageType>> connection
)
{
    ::std::cout << "[" << connection->getId() << "] Disconnected\n";
}

// after receiving
template <
    ::network::detail::IsEnum MessageType
> void ::network::ANode<MessageType>::onReceive(
    const ::network::Message<MessageType>& message,
    ::std::shared_ptr<::network::Connection<MessageType>> connection
)
{}

// before sending
template <
    ::network::detail::IsEnum MessageType
> void ::network::ANode<MessageType>::onSend(
    const ::network::Message<MessageType>& message,
    ::std::shared_ptr<::network::Connection<MessageType>> connection
)
{}



// ------------------------------------------------------------------ error user methods

template <
    ::network::detail::IsEnum MessageType
> void ::network::ANode<MessageType>::onConnectionDenial(
    ::std::shared_ptr<::network::Connection<MessageType>> connection
)
{}

template <
    ::network::detail::IsEnum MessageType
> void ::network::ANode<MessageType>::onIdentificationDenial(
    ::std::shared_ptr<::network::Connection<MessageType>> connection
)
{
    ::std::cerr << "[" << connection->getId() << "] Identification denied\n";
    connection->send(::network::MessageType::IdentificationDenied);
}
