#pragma once

#include <Server/AServer.hpp>



enum class MessageType : uint32_t
{
    ConnectionAccepted,
    ConnectionDenied,
    Ping,
    MessageAll,
    Message
};



class ServerExample
    : public ::network::AServer<::MessageType>
{

public:

public:

    // ------------------------------------------------------------------ *structors

    ServerExample(
        const ::std::uint16_t port
    )
        : ::network::AServer<::MessageType>{ port }
    {}

    ~ServerExample() = default;



private:

    // ------------------------------------------------------------------ user methods

    // after receiving
    virtual void onReceive(
        const ::network::Message<::MessageType>& message,
        AServer<::MessageType>::ClientConnectionPtr client
    ) override
    {
        switch (message.getType()) {
        case ::MessageType::MessageAll: {
            ::std::cout << "[" << client->getId() << "]: Message All\n";
            ::network::Message<::MessageType> newMessage{ ::MessageType::Message };
            newMessage << client->getId();
            this->sendToAllClient(newMessage, client);
            break;
        } case ::MessageType::Ping:
        default:
            client->send(message);
            break;
        }
    }

    // before sending
    virtual void onSend(
        const ::network::Message<::MessageType>& message,
        AServer<::MessageType>::ClientConnectionPtr client
    ) override
    {}

};
