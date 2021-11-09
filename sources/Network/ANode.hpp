#pragma once

#include <Network/Tcp/Connection.hpp>
#include <Network/Udp/Connection.hpp>



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



    // ------------------------------------------------------------------ default behaviors

    // returns true meaning the message is already handled
    virtual auto defaultReceiveBehaviour(
        ::network::Message<UserMessageType>& message,
        ::std::shared_ptr<::network::tcp::Connection<UserMessageType>> connection
    ) -> bool = 0;

    virtual auto defaultReceiveBehaviour(
        ::network::Message<UserMessageType>& message,
        ::std::shared_ptr<::network::udp::Connection<UserMessageType>> connection
    ) -> bool = 0;



    // ------------------------------------------------------------------ tcp events

    // refuses the connection by returning false
    virtual auto onConnect(
        ::std::shared_ptr<::network::tcp::Connection<UserMessageType>> connection
    ) -> bool;

    // handle the disconnection
    virtual void onDisconnect(
        ::std::shared_ptr<::network::tcp::Connection<UserMessageType>> connection
    );

    // after receiving
    virtual void onReceive(
        ::network::Message<UserMessageType>& message,
        ::std::shared_ptr<::network::tcp::Connection<UserMessageType>> connection
    );

    // refuses the identification by returning false
    [[ nodiscard ]] virtual auto onIdentification(
        ::std::shared_ptr<::network::tcp::Connection<UserMessageType>> connection
    ) -> bool;

    // refuses the identification by returning false
    [[ nodiscard ]] virtual auto onAuthentification(
        ::std::shared_ptr<::network::tcp::Connection<UserMessageType>> connection
    ) -> bool
        = 0;

    virtual void onConnectionValidated(
        ::std::shared_ptr<::network::tcp::Connection<UserMessageType>> connection
    ) = 0;

    // server: denides
    // client: is denided
    virtual void onConnectionDenial(
        ::std::shared_ptr<::network::tcp::Connection<UserMessageType>> connection
    );

    // TODO: ban list
    virtual void onIdentificationDenial(
        ::std::shared_ptr<::network::tcp::Connection<UserMessageType>> connection
    );

    // TODO: implemente
    virtual void onAuthentificationDenial(
        ::std::shared_ptr<::network::tcp::Connection<UserMessageType>> connection
    );



    // ------------------------------------------------------------------ udp events

    virtual auto onConnect(
        ::std::shared_ptr<::network::udp::Connection<UserMessageType>> connection
    ) -> bool;

    virtual void onDisconnect(
        ::std::shared_ptr<::network::udp::Connection<UserMessageType>> connection
    );

    virtual void onReceive(
        ::network::Message<UserMessageType>& message,
        ::std::shared_ptr<::network::udp::Connection<UserMessageType>> connection
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
