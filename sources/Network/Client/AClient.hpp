#pragma once

#include <Network/ANode.hpp>
#include <Network/UdpConnection.hpp>



namespace network {



template <
    ::detail::isEnum MessageType
> class AClient
    : public ::network::ANode<MessageType>
{

public:

    // ------------------------------------------------------------------ *structors

    explicit AClient();

    virtual ~AClient() = 0;



    // ------------------------------------------------------------------ connection

    void disconnect();

    [[ nodiscard ]] auto isConnected()
        -> bool;

    [[ nodiscard ]] auto getUdpPort()
        -> ::std::uint16_t;



    // ------------------------------------------------------------------ server

    [[ nodiscard ]] auto connectToServer(
        const ::std::string& host,
        ::std::uint16_t port
    ) -> bool;

    void disconnectFromServer();

    void sendToServer(
        ::network::Message<MessageType>& message
    );

    void sendToServer(
        ::network::Message<MessageType>&& message
    );

    // construct and tcpSend
    void sendToServer(
        MessageType messageType,
        auto&&... args
    )
    {
        m_connectionToServer->send(
            ::std::forward<decltype(messageType)>(messageType),
            ::std::forward<decltype(args)>(args)...
        );
    }


    [[ nodiscard ]] auto isConnectedToServer()
        -> bool;



    // ------------------------------------------------------------------ peer

    void openUdpConnection();

    auto targetPeer(
        const ::std::string& host,
        ::std::uint16_t port
    ) -> bool;

    void closePeerConnection();

    void sendToPeer(
        ::network::Message<MessageType>&& message
    );

    // construct and tcpSend
    void sendToPeer(
        MessageType messageType,
        auto&&... args
    )
    {
        m_connectionToPeer->send(
            ::std::forward<decltype(messageType)>(messageType),
            ::std::forward<decltype(args)>(args)...
        );
    }


    [[ nodiscard ]] auto isConnectedToPeer()
        -> bool;



    // ------------------------------------------------------------------ default behaviours

    virtual auto defaultReceiveBehaviour(
        ::network::Message<MessageType>& message,
        ::std::shared_ptr<::network::TcpConnection<MessageType>> connection
    ) -> bool
        override final;

    virtual void onDisconnect(
        ::std::shared_ptr<::network::TcpConnection<MessageType>> connection
    ) override;

    // handle the disconnection
    virtual void onUdpDisconnect(
        ::std::shared_ptr<::network::UdpConnection<MessageType>> connection
    );


protected:

    // hardware connection to the server
    ::std::shared_ptr<::network::TcpConnection<MessageType>> m_connectionToServer;
    ::std::shared_ptr<::network::UdpConnection<MessageType>> m_connectionToPeer;

};



} // namespace network
