#pragma once

#include <Network/ANode.hpp>
#include <Network/Connection.hpp>



namespace network::client {



template <
    ::detail::constraint::isEnum UserMessageType
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


    [[ nodiscard ]] auto isConnectedToServer() const
        -> bool;



    // ------------------------------------------------------------------ tcp

    void tcpSendToServer(
        ::network::Message<UserMessageType>::SystemType messageType,
        auto&&... args
    );

    void tcpSendToServer(
        UserMessageType messageType,
        auto&&... args
    );

    void tcpSendToServer(
        ::network::Message<UserMessageType>& message
    );

    void tcpSendToServer(
        ::network::Message<UserMessageType>&& message
    );



    // ------------------------------------------------------------------ udp

    void udpSendToServer(
        ::network::Message<UserMessageType>::SystemType messageType,
        auto&&... args
    );

    void udpSendToServer(
        UserMessageType messageType,
        auto&&... args
    );

    void udpSendToServer(
        ::network::Message<UserMessageType>& message
    );

    void udpSendToServer(
        ::network::Message<UserMessageType>&& message
    );



    // ------------------------------------------------------------------ default behaviours

    virtual auto defaultReceiveBehaviour(
        ::network::Message<UserMessageType>& message,
        ::std::shared_ptr<::network::Connection<UserMessageType>> connection
    ) -> bool
        override final;



    // ------------------------------------------------------------------ user behaviours

    virtual void onDisconnect(
        ::std::shared_ptr<::network::Connection<UserMessageType>> connection
    ) override;

    // refuses the identification by returning false, used to set username for exemple
    [[ nodiscard ]] virtual auto onAuthentification(
        ::std::shared_ptr<::network::Connection<UserMessageType>> connection
    ) -> bool
        override;

    virtual void onConnectionValidated(
        ::std::shared_ptr<::network::Connection<UserMessageType>> connection
    ) override;



    // ------------------------------------------------------------------ others

    template <
        ::network::SharableInformations::Index indexValue
    > void setInformation(
        ::detail::Id id,
        auto&&... args
    );

    template <
        ::network::SharableInformations::Index indexValue
    > void setInformation(
        auto&&... args
    );



protected:

    ::std::shared_ptr<::network::Connection<UserMessageType>> m_connectionToServer;

    ::std::map<::detail::Id, ::network::SharableInformations> m_connectedClientsInformations;

};



} // namespace network::client

#include <Network/Client/AClient.impl.hpp>
