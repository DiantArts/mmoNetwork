#include <pch.hpp>
#include <Network/ANode.hpp>
#include <Network/MessageType.hpp>



// ------------------------------------------------------------------ explicit instantiations

template class ::network::ANode<::network::MessageType>;



// ------------------------------------------------------------------ *structors

template <
    ::detail::isEnum MessageType
> ::network::ANode<MessageType>::ANode(
    ANode<MessageType>::Type type
)
    : m_type{ type }
{}

template <
    ::detail::isEnum MessageType
> ::network::ANode<MessageType>::~ANode() = default;



// ------------------------------------------------------------------ async - in

template <
    ::detail::isEnum MessageType
> void ::network::ANode<MessageType>::pullIncommingMessage()
{
    auto message{ m_messagesIn.pop_front() };
    if (!this->defaultReceiveBehaviour(message, message.getRemote())) {
        this->onTcpReceive(message, message.getRemote());
    }
}

template <
    ::detail::isEnum MessageType
> void ::network::ANode<MessageType>::pullIncommingMessages()
{
    while (!m_messagesIn.empty()) {
        this->pullIncommingMessage();
    }
}

template <
    ::detail::isEnum MessageType
> void ::network::ANode<MessageType>::blockingPullIncommingMessages()
{
    m_messagesIn.wait();
    this->pullIncommingMessages();
}

template <
    ::detail::isEnum MessageType
> auto ::network::ANode<MessageType>::getIncommingMessages()
    -> ::detail::Queue<::network::OwnedMessage<MessageType>>&
{
    return m_messagesIn;
}



// ------------------------------------------------------------------ getter

template <
    ::detail::isEnum MessageType
> auto ::network::ANode<MessageType>::getAsioContext()
    -> ::boost::asio::io_context&
{
    return m_asioContext;
}

template <
    ::detail::isEnum MessageType
> auto ::network::ANode<MessageType>::getThreadContext()
    -> ::std::thread&
{
    return m_threadContext;
}

template <
    ::detail::isEnum MessageType
> auto ::network::ANode<MessageType>::getType()
    -> ANode<MessageType>::Type
{
    return m_type;
}



// ------------------------------------------------------------------ user methods

// before the actual disconnection
template <
    ::detail::isEnum MessageType
> void ::network::ANode<MessageType>::onDisconnect(
    ::std::shared_ptr<::network::Connection<MessageType>> connection
)
{
    ::std::cout << "[" << connection->getId() << "] Disconnected\n";
}



// after receiving
template <
    ::detail::isEnum MessageType
> void ::network::ANode<MessageType>::onTcpReceive(
    ::network::Message<MessageType>& message,
    ::std::shared_ptr<::network::Connection<MessageType>> connection
)
{}

template <
    ::detail::isEnum MessageType
> void ::network::ANode<MessageType>::onUdpReceive(
    ::network::Message<MessageType>& message,
    ::std::shared_ptr<::network::Connection<MessageType>> connection
)
{}



template <
    ::detail::isEnum MessageType
> void ::network::ANode<MessageType>::onTcpSend(
    ::network::Message<MessageType>& message,
    ::std::shared_ptr<::network::Connection<MessageType>> connection
)
{}

template <
    ::detail::isEnum MessageType
> void ::network::ANode<MessageType>::onUdpSend(
    ::network::Message<MessageType>& message,
    ::std::shared_ptr<::network::Connection<MessageType>> connection
)
{}



// ------------------------------------------------------------------ error user methods

template <
    ::detail::isEnum MessageType
> void ::network::ANode<MessageType>::onConnectionDenial(
    ::std::shared_ptr<::network::Connection<MessageType>> connection
)
{}

template <
    ::detail::isEnum MessageType
> void ::network::ANode<MessageType>::onIdentificationDenial(
    ::std::shared_ptr<::network::Connection<MessageType>> connection
)
{
    ::std::cerr << "[" << connection->getId() << "] Identification denied\n";
    connection->tcpSend(MessageType::identificationDenied);
}
