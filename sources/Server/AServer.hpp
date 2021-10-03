#pragma once

#include <Id.hpp>
#include <Detail/Concepts.hpp>
#include <Connection.hpp>



namespace network {



template <
    ::network::detail::IsEnum MessageEnumType
> class AServer {

public:

    using ClientConnectionPtr = ::std::shared_ptr<::network::Connection<MessageEnumType>>;


public:

    // ------------------------------------------------------------------ *structors

    AServer(
        const ::std::uint16_t port
    )
        : m_asioAcceptor{
            m_asioContext,
            ::boost::asio::ip::tcp::endpoint{ ::boost::asio::ip::tcp::v4(), port }
        }
    {
        ::std::cout << "[SERVER] Ready to listen port " << port << ".\n";
    }

    virtual ~AServer()
    {
        this->stop();
    }



    // ------------------------------------------------------------------ running

    auto start()
        -> bool
    {
        try {
            this->startReceivingConnections();

            // start the routine
            m_threadContext = ::std::thread([this](){ m_asioContext.run(); });

        } catch (::std::exception& e) {
            ::std::cerr << "[SERVER] Exception: " << e.what() << ".\n";
            return false;
        }
        ::std::cout << "[SERVER] Started" << '\n';
        return true;
    }

    void stop()
    {
        // stop everything running in parallele
        m_asioContext.stop();
        if(m_threadContext.joinable()) {
            m_threadContext.join();
        }

        ::std::cout << "[SERVER] Stopped" << '\n';
    }

    auto isRunning()
        -> bool
    {
        // TODO: isRunning implemetation
        return true;
    }



    // ------------------------------------------------------------------ in - async

    void startReceivingConnections()
    {
        m_asioAcceptor.async_accept(
            [this](
                const boost::system::error_code& errorCode,
                ::boost::asio::ip::tcp::socket socket
            ) {
                if (!errorCode) {
                    ::std::cout << "[SERVER] New incomming connection: " << socket.remote_endpoint() << ".\n";
                    auto newConnection{ ::std::make_shared<::network::Connection<MessageEnumType>>(
                        ::network::Connection<MessageEnumType>::owner::server,
                        m_asioContext,
                        ::std::move(socket),
                        m_messagesIn
                    ) };
                    if (this->onClientConnect(newConnection)) {
                        newConnection->connectToClient(++m_idCounter);
                        m_connections.push_back(::std::move(newConnection));
                        ::std::cout << "[SERVER] Connection ["
                            << m_connections.back()->getId() << "] approved.\n";
                    } else {
                        ::std::clog << "[SERVER] Connection denied.\n";
                    }
                } else {
                    ::std::clog << "[SERVER] New connection error: " << errorCode.message() << ".\n";
                }
                this->startReceivingConnections();
            }
        );
    }

    void update()
    {
        while (!m_messagesIn.empty()) {
            auto message{ m_messagesIn.pop_front() };
            this->onReceive(message, message.getRemote());
        }
    }

    void update(
        ::std::size_t numberOfMessages
    )
    {
        for (::std::size_t i{ 0 }; i < numberOfMessages && !m_messagesIn.empty(); ++i) {
            auto message{ m_messagesIn.pop_front() };
            this->onReceive(message);
        }
    }



    // ------------------------------------------------------------------ out - async

    void send(
        const ::network::Message<MessageEnumType>& message,
        AServer<MessageEnumType>::ClientConnectionPtr client
    )
    {
        if (client->isConnected()) {
            this->onSend(message, client);
            client->send(message);
        } else {
            this->onClientDisconnect(client);
            client.reset();
            m_connections.resize(m_connections.size() - ::std::ranges::remove(m_connections, nullptr).size());
        }
    }

    void send(
        const ::network::Message<MessageEnumType>& message,
        ::std::same_as<AServer<MessageEnumType>::ClientConnectionPtr> auto... clients
    )
    {
        auto invalidClientDetected{ false };

        for (auto& client : {clients...}) {
            if (client->isConnected()) {
                this->onSend(message, client);
                client->send(message);
            } else {
                this->onClientDisconnect(client);
                client.reset();
                invalidClientDetected = true;
            }
        }

        if (invalidClientDetected) {
            m_connections.resize(m_connections.size() - ::std::ranges::remove(m_connections, nullptr).size());
        }
    }

    void sendToAllClient(
        const ::network::Message<MessageEnumType>& message,
        ::std::same_as<AServer<MessageEnumType>::ClientConnectionPtr> auto... ignoredClients
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
                this->onClientDisconnect(client);
                client.reset();
                invalidClientDetected = true;
            }
        }

        if (invalidClientDetected) {
            m_connections.resize(m_connections.size() - ::std::ranges::remove(m_connections, nullptr).size());
        }
    }



protected:

    // ------------------------------------------------------------------ user methods

    // refuses the connection by returning false
    virtual auto onClientConnect(
        AServer<MessageEnumType>::ClientConnectionPtr client
    ) -> bool
    {
        ::network::Message<MessageEnumType> message{ MessageEnumType::ConnectionAccepted };
        client->send(message);
        return true;
    }

    virtual void onClientDisconnect(
        AServer<MessageEnumType>::ClientConnectionPtr client
    )
    {
        ::std::cout << "[SERVER:" << client->getId() << "] Disconnected, client removed.\n";
    }

    // after receiving
    virtual void onReceive(
        const ::network::Message<MessageEnumType>& message,
        AServer<MessageEnumType>::ClientConnectionPtr client
    )
    {}

    // before sending
    virtual void onSend(
        const ::network::Message<MessageEnumType>& message,
        AServer<MessageEnumType>::ClientConnectionPtr client
    )
    {}



protected:

    // context running on a seperate thread
    ::boost::asio::io_context m_asioContext;
    ::std::thread m_threadContext;

    // hardware connection to the server
    ::boost::asio::ip::tcp::acceptor m_asioAcceptor;

    ::network::Queue<::network::OwnedMessage<MessageEnumType>> m_messagesIn;

    ::std::deque<AServer<MessageEnumType>::ClientConnectionPtr> m_connections;

    ::network::Id m_idCounter{ 1 };

};



} // namespace network
