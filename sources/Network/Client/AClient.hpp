#pragma once

#include <Network/ANode.hpp>
#include <Network/UdpConnection.hpp>



namespace network {



template <
    ::detail::isEnum UserMessageType
> class AClient
    : public ::network::ANode<UserMessageType>
{

public:

    // ------------------------------------------------------------------ *structors

    explicit AClient();

    virtual ~AClient() = 0;



    // ------------------------------------------------------------------ connection

    void disconnect();

    [[ nodiscard ]] auto isConnected() const
        -> bool;

    [[ nodiscard ]] auto getUdpPort() const
        -> ::std::uint16_t;



    // ------------------------------------------------------------------ server

    [[ nodiscard ]] auto connectToServer(
        const ::std::string& host,
        ::std::uint16_t port
    ) -> bool;

    void disconnectFromServer();

    void sendToServer(
        ::network::Message<UserMessageType>& message
    );

    void sendToServer(
        ::network::Message<UserMessageType>&& message
    );

    // construct and tcpSend
    void sendToServer(
        UserMessageType messageType,
        auto&&... args
    )
    {
        m_tcpConnectionToServer->send(
            ::std::forward<decltype(messageType)>(messageType),
            ::std::forward<decltype(args)>(args)...
        );
    }


    [[ nodiscard ]] auto isConnectedToServer() const
        -> bool;



    // ------------------------------------------------------------------ peer

    void openUdpConnection();

    auto targetPeer(
        const ::std::string& host,
        ::std::uint16_t port
    ) -> bool;

    void closePeerConnection();

    void sendToPeer(
        ::network::Message<UserMessageType>&& message
    );

    // construct and tcpSend
    void sendToPeer(
        UserMessageType messageType,
        auto&&... args
    )
    {
        m_connectionToPeer->send(
            ::std::forward<decltype(messageType)>(messageType),
            ::std::forward<decltype(args)>(args)...
        );
    }


    [[ nodiscard ]] auto isConnectedToPeer() const
        -> bool;



    // ------------------------------------------------------------------ user behaviours

    virtual auto defaultReceiveBehaviour(
        ::network::Message<UserMessageType>& message,
        ::std::shared_ptr<::network::TcpConnection<UserMessageType>> connection
    ) -> bool
        override final;

    virtual void onDisconnect(
        ::std::shared_ptr<::network::TcpConnection<UserMessageType>> connection
    ) override;

    // refuses the identification by returning false, used to set username for exemple
    [[ nodiscard ]] virtual auto onAuthentification(
        ::std::shared_ptr<::network::TcpConnection<UserMessageType>> connection
    ) -> bool
        override;

    virtual void onConnectionValidated(
        ::std::shared_ptr<::network::TcpConnection<UserMessageType>> connection
    ) override;



    // handle the disconnection
    virtual void onUdpDisconnect(
        ::std::shared_ptr<::network::UdpConnection<UserMessageType>> connection
    );



protected:

    // hardware connection to the server
    ::std::shared_ptr<::network::TcpConnection<UserMessageType>> m_tcpConnectionToServer;
    // TODO: implemente udp connection to server
    ::std::shared_ptr<::network::UdpConnection<UserMessageType>> m_udpConnectionToServer;
    ::std::shared_ptr<::network::UdpConnection<UserMessageType>> m_connectionToPeer;

};



} // namespace network

#include <Network/Client/AClient.impl.hpp>
