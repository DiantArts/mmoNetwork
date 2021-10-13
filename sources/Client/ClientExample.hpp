#pragma once

#include <Network/Client/AClient.hpp>
#include <Network/MessageType.hpp>
#include <Network/Message.hpp>
#include <Network/OwnedMessage.hpp>



class ClientExample
    : public ::network::AClient<::network::MessageType>
{

public:

    // ------------------------------------------------------------------ Other

    void pingServer()
    {
        this->tcpSend(::network::MessageType::ping);
    }

    void messageServer(
        ::std::string_view message
    )
    {
        this->tcpSend(::network::MessageType::messageAll, message);
    }

    void handleMessagesIn()
    {
        while (!this->getIncommingMessages().empty()) {
            auto message{ this->getIncommingMessages().pop_front() };
            switch (message.getType()) {
            case ::network::MessageType::message: {
                ::std::cout << "message from [" << message.extract<::detail::Id>() << "]: "
                    << message.extract<::std::string>() << ::std::endl;
                break;
            } case ::network::MessageType::ping: {
                // auto timeNow{ ::std::chrono::system_clock::now() };
                // decltype(timeNow) timeThen;
                // message >> timeThen;
                ::std::cout << "Ping received" << ::std::endl;
                break;
            } default: break;
            }
        }
    }

};
