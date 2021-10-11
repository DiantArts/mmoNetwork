#pragma once

#include <Detail/Id.hpp>
#include <Detail/Concepts.hpp>
#include <Network/Connection.hpp>
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
        -> ::boost::asio::io_context&;

    [[ nodiscard ]] auto getThreadContext()
        -> ::std::thread&;

    [[ nodiscard ]] auto getType()
        -> ANode<MessageType>::Type;



    // ------------------------------------------------------------------ user methods

    // handle the disconnection
    virtual void onDisconnect(
        ::std::shared_ptr<::network::Connection<MessageType>> connection
    );



    // after receiving
    virtual void onTcpReceive(
        ::network::Message<MessageType>& message,
        ::std::shared_ptr<::network::Connection<MessageType>> connection
    );

    virtual void onUdpReceive(
        ::network::Message<MessageType>& message,
        ::std::shared_ptr<::network::Connection<MessageType>> connection
    );

    // returns true meaning the message is already handled
    virtual auto defaultReceiveBehaviour(
        ::network::Message<MessageType>& message,
        ::std::shared_ptr<::network::Connection<MessageType>> connection
    ) -> bool = 0;



    // before sending
    virtual void onTcpSend(
        ::network::Message<MessageType>& message,
        ::std::shared_ptr<::network::Connection<MessageType>> connection
    );

    virtual void onUdpSend(
        ::network::Message<MessageType>& message,
        ::std::shared_ptr<::network::Connection<MessageType>> connection
    );



    // ------------------------------------------------------------------ error user methods
    // server: denides
    // client: is denided

    virtual void onConnectionDenial(
        ::std::shared_ptr<::network::Connection<MessageType>> connection
    );

    // TODO: ban list
    virtual void onIdentificationDenial(
        ::std::shared_ptr<::network::Connection<MessageType>> connection
    );



private:

    // context running on a seperate thread
    ::boost::asio::io_context m_asioContext;
    ::std::thread m_threadContext;

    ::detail::Queue<::network::OwnedMessage<MessageType>> m_messagesIn;

    ANode<MessageType>::Type m_type;

};



} // namespace network
