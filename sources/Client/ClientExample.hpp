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
        ::network::Message<::MessageType> message{ ::MessageType::Ping };
        message << ::std::chrono::system_clock::now();
        this->send(::std::move(message));
    }

    void messageServer()
    {
        ::network::Message<::MessageType> message{ ::MessageType::MessageAll };
        this->send(::std::move(message));
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
                ::network::Id clientId;
                message >> clientId;
                ::std::cout << "message from [" << clientId << "]" << ::std::endl;
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
