#pragma once

#include <Network/ANode.hpp>
#include <Network/Udp/Connection.hpp>



namespace network::client {



template <
    typename UserMessageType
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

    [[ nodiscard ]] auto startConnectingToServer(
        const ::std::string& host,
        ::std::uint16_t port
    ) -> bool;

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
    template <
        typename... Args
    > void sendToServer(
        UserMessageType messageType,
        Args&&... args
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
    template <
        typename... Args
    > void sendToPeer(
        UserMessageType messageType,
        Args&&... args
    )
    {
        m_connectionToPeer->send(
            ::std::forward<decltype(messageType)>(messageType),
            ::std::forward<decltype(args)>(args)...
        );
    }


    [[ nodiscard ]] auto isConnectedToPeer() const
        -> bool;



    // ------------------------------------------------------------------ default behaviours

    virtual auto defaultReceiveBehaviour(
        ::network::Message<UserMessageType>& message,
        ::std::shared_ptr<::network::tcp::Connection<UserMessageType>> connection
    ) -> bool
        override final;

    virtual auto defaultReceiveBehaviour(
        ::network::Message<UserMessageType>& message,
        ::std::shared_ptr<::network::udp::Connection<UserMessageType>> connection
    ) -> bool
        override final;



    // ------------------------------------------------------------------ user behaviours

    virtual void onDisconnect(
        ::std::shared_ptr<::network::tcp::Connection<UserMessageType>> connection
    ) override;

    // refuses the identification by returning false, used to set username for exemple
    [[ nodiscard ]] virtual auto onAuthentification(
        ::std::shared_ptr<::network::tcp::Connection<UserMessageType>> connection
    ) -> bool
        override;

    virtual void onConnectionValidated(
        ::std::shared_ptr<::network::tcp::Connection<UserMessageType>> connection
    ) override;



    virtual void onDisconnect(
        ::std::shared_ptr<::network::udp::Connection<UserMessageType>> connection
    ) override;






protected:

    // hardware connection to the server
    ::std::shared_ptr<::network::tcp::Connection<UserMessageType>> m_tcpConnectionToServer;

    // TODO: implemente udp connection to server
    ::std::shared_ptr<::network::udp::Connection<UserMessageType>> m_udpConnectionToServer;
    ::std::shared_ptr<::network::udp::Connection<UserMessageType>> m_connectionToPeer;

};



} // namespace network::client

#include <Network/Client/AClient.impl.hpp>
