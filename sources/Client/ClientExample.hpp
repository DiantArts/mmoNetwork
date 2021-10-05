#pragma once

#include <Client/AClient.hpp>
#include <MessageType.hpp>
#include <Message.hpp>
#include <OwnedMessage.hpp>



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
                ::std::cout << "message from [" << message.extract<::network::Id>() << "]" << ::std::endl;
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
