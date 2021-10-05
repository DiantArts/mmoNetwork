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
        this->send(::network::MessageType::Ping, ::std::chrono::system_clock::now());
    }

    void messageServer()
    {
        this->send(::network::MessageType::MessageAll);
    }

    void handleMessagesIn()
    {
        while (!this->getIncommingMessages().empty()) {
            auto message{ this->getIncommingMessages().pop_front() };
            switch (message.getType()) {
            case ::network::MessageType::Message: {
                ::std::cout << "message from [" << message.extract<::detail::Id>() << "]" << ::std::endl;
                break;
            } case ::network::MessageType::Ping: {
                auto timeNow{ ::std::chrono::system_clock::now() };
                decltype(timeNow) timeThen;
                message >> timeThen;
                ::std::cout << "Ping: " << ::std::chrono::duration<double>(timeNow - timeThen).count() << ::std::endl;
                break;
            } default: break;
            }
        }
    }

};
