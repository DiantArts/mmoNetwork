#pragma once

#include <Network/ANode.hpp>



namespace network {



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



    // ------------------------------------------------------------------ async - in

    void startReceivingConnections();



    // ------------------------------------------------------------------ async - out

    void send(
        ::network::Message<UserMessageType>& message,
        ::std::shared_ptr<::network::TcpConnection<UserMessageType>> client
    );

    void send(
        ::network::Message<UserMessageType>&& message,
        ::std::shared_ptr<::network::TcpConnection<UserMessageType>> client
    );

    void send(
        ::network::Message<UserMessageType>& message,
        ::detail::sameAs<::std::shared_ptr<::network::TcpConnection<UserMessageType>>> auto... clients
    )
    {
        for (auto& client : {clients...}) {
            client->send(message);
        }
    }

    void send(
        ::network::Message<UserMessageType>&& message,
        ::detail::sameAs<::std::shared_ptr<::network::TcpConnection<UserMessageType>>> auto... clients
    )
    {
        for (auto& client : {clients...}) {
            client->send(::std::move(message));
        }
    }

    void sendToAllClients(
        ::network::Message<UserMessageType>& message,
        ::detail::sameAs<::std::shared_ptr<::network::TcpConnection<UserMessageType>>> auto... ignoredClients
    )
    {
        for (auto& client : m_connections) {
            if (((client != ignoredClients) && ...)) {
                client->send(message);
            }
        }
    }

    void sendToAllClients(
        ::network::Message<UserMessageType>&& message,
        ::detail::sameAs<::std::shared_ptr<::network::TcpConnection<UserMessageType>>> auto... ignoredClients
    )
    {
        for (auto& client : m_connections) {
            if (((client != ignoredClients) && ...)) {
                client->send(::std::move(message));
            }
        }
    }



    // ------------------------------------------------------------------ receive behaviour

    virtual auto defaultReceiveBehaviour(
        ::network::Message<UserMessageType>& message,
        ::std::shared_ptr<::network::TcpConnection<UserMessageType>> connection
    ) -> bool
        override final;



    // ------------------------------------------------------------------ user behaviors

    // handle the disconnection
    virtual void onDisconnect(
        ::std::shared_ptr<::network::TcpConnection<UserMessageType>> connection
    ) override;

    // refuses the identification by returning false, username received already
    [[ nodiscard ]] virtual auto onAuthentification(
        ::std::shared_ptr<::network::TcpConnection<UserMessageType>> connection
    ) -> bool
        override;

    virtual void onConnectionValidated(
        ::std::shared_ptr<::network::TcpConnection<UserMessageType>> connection
    ) override;



    virtual void onAuthentificationDenial(
        ::std::shared_ptr<::network::TcpConnection<UserMessageType>> connection
    ) override;




    // ------------------------------------------------------------------ other

    [[ nodiscard ]] auto getConnection(
        ::detail::Id id
    ) -> ::std::shared_ptr<::network::TcpConnection<UserMessageType>>;

    [[ nodiscard ]] auto getConnectionsSharableInformations() const
        -> ::std::vector<::std::pair<::std::string, ::detail::Id>>;



private:

    // hardware connection to the server
    ::asio::ip::tcp::acceptor m_asioAcceptor;

    ::std::deque<::std::shared_ptr<::network::TcpConnection<UserMessageType>>> m_incommingConnections;
    ::std::deque<::std::shared_ptr<::network::TcpConnection<UserMessageType>>> m_connections;

    ::detail::Id m_idCounter{ 1 };

    bool hasClientBeenDisconnected{ false };

};



} // namespace network

#include <Network/Server/AServer.impl.hpp>
