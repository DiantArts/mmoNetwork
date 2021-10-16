#pragma once

#include <Network/ANode.hpp>



namespace network {



template <
    ::detail::isEnum MessageType
> class AServer
    : public ::network::ANode<MessageType>
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

    void pullIncommingMessage();

    void pullIncommingMessages();

    void blockingPullIncommingMessages();



    // ------------------------------------------------------------------ async - out

    void send(
        ::network::Message<MessageType>& message,
        ::std::shared_ptr<::network::TcpConnection<MessageType>> client
    );

    void send(
        ::network::Message<MessageType>&& message,
        ::std::shared_ptr<::network::TcpConnection<MessageType>> client
    );

    void send(
        ::network::Message<MessageType>& message,
        ::detail::sameAs<::std::shared_ptr<::network::TcpConnection<MessageType>>> auto... clients
    )
    {
        for (auto& client : {clients...}) {
            client->send(message);
        }
    }

    void send(
        ::network::Message<MessageType>&& message,
        ::detail::sameAs<::std::shared_ptr<::network::TcpConnection<MessageType>>> auto... clients
    )
    {
        for (auto& client : {clients...}) {
            client->send(::std::move(message));
        }
    }

    void sendToAllClients(
        ::network::Message<MessageType>& message,
        ::detail::sameAs<::std::shared_ptr<::network::TcpConnection<MessageType>>> auto... ignoredClients
    )
    {
        for (auto& client : m_connections) {
            if (((client != ignoredClients) && ...)) {
                client->send(message);
            }
        }
    }

    void sendToAllClients(
        ::network::Message<MessageType>&& message,
        ::detail::sameAs<::std::shared_ptr<::network::TcpConnection<MessageType>>> auto... ignoredClients
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
        ::network::Message<MessageType>& message,
        ::std::shared_ptr<::network::TcpConnection<MessageType>> connection
    ) -> bool
        override final;



    // ------------------------------------------------------------------ user methods

    // handle the disconnection
    virtual void onDisconnect(
        ::std::shared_ptr<::network::TcpConnection<MessageType>> connection
    );

    // refuses the connection by returning false
    virtual auto onClientConnect(
        ::std::shared_ptr<::network::TcpConnection<MessageType>> connection
    ) -> bool;

    // refuses the identification by returning false
    [[ nodiscard ]] virtual auto onClientIdentificate(
        ::std::shared_ptr<::network::TcpConnection<MessageType>> connection
    ) -> bool;



    // ------------------------------------------------------------------ other

    // needs inlinment for linkage pupruses
    inline void validateConnection(
        ::std::shared_ptr<::network::TcpConnection<MessageType>> connection
    )
    {
        m_incommingConnections.erase(::std::ranges::find(m_incommingConnections, connection));
        m_connections.push_back(::std::move(connection));
    }

    [[ nodiscard ]] auto getConnection(
        ::detail::Id id
    ) -> ::std::shared_ptr<::network::TcpConnection<MessageType>>;



private:

    // hardware connection to the server
    ::asio::ip::tcp::acceptor m_asioAcceptor;

    ::std::deque<::std::shared_ptr<::network::TcpConnection<MessageType>>> m_incommingConnections;
    ::std::deque<::std::shared_ptr<::network::TcpConnection<MessageType>>> m_connections;

    ::detail::Id m_idCounter{ 1 };

    bool hasClientBeenDisconnected{ false };

};



} // namespace network
