#pragma once

#include <Detail/Id.hpp>
#include <Detail/Concepts.hpp>
#include <Network/Connection.hpp>
#include <Network/MessageType.hpp>
#include <Network/ANode.hpp>



namespace network {



template <
    ::detail::IsEnum MessageType
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



    // ------------------------------------------------------------------ async - autoProtocol

    void send(
        ::network::Message<MessageType>&& message,
        ::std::shared_ptr<::network::Connection<MessageType>>&& client
    );

    void send(
        ::network::Message<MessageType>&& message,
        ::detail::same_as<::std::shared_ptr<::network::Connection<MessageType>>> auto&&... clients
    )
    {
        if (message.getTransmissionProtocol() == ::network::TransmissionProtocol::tcp) {
            this->tcpSend(
                ::std::forward<decltype(message)>(message),
                ::std::forward<decltype(clients)>(clients)...
            );
        } else {
            this->udpSend(
                message,
                ::std::forward<decltype(message)>(message),
                ::std::forward<decltype(clients)>(clients)...
            );
        }
    }

    void sendToAllClients(
        ::network::Message<MessageType>&& message,
        ::detail::same_as<::std::shared_ptr<::network::Connection<MessageType>>> auto&&... ignoredClients
    )
    {
        if (message.getTransmissionProtocol() == ::network::TransmissionProtocol::tcp) {
            this->tcpSendToAllClients(
                ::std::forward<decltype(message)>(message),
                ::std::forward<decltype(ignoredClients)>(ignoredClients)...
            );
        } else {
            this->udpSendToAllClients(
                ::std::forward<decltype(message)>(message),
                ::std::forward<decltype(ignoredClients)>(ignoredClients)...
            );
        }
    }



    // ------------------------------------------------------------------ async - tcpOut

    void tcpSend(
        ::network::Message<MessageType>& message,
        ::std::shared_ptr<::network::Connection<MessageType>> client
    );

    void tcpSend(
        ::network::Message<MessageType>&& message,
        ::std::shared_ptr<::network::Connection<MessageType>> client
    );

    void tcpSend(
        ::network::Message<MessageType>& message,
        ::detail::same_as<::std::shared_ptr<::network::Connection<MessageType>>> auto... clients
    )
    {
        for (auto& client : {clients...}) {
            this->onTcpSend(message, client);
            client->tcpSend(message);
        }
    }

    void tcpSend(
        ::network::Message<MessageType>&& message,
        ::detail::same_as<::std::shared_ptr<::network::Connection<MessageType>>> auto... clients
    )
    {
        for (auto& client : {clients...}) {
            this->onTcpSend(message, client);
            client->tcpSend(::std::move(message));
        }
    }

    void tcpSendToAllClients(
        ::network::Message<MessageType>& message,
        ::detail::same_as<::std::shared_ptr<::network::Connection<MessageType>>> auto... ignoredClients
    )
    {
        for (auto& client : m_connections) {
            if (((client != ignoredClients) && ...)) {
                this->onTcpSend(message, client);
                client->tcpSend(message);
            }
        }
    }

    void tcpSendToAllClients(
        ::network::Message<MessageType>&& message,
        ::detail::same_as<::std::shared_ptr<::network::Connection<MessageType>>> auto... ignoredClients
    )
    {
        for (auto& client : m_connections) {
            if (((client != ignoredClients) && ...)) {
                this->onTcpSend(message, client);
                client->tcpSend(::std::move(message));
            }
        }
    }



    // ------------------------------------------------------------------ async - udpOut

    void udpSend(
        ::network::Message<MessageType>& message,
        ::std::shared_ptr<::network::Connection<MessageType>> client
    );

    void udpSend(
        ::network::Message<MessageType>&& message,
        ::std::shared_ptr<::network::Connection<MessageType>> client
    );

    void udpSend(
        ::network::Message<MessageType>& message,
        ::detail::same_as<::std::shared_ptr<::network::Connection<MessageType>>> auto... clients
    )
    {
        for (auto& client : {clients...}) {
            this->onUdpSend(message, client);
            client->udpSend(message);
        }
    }

    void udpSend(
        ::network::Message<MessageType>&& message,
        ::detail::same_as<::std::shared_ptr<::network::Connection<MessageType>>> auto... clients
    )
    {
        for (auto& client : {clients...}) {
            this->onUdpSend(message, client);
            client->udpSend(::std::move(message));
        }
    }

    void udpSendToAllClients(
        ::network::Message<MessageType>& message,
        ::detail::same_as<::std::shared_ptr<::network::Connection<MessageType>>> auto... ignoredClients
    )
    {
        for (auto& client : m_connections) {
            if (((client != ignoredClients) && ...)) {
                this->onUdpSend(message, client);
                client->udpSend(message);
            }
        }
    }

    void udpSendToAllClients(
        ::network::Message<MessageType>&& message,
        ::detail::same_as<::std::shared_ptr<::network::Connection<MessageType>>> auto... ignoredClients
    )
    {
        for (auto& client : m_connections) {
            if (((client != ignoredClients) && ...)) {
                this->onUdpSend(message, client);
                client->udpSend(::std::move(message));
            }
        }
    }



    // ------------------------------------------------------------------ receive behaviour

    virtual auto defaultReceiveBehaviour(
        ::network::Message<MessageType>& message,
        ::std::shared_ptr<::network::Connection<MessageType>> connection
    ) -> bool
        override final;



    // ------------------------------------------------------------------ user methods

    // handle the disconnection
    virtual void onDisconnect(
        ::std::shared_ptr<::network::Connection<MessageType>> connection
    );

    // refuses the connection by returning false
    virtual auto onClientConnect(
        ::std::shared_ptr<::network::Connection<MessageType>> connection
    ) -> bool;

    // refuses the identification by returning false
    [[ nodiscard ]] virtual auto onClientIdentificate(
        ::std::shared_ptr<::network::Connection<MessageType>> connection
    ) -> bool;



    // ------------------------------------------------------------------ other

    // needs inlinment for linkage pupruses
    inline void validateConnection(
        ::std::shared_ptr<::network::Connection<MessageType>> connection
    )
    {
        ::std::cout << connection.use_count() << ::std::endl;
        m_incommingConnections.erase(::std::ranges::find(m_incommingConnections, connection));
        ::std::cout << connection.use_count() << ::std::endl;
        m_connections.push_back(::std::move(connection));
    }



private:

    // hardware connection to the server
    ::boost::asio::ip::tcp::acceptor m_asioAcceptor;

    ::std::deque<::std::shared_ptr<::network::Connection<MessageType>>> m_incommingConnections;
    ::std::deque<::std::shared_ptr<::network::Connection<MessageType>>> m_connections;

    ::detail::Id m_idCounter{ 1 };

    bool hasClientBeenDisconnected{ false };

};



} // namespace network
