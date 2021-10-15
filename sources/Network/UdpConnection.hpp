#pragma once

#include <Detail/Queue.hpp>
#include <Detail/Id.hpp>
#include <Detail/Concepts.hpp>
#include <Network/Message.hpp>
#include <Network/OwnedMessage.hpp>

namespace network { template <::detail::isEnum MessageType> class AClient; }



namespace network {



template <
    ::detail::isEnum MessageType
> class UdpConnection
    : public ::std::enable_shared_from_this<UdpConnection<MessageType>>
{

public:

    // ------------------------------------------------------------------ *structors

    UdpConnection(
        ::network::AClient<MessageType>& owner
    );

    // doesnt call onDisconnect
    ~UdpConnection();



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
        MessageType messageType,
        auto&&... args
    )
    {
        ::network::Message message{
            ::std::forward<decltype(messageType)>(messageType),
            ::network::TransmissionProtocol::udp,
            ::std::forward<decltype(args)>(args)...
        };
        ::boost::asio::post(
            m_owner.getAsioContext(),
            [this, message]()
            {
                auto wasOutQueueEmpty{ m_messagesOut.empty() };
                m_messagesOut.push_back(::std::move(message));
                if (wasOutQueueEmpty) {
                    this->writeHeader();
                }
            }
        );
    }

    void send(
        ::network::Message<MessageType>&& message
    );



    // ------------------------------------------------------------------ other

    [[ nodiscard ]] auto getOwner() const
        -> const ::network::AClient<MessageType>&;

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

    ::network::AClient<MessageType>& m_owner;

    ::boost::asio::ip::udp::socket m_socket;
    ::network::Message<MessageType> m_bufferIn;
    ::detail::Queue<::network::Message<MessageType>> m_messagesOut;

};



} // namespace network
