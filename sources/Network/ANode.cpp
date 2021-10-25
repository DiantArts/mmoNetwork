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
> ::network::ANode<MessageType>::~ANode()
{
    this->stopThread();
}

template <
    ::detail::isEnum MessageType
> void ::network::ANode<MessageType>::stopThread()
{
    m_asioContext.stop();
    if(m_threadContext.joinable()) {
        m_threadContext.join();
    }
}



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
    -> ::asio::io_context&
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

template <
    ::detail::isEnum MessageType
> auto ::network::ANode<MessageType>::onConnect(
    ::std::shared_ptr<::network::TcpConnection<MessageType>> connection
) -> bool
{
    ::std::cout << "[ANode:TCP] OnConnect.\n";
    return true;
}

template <
    ::detail::isEnum MessageType
> void ::network::ANode<MessageType>::onDisconnect(
    ::std::shared_ptr<::network::TcpConnection<MessageType>> connection
)
{
    ::std::cout << "[ANode:TCP:" << connection->getId() << "] OnDisconnect.\n";
}



template <
    ::detail::isEnum MessageType
> auto ::network::ANode<MessageType>::onIdentificate(
    ::std::shared_ptr<::network::TcpConnection<MessageType>> connection
) -> bool
{
    ::std::cout << "[ANode:TCP:" << connection->getId() << "] OnIdentificate.\n";
    return true;
}



// after receiving
template <
    ::detail::isEnum MessageType
> void ::network::ANode<MessageType>::onTcpReceive(
    ::network::Message<MessageType>& message,
    ::std::shared_ptr<::network::TcpConnection<MessageType>> connection
)
{
    ::std::cout << "[ANode:TCP] OnReceive.\n";
}

template <
    ::detail::isEnum MessageType
> void ::network::ANode<MessageType>::onUdpReceive(
    ::network::Message<MessageType>& message,
    ::std::shared_ptr<::network::TcpConnection<MessageType>> connection
)
{
    ::std::cout << "[ANode:UDP] OnReceive.\n";
}



// ------------------------------------------------------------------ error user methods

template <
    ::detail::isEnum MessageType
> void ::network::ANode<MessageType>::onConnectionDenial(
    ::std::shared_ptr<::network::TcpConnection<MessageType>> connection
)
{
    ::std::cout << "[ANode:TCP] OnConnectionDenial.\n";
}

template <
    ::detail::isEnum MessageType
> void ::network::ANode<MessageType>::onIdentificationDenial(
    ::std::shared_ptr<::network::TcpConnection<MessageType>> connection
)
{
    ::std::cerr << "[ANode:TCP:" << connection->getId() << "] Identification denied\n";
    connection->send(MessageType::identificationDenied);
}
