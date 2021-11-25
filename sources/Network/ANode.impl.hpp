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
    const auto& remote{ message.getRemote() };
    if (!this->defaultReceiveBehaviour(message, remote)) {
        this->onReceive(message, remote);
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



// ------------------------------------------------------------------ events

template <
    ::detail::isEnum UserMessageType
> auto ::network::ANode<UserMessageType>::onConnect(
    ::std::shared_ptr<::network::Connection<UserMessageType>> connection
) -> bool
{
    ::std::cout << "[ANode] onConnect.\n";
    return true;
}

template <
    ::detail::isEnum UserMessageType
> void ::network::ANode<UserMessageType>::onDisconnect(
    ::std::shared_ptr<::network::Connection<UserMessageType>> connection
)
{
    ::std::cout << "[ANode:" << connection->informations.userName << "] onDisconnect.\n";
}

// after receiving
template <
    ::detail::isEnum UserMessageType
> void ::network::ANode<UserMessageType>::onReceive(
    ::network::Message<UserMessageType>& message,
    ::std::shared_ptr<::network::Connection<UserMessageType>> connection
)
{
    ::std::cout << "[ANode] onReceive.\n";
}

template <
    ::detail::isEnum UserMessageType
> auto ::network::ANode<UserMessageType>::onIdentification(
    ::std::shared_ptr<::network::Connection<UserMessageType>> connection
) -> bool
{
    ::std::cout << "[ANode:" << connection->informations.userName << "] onIdentification.\n";
    return true;
}

template <
    ::detail::isEnum UserMessageType
> void ::network::ANode<UserMessageType>::onConnectionDenial(
    ::std::shared_ptr<::network::Connection<UserMessageType>> connection
)
{
    ::std::cout << "[ANode] onConnectionDenial.\n";
}

template <
    ::detail::isEnum UserMessageType
> void ::network::ANode<UserMessageType>::onIdentificationDenial(
    ::std::shared_ptr<::network::Connection<UserMessageType>> connection
)
{
    ::std::cerr << "[ANode:" << connection->informations.userName << "] Identification denied.\n";
}

template <
    ::detail::isEnum UserMessageType
> void ::network::ANode<UserMessageType>::onAuthentificationDenial(
    ::std::shared_ptr<::network::Connection<UserMessageType>> connection
)
{
    ::std::cerr << "[ANode:" << connection->informations.userName << "] Authentification denied.\n";
}
