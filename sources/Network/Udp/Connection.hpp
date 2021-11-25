#pragma once

#include <Detail/Concepts.hpp>
#include <Detail/Queue.hpp>
#include <Network/Message.hpp>
#include <Network/OwnedMessage.hpp>

#ifdef ENABLE_ENCRYPTION
#include <Security/Cipher.hpp>
#endif

namespace network { template <::detail::isEnum UserMessageType> class ANode; }
namespace network { template <::detail::isEnum UserMessageType> class Connection; }



namespace network::udp {



template <
    ::detail::isEnum UserMessageType
> class Connection {

public:

    // ------------------------------------------------------------------ *structors

    Connection(
        ::asio::io_context& asioContext
    );

    // doesnt call onDisconnect
    ~Connection();



    // ------------------------------------------------------------------ async - connection

    void target(
        const ::std::string& host,
        const ::std::uint16_t port
    );

    void close();

    [[ nodiscard ]] auto isOpen() const
        -> bool;



    // ------------------------------------------------------------------ async - out

    void send(
        UserMessageType messageType,
        auto&&... args
    );

    void send(
        ::network::Message<UserMessageType> message
    );



    // ------------------------------------------------------------------ other

    [[ nodiscard ]] auto getPort() const
        -> ::std::uint16_t;

    [[ nodiscard ]] auto getAddress() const
        -> ::std::string;

    void assignConnection(
        ::std::shared_ptr<::network::Connection<UserMessageType>> connection
    );


private:

    // ------------------------------------------------------------------ async - out

    void writeHeader(
        ::std::size_t bytesAlreadySent = 0
    );

    void writeBody(
        ::std::size_t bytesAlreadySent = 0
    );



    // ------------------------------------------------------------------ async - in

    void readHeader(
        ::std::size_t bytesAlreadyRead = 0
    );

    void readBody(
        ::std::size_t bytesAlreadyRead = 0
    );

    void transferBufferToInQueue();



private:

    ::std::shared_ptr<::network::Connection<UserMessageType>> m_connection;

    ::asio::ip::udp::socket m_socket;
    ::network::Message<UserMessageType> m_bufferIn;
    ::detail::Queue<::network::Message<UserMessageType>> m_messagesOut;

};



} // namespace network::udp

#include <Network/Udp/Connection.impl.hpp>
