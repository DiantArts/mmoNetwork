#pragma once

#include <Network/Server/AServer.hpp>
#include <Network/Message.hpp>
#include <Network/OwnedMessage.hpp>



class ServerExample
    : public ::network::AServer<::network::MessageType>
{

public:

public:

    // ------------------------------------------------------------------ *structors

    ServerExample(
        const ::std::uint16_t port
    )
        : ::network::AServer<::network::MessageType>{ port }
    {}

    ~ServerExample() = default;



private:

    // ------------------------------------------------------------------ user methods

    // refuses the connection by returning false
    virtual auto onClientConnect(
        ::std::shared_ptr<::network::Connection<::network::MessageType>> connection
    ) -> bool
        override
    {
        return true;
    }

    // after receiving
    virtual void onTcpReceive(
        ::network::Message<::network::MessageType>& message,
        ::std::shared_ptr<::network::Connection<::network::MessageType>> connection
    ) override
    {
        switch (message.getType()) {
        case ::network::MessageType::messageAll: {
            ::std::cout << "[" << connection->getId() << "]: Message All\n";
            this->sendToAllClients(
                ::network::Message{
                    ::network::MessageType::message,
                    ::network::TransmissionProtocol::tcp,
                    connection->getId()
                },
                connection
            );
            break;
        } default: break;
        }
    }

};
