#pragma once

#include <Detail/Id.hpp>
#include <Detail/Concepts.hpp>
#include <Network/TcpConnection.hpp>
#include <Network/MessageType.hpp>



namespace network {



template <
    ::detail::isEnum MessageType
> class ANode {

public:

    enum class Type {
        server,
        client
    };



public:

    // ------------------------------------------------------------------ *structors

    explicit ANode(
        ANode<MessageType>::Type type
    );

    virtual ~ANode() = 0;

    void stopThread();



    // ------------------------------------------------------------------ async - in

    void pullIncommingMessage();

    void pullIncommingMessages();

    void blockingPullIncommingMessages();

    void pushIncommingMessage(
        auto&&... args
    )
    {
        m_messagesIn.push_back(::std::forward<decltype(args)>(args)...);
    }

    auto getIncommingMessages()
        -> ::detail::Queue<::network::OwnedMessage<MessageType>>&;



    // ------------------------------------------------------------------ getter

    [[ nodiscard ]] auto getAsioContext()
        -> ::asio::io_context&;

    [[ nodiscard ]] auto getThreadContext()
        -> ::std::thread&;

    [[ nodiscard ]] auto getType()
        -> ANode<MessageType>::Type;



    // ------------------------------------------------------------------ user methods

    // refuses the connection by returning false
    virtual auto onConnect(
        ::std::shared_ptr<::network::TcpConnection<MessageType>> connection
    ) -> bool;

    // handle the disconnection
    virtual void onDisconnect(
        ::std::shared_ptr<::network::TcpConnection<MessageType>> connection
    );



    // refuses the identification by returning false
    [[ nodiscard ]] virtual auto onIdentificate(
        ::std::shared_ptr<::network::TcpConnection<MessageType>> connection
    ) -> bool;



    // after receiving
    virtual void onTcpReceive(
        ::network::Message<MessageType>& message,
        ::std::shared_ptr<::network::TcpConnection<MessageType>> connection
    );

    virtual void onUdpReceive(
        ::network::Message<MessageType>& message,
        ::std::shared_ptr<::network::TcpConnection<MessageType>> connection
    );

    // returns true meaning the message is already handled
    virtual auto defaultReceiveBehaviour(
        ::network::Message<MessageType>& message,
        ::std::shared_ptr<::network::TcpConnection<MessageType>> connection
    ) -> bool = 0;



    // ------------------------------------------------------------------ error user methods
    // server: denides
    // client: is denided

    virtual void onConnectionDenial(
        ::std::shared_ptr<::network::TcpConnection<MessageType>> connection
    );

    // TODO: ban list
    virtual void onIdentificationDenial(
        ::std::shared_ptr<::network::TcpConnection<MessageType>> connection
    );



private:

    // context running on a seperate thread
    ::asio::io_context m_asioContext;
    ::std::thread m_threadContext;

    ::detail::Queue<::network::OwnedMessage<MessageType>> m_messagesIn;

    ANode<MessageType>::Type m_type;

};



} // namespace network
