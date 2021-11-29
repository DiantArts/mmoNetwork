#pragma once

#include <Network/AServer.hpp>
#include <MessageType.hpp>



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
        ::network::Message<::MessageType>& message,
        ::std::shared_ptr<::network::Connection<::MessageType>> connection
    ) override
    {
        switch (message.getType()) {
        case ::MessageType::messageAll: {
            ::std::cout << "[" << connection->informations.getName() << "] Message All.\n";
            this->tcpSendToAllClients(
                ::network::Message{
                    ::MessageType::message,
                    message.pull<::std::string>(),
                    connection->informations.getName()
                },
                connection
            );
            break;
        } default: break;
        }
    }

};
