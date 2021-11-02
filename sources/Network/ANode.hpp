#pragma once

#include <Detail/Id.hpp>
#include <Detail/Concepts.hpp>
#include <Network/TcpConnection.hpp>



namespace network {



template <
    ::detail::isEnum UserMessageType
> class ANode {

public:

    enum class Type {
        server,
        client
    };



public:

    // ------------------------------------------------------------------ *structors

    explicit ANode(
        ANode<UserMessageType>::Type type
    );

    virtual ~ANode() = 0;

    void stopThread();



    // ------------------------------------------------------------------ async - in

    void pullIncommingMessage();

    void pullIncommingMessages();

    void blockingPullIncommingMessages();



    void pushIncommingMessage(
        auto&&... args
    );

    auto getIncommingMessages()
        -> ::detail::Queue<::network::OwnedMessage<UserMessageType>>&;



    // ------------------------------------------------------------------ getter

    [[ nodiscard ]] auto getAsioContext()
        -> ::asio::io_context&;

    [[ nodiscard ]] auto getThreadContext()
        -> ::std::thread&;

    [[ nodiscard ]] auto getType()
        -> ANode<UserMessageType>::Type;



    // ------------------------------------------------------------------ user methods

    // refuses the connection by returning false
    virtual auto onConnect(
        ::std::shared_ptr<::network::TcpConnection<UserMessageType>> connection
    ) -> bool;

    // handle the disconnection
    virtual void onDisconnect(
        ::std::shared_ptr<::network::TcpConnection<UserMessageType>> connection
    );


    // refuses the identification by returning false
    [[ nodiscard ]] virtual auto onIdentification(
        ::std::shared_ptr<::network::TcpConnection<UserMessageType>> connection
    ) -> bool;

    // refuses the identification by returning false
    [[ nodiscard ]] virtual auto onAuthentification(
        ::std::shared_ptr<::network::TcpConnection<UserMessageType>> connection
    ) -> bool
        = 0;

    virtual void onConnectionValidated(
        ::std::shared_ptr<::network::TcpConnection<UserMessageType>> connection
    ) = 0;



    // after receiving
    virtual void onTcpReceive(
        ::network::Message<UserMessageType>& message,
        ::std::shared_ptr<::network::TcpConnection<UserMessageType>> connection
    );

    virtual void onUdpReceive(
        ::network::Message<UserMessageType>& message,
        ::std::shared_ptr<::network::TcpConnection<UserMessageType>> connection
    );

    // returns true meaning the message is already handled
    virtual auto defaultReceiveBehaviour(
        ::network::Message<UserMessageType>& message,
        ::std::shared_ptr<::network::TcpConnection<UserMessageType>> connection
    ) -> bool = 0;



    // ------------------------------------------------------------------ error user methods
    // server: denides
    // client: is denided

    virtual void onConnectionDenial(
        ::std::shared_ptr<::network::TcpConnection<UserMessageType>> connection
    );

    // TODO: ban list
    virtual void onIdentificationDenial(
        ::std::shared_ptr<::network::TcpConnection<UserMessageType>> connection
    );

    // TODO: implemente
    virtual void onAuthentificationDenial(
        ::std::shared_ptr<::network::TcpConnection<UserMessageType>> connection
    );



private:

    // context running on a seperate thread
    ::asio::io_context m_asioContext;
    ::std::thread m_threadContext;

    ::detail::Queue<::network::OwnedMessage<UserMessageType>> m_messagesIn;

    ANode<UserMessageType>::Type m_type;

};



} // namespace network

#include <Network/ANode.impl.hpp>
