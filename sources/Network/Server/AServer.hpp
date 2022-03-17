#pragma once

#include <Network/ANode.hpp>
#include <Network/Connection.hpp>



namespace network::server {



template <
    ::detail::constraint::isEnum UserMessageType
> class AServer
    : public ::network::ANode<UserMessageType>
{

public:

    // ------------------------------------------------------------------ *structors

    explicit AServer(
        const ::std::uint16_t port
    );

    virtual ~AServer() = 0;



    // ------------------------------------------------------------------ running

    [[ nodiscard ]] auto start()
        -> bool;

    void stop();

    [[ nodiscard ]] auto isRunning()
        -> bool;



    // ------------------------------------------------------------------ in

    void startReceivingConnections();



    // ------------------------------------------------------------------ tcpSend

    void tcpSendToClient(
        ::network::Message<UserMessageType>& message,
        ::std::shared_ptr<::network::Connection<UserMessageType>> client
    );

    void tcpSendToClient(
        ::network::Message<UserMessageType>&& message,
        ::std::shared_ptr<::network::Connection<UserMessageType>> client
    );

    void tcpSendToClient(
        ::network::Message<UserMessageType>& message,
        ::detail::Id clientId
    );

    void tcpSendToClient(
        ::network::Message<UserMessageType>&& message,
        ::detail::Id clientId
    );

    void tcpSendToClient(
        const ::network::Message<UserMessageType>& message,
        ::detail::constraint::sameAs<::std::shared_ptr<::network::Connection<UserMessageType>>> auto... clients
    );

    void tcpSendToAllClients(
        ::network::Message<UserMessageType>::SystemType messageType,
        auto&&... args
    );

    void tcpSendToAllClients(
        UserMessageType messageType,
        auto&&... args
    );

    void tcpSendToAllClients(
        const ::network::Message<UserMessageType>& message
    );

    void tcpSendToAllClients(
        const ::network::Message<UserMessageType>& message,
        ::detail::constraint::sameAs<::std::shared_ptr<::network::Connection<UserMessageType>>> auto... ignoredClients
    );

    void tcpSendToAllClients(
        const ::network::Message<UserMessageType>& message,
        ::detail::constraint::sameAs<::detail::Id> auto... ignoredClientIds
    );



    // ------------------------------------------------------------------ udpSend

    void udpSendToClient(
        ::network::Message<UserMessageType>& message,
        ::std::shared_ptr<::network::Connection<UserMessageType>> client
    );

    void udpSendToClient(
        ::network::Message<UserMessageType>&& message,
        ::std::shared_ptr<::network::Connection<UserMessageType>> client
    );

    void udpSendToClient(
        ::network::Message<UserMessageType>& message,
        ::detail::Id clientId
    );

    void udpSendToClient(
        ::network::Message<UserMessageType>&& message,
        ::detail::Id clientId
    );

    void udpSendToClient(
        const ::network::Message<UserMessageType>& message,
        ::detail::constraint::sameAs<::std::shared_ptr<::network::Connection<UserMessageType>>> auto... clients
    );

    void udpSendToAllClients(
        ::network::Message<UserMessageType>::SystemType messageType,
        auto&&... args
    );

    void udpSendToAllClients(
        UserMessageType messageType,
        auto&&... args
    );

    void udpSendToAllClients(
        const ::network::Message<UserMessageType>& message
    );

    void udpSendToAllClients(
        const ::network::Message<UserMessageType>& message,
        ::detail::constraint::sameAs<::std::shared_ptr<::network::Connection<UserMessageType>>> auto... ignoredClients
    );

    void udpSendToAllClients(
        const ::network::Message<UserMessageType>& message,
        ::detail::constraint::sameAs<::detail::Id> auto... ignoredClientIds
    );



    // ------------------------------------------------------------------ receive behaviour

    virtual auto defaultReceiveBehaviour(
        ::network::Message<UserMessageType>& message,
        ::std::shared_ptr<::network::Connection<UserMessageType>> connection
    ) -> bool
        override final;



    // ------------------------------------------------------------------ user behaviors

    // handle the disconnection
    virtual void onDisconnect(
        ::std::shared_ptr<::network::Connection<UserMessageType>> connection
    ) override;

    // refuses the identification by returning false, username received already
    [[ nodiscard ]] virtual auto onAuthentification(
        ::std::shared_ptr<::network::Connection<UserMessageType>> connection
    ) -> bool
        override;

    virtual void onConnectionValidated(
        ::std::shared_ptr<::network::Connection<UserMessageType>> connection
    ) override;



    virtual void onAuthentificationDenial(
        ::std::shared_ptr<::network::Connection<UserMessageType>> connection
    ) override;




    // ------------------------------------------------------------------ others

    [[ nodiscard ]] auto getConnection(
        ::detail::Id id
    ) -> ::std::shared_ptr<::network::Connection<UserMessageType>>;

    [[ nodiscard ]] auto getSharableInformations()
        -> ::std::map<::detail::Id, ::network::SharableInformations>;

    template <
        ::network::SharableInformations::Index indexValue
    > void setInformation(
        ::detail::Id id,
        auto&&... args
    );

    template <
        ::network::SharableInformations::Index indexValue
    > void setInformation(
        ::std::shared_ptr<::network::Connection<UserMessageType>> connection,
        auto&&... args
    );



private:

    // hardware connection to the server
    ::asio::ip::tcp::acceptor m_asioAcceptor;

    ::std::deque<::std::shared_ptr<::network::Connection<UserMessageType>>> m_incommingConnections;
    ::std::deque<::std::shared_ptr<::network::Connection<UserMessageType>>> m_connections;

    // TODO rooms
    // ::std::vector<::network::server::Room> m_rooms;

    ::detail::Id m_idCounter{ 0 };

};



} // namespace network::server

#include <Network/Server/AServer.impl.hpp>
