#pragma once

#include <Network/AConnection.hpp>



namespace network::udp {



template <
    ::detail::isEnum UserMessageType
> class Connection
    : public ::network::AConnection<UserMessageType>
    , public ::std::enable_shared_from_this<Connection<UserMessageType>>
{

public:

    // ------------------------------------------------------------------ *structors

    Connection(
        ::network::ANode<UserMessageType>& owner
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

    [[ nodiscard ]] auto getOwner() const
        -> const ::network::ANode<UserMessageType>&;

    [[ nodiscard ]] auto getPort() const
        -> ::std::uint16_t;

    [[ nodiscard ]] auto getAddress() const
        -> ::std::string;



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

    ::asio::ip::udp::socket m_socket;

    using ::network::AConnection<UserMessageType>::m_owner;
    using ::network::AConnection<UserMessageType>::m_bufferIn;
    using ::network::AConnection<UserMessageType>::m_messagesOut;
#ifdef ENABLE_ENCRYPTION
    using ::network::AConnection<UserMessageType>::m_cipher;
#endif // ENABLE_ENCRYPTION

};



} // namespace network::udp

#include <Network/Udp/Connection.impl.hpp>
