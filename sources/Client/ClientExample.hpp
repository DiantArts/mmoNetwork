#pragma once

#include <Client/AClient.hpp>



enum class MessageType : uint32_t
{
    ConnectionAccepted,
    ConnectionDenied,
    Ping,
    MessageAll,
    Message
};



class ClientExample
    : public ::network::AClient<::MessageType>
{
public:
    void pingServer()
    {
        this->send(::MessageType::Ping, ::std::chrono::system_clock::now());
    }

    void messageServer()
    {
        this->send(::MessageType::MessageAll);
    }

    void handleMessagesIn()
    {
        while (!this->getIncommingMessages().empty()) {
            auto message{ this->getIncommingMessages().pop_front() };
            switch (message.getType()) {
            case ::MessageType::ConnectionAccepted: {
                ::std::cout << "[CLIENT] Authentification accepted" << ::std::endl;
                break;
            } case ::MessageType::Message: {
                ::std::cout << "message from [" << message.extract<::network::Id>() << "]" << ::std::endl;
                break;
            } case ::MessageType::Ping: {
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
