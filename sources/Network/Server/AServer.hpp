#pragma once

#include <Network/ANode.hpp>



namespace network::server {



template <
    ::detail::isEnum UserMessageType
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



    // ------------------------------------------------------------------ out

    void send(
        ::network::Message<UserMessageType>& message,
        ::std::shared_ptr<::network::tcp::Connection<UserMessageType>> client
    );

    void send(
        ::network::Message<UserMessageType>&& message,
        ::std::shared_ptr<::network::tcp::Connection<UserMessageType>> client
    );

    void send(
        ::network::Message<UserMessageType>& message,
        ::detail::Id clientId
    );

    void send(
        ::network::Message<UserMessageType>&& message,
        ::detail::Id clientId
    );

    void send(
        const ::network::Message<UserMessageType>& message,
        ::detail::sameAs<::std::shared_ptr<::network::tcp::Connection<UserMessageType>>> auto... clients
    );

    void sendToAllClients(
        const ::network::Message<UserMessageType>& message,
        ::detail::sameAs<::std::shared_ptr<::network::tcp::Connection<UserMessageType>>> auto... ignoredClients
    );

    void sendToAllClients(
        const ::network::Message<UserMessageType>& message,
        ::detail::sameAs<::detail::Id> auto... ignoredClientIds
    );



    // ------------------------------------------------------------------ receive behaviour

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



    // ------------------------------------------------------------------ user behaviors

    // handle the disconnection
    virtual void onDisconnect(
        ::std::shared_ptr<::network::tcp::Connection<UserMessageType>> connection
    ) override;

    // refuses the identification by returning false, username received already
    [[ nodiscard ]] virtual auto onAuthentification(
        ::std::shared_ptr<::network::tcp::Connection<UserMessageType>> connection
    ) -> bool
        override;

    virtual void onConnectionValidated(
        ::std::shared_ptr<::network::tcp::Connection<UserMessageType>> connection
    ) override;



    virtual void onAuthentificationDenial(
        ::std::shared_ptr<::network::tcp::Connection<UserMessageType>> connection
    ) override;




    // ------------------------------------------------------------------ other

    [[ nodiscard ]] auto getConnection(
        ::detail::Id id
    ) -> ::std::shared_ptr<::network::tcp::Connection<UserMessageType>>;

    [[ nodiscard ]] auto getConnectionsSharableInformations() const
        -> ::std::vector<::std::pair<::std::string, ::detail::Id>>;



private:

    // hardware connection to the server
    ::asio::ip::tcp::acceptor m_asioAcceptor;

    ::std::deque<::std::shared_ptr<::network::tcp::Connection<UserMessageType>>> m_incommingConnections;
    ::std::deque<::std::shared_ptr<::network::tcp::Connection<UserMessageType>>> m_connections;
    // ::std::vector<::network::server::Room> m_rooms;

    ::detail::Id m_idCounter{ 1 };

};



} // namespace network::server

#include <Network/Server/AServer.impl.hpp>
