#pragma once

#include <Server/AServer.hpp>
#include <Message.hpp>
#include <OwnedMessage.hpp>



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

    virtual void onDisconnect(
        ::std::shared_ptr<::network::Connection<::network::MessageType>> connection
    ) override
    {}

    // after receiving
    virtual void onReceive(
        const ::network::Message<::network::MessageType>& message,
        ::std::shared_ptr<::network::Connection<::network::MessageType>> connection
    ) override
    {
        switch (message.getType()) {
        case ::network::MessageType::MessageAll: {
            ::std::cout << "[" << connection->getId() << "]: Message All\n";
            ::network::Message<::network::MessageType> newMessage{ ::network::MessageType::Message };
            newMessage << connection->getId();
            this->sendToAllClient(newMessage, connection);
            break;
        } case ::network::MessageType::Ping:
        default:
            connection->send(message);
            break;
        }
    }

    // before sending
    virtual void onSend(
        const ::network::Message<::network::MessageType>& message,
        ::std::shared_ptr<::network::Connection<::network::MessageType>> connection
    ) override
    {}

};
