#pragma once

// ------------------------------------------------------------------ *structors

template <
    ::detail::isEnum UserMessageType
> ::network::ANode<UserMessageType>::ANode(
    ANode<UserMessageType>::Type type
)
    : m_type{ type }
{}

template <
    ::detail::isEnum UserMessageType
> ::network::ANode<UserMessageType>::~ANode()
{
    this->stopThread();
}

template <
    ::detail::isEnum UserMessageType
> void ::network::ANode<UserMessageType>::stopThread()
{
    m_asioContext.stop();
    if(m_threadContext.joinable()) {
        m_threadContext.join();
    }
}



// ------------------------------------------------------------------ async - in

template <
    ::detail::isEnum UserMessageType
> void ::network::ANode<UserMessageType>::pullIncommingMessage()
{
    auto message{ this->getIncommingMessages().pop_front() };
    if (!this->defaultReceiveBehaviour(message, message.getRemote())) {
        if (
            message.getTransmissionProtocol() ==
            ::network::Message<UserMessageType>::TransmissionProtocol::tcp
        ) {
            this->onTcpReceive(message, message.getRemote());
        } else if (
            message.getTransmissionProtocol() ==
            ::network::Message<UserMessageType>::TransmissionProtocol::udp
        ) {
            this->onUdpReceive(message, message.getRemote());
        }
    }
}

template <
    ::detail::isEnum UserMessageType
> void ::network::ANode<UserMessageType>::pullIncommingMessages()
{
    while (!this->getIncommingMessages().empty()) {
        this->pullIncommingMessage();
    }
}

template <
    ::detail::isEnum UserMessageType
> void ::network::ANode<UserMessageType>::blockingPullIncommingMessages()
{
    this->getIncommingMessages().wait();
    this->pullIncommingMessages();
}



template <
    ::detail::isEnum UserMessageType
> void ::network::ANode<UserMessageType>::pushIncommingMessage(
    auto&&... args
)
{
    m_messagesIn.push_back(::std::forward<decltype(args)>(args)...);
}

template <
    ::detail::isEnum UserMessageType
> auto ::network::ANode<UserMessageType>::getIncommingMessages()
    -> ::detail::Queue<::network::OwnedMessage<UserMessageType>>&
{
    return m_messagesIn;
}



// ------------------------------------------------------------------ getter

template <
    ::detail::isEnum UserMessageType
> auto ::network::ANode<UserMessageType>::getAsioContext()
    -> ::asio::io_context&
{
    return m_asioContext;
}

template <
    ::detail::isEnum UserMessageType
> auto ::network::ANode<UserMessageType>::getThreadContext()
    -> ::std::thread&
{
    return m_threadContext;
}

template <
    ::detail::isEnum UserMessageType
> auto ::network::ANode<UserMessageType>::getType()
    -> ANode<UserMessageType>::Type
{
    return m_type;
}



// ------------------------------------------------------------------ user methods

template <
    ::detail::isEnum UserMessageType
> auto ::network::ANode<UserMessageType>::onConnect(
    ::std::shared_ptr<::network::TcpConnection<UserMessageType>> connection
) -> bool
{
    ::std::cout << "[ANode:TCP] onConnect.\n";
    return true;
}

template <
    ::detail::isEnum UserMessageType
> void ::network::ANode<UserMessageType>::onDisconnect(
    ::std::shared_ptr<::network::TcpConnection<UserMessageType>> connection
)
{
    ::std::cout << "[ANode:TCP:" << connection->getId() << "] onDisconnect.\n";
}



template <
    ::detail::isEnum UserMessageType
> auto ::network::ANode<UserMessageType>::onIdentification(
    ::std::shared_ptr<::network::TcpConnection<UserMessageType>> connection
) -> bool
{
    ::std::cout << "[ANode:TCP:" << connection->getId() << "] onIdentification.\n";
    return true;
}



// after receiving
template <
    ::detail::isEnum UserMessageType
> void ::network::ANode<UserMessageType>::onTcpReceive(
    ::network::Message<UserMessageType>& message,
    ::std::shared_ptr<::network::TcpConnection<UserMessageType>> connection
)
{
    ::std::cout << "[ANode:TCP] onReceive.\n";
}

template <
    ::detail::isEnum UserMessageType
> void ::network::ANode<UserMessageType>::onUdpReceive(
    ::network::Message<UserMessageType>& message,
    ::std::shared_ptr<::network::TcpConnection<UserMessageType>> connection
)
{
    ::std::cout << "[ANode:UDP] onReceive.\n";
}



// ------------------------------------------------------------------ error user methods

template <
    ::detail::isEnum UserMessageType
> void ::network::ANode<UserMessageType>::onConnectionDenial(
    ::std::shared_ptr<::network::TcpConnection<UserMessageType>> connection
)
{
    ::std::cout << "[ANode:TCP] onConnectionDenial.\n";
}

template <
    ::detail::isEnum UserMessageType
> void ::network::ANode<UserMessageType>::onIdentificationDenial(
    ::std::shared_ptr<::network::TcpConnection<UserMessageType>> connection
)
{
    ::std::cerr << "[ANode:TCP:" << connection->getId() << "] Identification denied.\n";
}

template <
    ::detail::isEnum UserMessageType
> void ::network::ANode<UserMessageType>::onAuthentificationDenial(
    ::std::shared_ptr<::network::TcpConnection<UserMessageType>> connection
)
{
    ::std::cerr << "[ANode:TCP:" << connection->getId() << "] Authentification denied.\n";
}
