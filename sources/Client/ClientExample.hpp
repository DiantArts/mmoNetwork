#pragma once

#include <Network/AClient.hpp>
#include <MessageType.hpp>



class ClientExample
    : public ::network::AClient<::MessageType>
{

public:

    // ------------------------------------------------------------------ Other

    void messageTcpServer(
        const ::std::string& message
    )
    {
        this->tcpSendToServer(::MessageType::messageAll, message);
    }

    void messageUdpServer(
        const ::std::string& message
    )
    {
        this->udpSendToServer(::MessageType::messageAll, message);
    }

    virtual void onReceive(
        ::network::Message<::MessageType>& message,
        ::std::shared_ptr<::network::Connection<::MessageType>> connection
    ) override
    {
        switch (message.getType()) {
        case ::MessageType::message: {
            ::std::cout << "message from [" << message.pull<::std::string>() << "] "
                << message.pull<::std::string>() << ::std::endl;
            break;
        } default: break;
        }
    }
};
