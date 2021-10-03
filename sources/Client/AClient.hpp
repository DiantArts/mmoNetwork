#pragma once

#include <Id.hpp>
#include <Detail/Concepts.hpp>
#include <Connection.hpp>



namespace network {



template <
    ::network::detail::IsEnum MessageEnumType
> class AClient {

public:

    // ------------------------------------------------------------------ *structors

    AClient() = default;

    virtual ~AClient()
    {
        this->disconnect();
    }



    // ------------------------------------------------------------------ connection

    auto connect(
        const ::std::string& host,
        const ::std::uint16_t port
    ) -> bool
    {
        try {
            m_connection = ::std::make_unique<::network::Connection<MessageEnumType>>(
                ::network::Connection<MessageEnumType>::owner::client,
                m_asioContext,
                ::boost::asio::ip::tcp::socket(m_asioContext),
                m_messagesIn
            );

            if (m_connection->connectToServer(host, port)) {
                ::std::cout << "[CLIENT] Connected to " << host << ":" << port << "\n";
                m_threadContext = ::std::thread([this](){ m_asioContext.run(); });
            } else {
                m_connection->disconnect(); // disconnect if needed
                m_connection.release();
            }

        } catch (::std::exception& e) {
            ::std::cerr << "[Client] Exception: " << e.what() << '\n';
            return false;
        }
        return true;
    }

    void disconnect()
    {
        m_connection->disconnect();

        // stop everything running in parallele
        m_asioContext.stop();
        if(m_threadContext.joinable()) {
            m_threadContext.join();
        }

        // destroy the connection
        m_connection.release();
    }

    auto isConnected()
        -> bool
    {
        return m_connection && m_connection->isConnected();
    }



    // ------------------------------------------------------------------ send

    void send(
        ::network::Message<MessageEnumType>& message
    )
    {
        m_connection->send(message);
    }

    void send(
        ::network::Message<MessageEnumType>&& message
    )
    {
        m_connection->send(::std::forward<decltype(message)>(message)); // TODO: replace that
    }



    // ------------------------------------------------------------------ queue

    auto getIncommingMessages()
        -> ::network::Queue<::network::OwnedMessage<MessageEnumType>>&
    {
        return m_messagesIn;
    }



private:

    // context running on a seperate thread
    ::boost::asio::io_context m_asioContext;
    ::std::thread m_threadContext;

    // hardware connection to the server
    ::std::unique_ptr<::network::Connection<MessageEnumType>> m_connection;

    ::network::Queue<::network::OwnedMessage<MessageEnumType>> m_messagesIn;

};



} // namespace network
