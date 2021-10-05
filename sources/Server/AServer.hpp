#pragma once

#include <Id.hpp>
#include <Detail/Concepts.hpp>
#include <Connection.hpp>
#include <MessageType.hpp>
#include <ANode.hpp>



namespace network {



template <
    ::network::detail::IsEnum MessageType
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
        const ::network::Message<MessageType>& message,
        ::std::shared_ptr<::network::Connection<MessageType>> client
    );

    void send(
        const ::network::Message<MessageType>& message,
        ::std::same_as<::std::shared_ptr<::network::Connection<MessageType>>> auto... clients
    )
    {
        auto invalidClientDetected{ false };

        for (auto& client : {clients...}) {
            if (client->isConnected()) {
                this->onSend(message, client);
                client->send(message);
            } else {
                this->onDisconnect(client);
                client.reset();
                invalidClientDetected = true;
            }
        }

        if (invalidClientDetected) {
            m_connections.resize(m_connections.size() - ::std::ranges::remove(m_connections, nullptr).size());
        }
    }

    void sendToAllClient(
        const ::network::Message<MessageType>& message,
        ::std::same_as<::std::shared_ptr<::network::Connection<MessageType>>> auto... ignoredClients
    )
    {
        auto invalidClientDetected{ false };

        for (auto& client : m_connections) {
            if (client->isConnected()) {
                if (((client != ignoredClients) && ...)) {
                    this->onSend(message, client);
                    client->send(message);
                }
            } else {
                this->onDisconnect(client);
                client.reset();
                invalidClientDetected = true;
            }
        }

        if (invalidClientDetected) {
            m_connections.resize(m_connections.size() - ::std::ranges::remove(m_connections, nullptr).size());
        }
    }



    // ------------------------------------------------------------------ user methods

    // refuses the connection by returning false
    virtual auto onClientConnect(
        ::std::shared_ptr<::network::Connection<MessageType>> connection
    ) -> bool;

    // refuses the identification by returning false
    [[ nodiscard ]] virtual auto onClientIdentificate(
        ::std::shared_ptr<::network::Connection<MessageType>> connection
    ) -> bool;




private:

    // hardware connection to the server
    ::boost::asio::ip::tcp::acceptor m_asioAcceptor;


    ::std::deque<::std::shared_ptr<::network::Connection<MessageType>>> m_connections;

    ::network::Id m_idCounter{ 1 };

};



} // namespace network
