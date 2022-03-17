#pragma once

#include <Network/AClient.hpp>
#include <MessageType.hpp>



class ClientExample
    : public ::network::AClient<::MessageType>
{

public:

    // ------------------------------------------------------------------ Other

    void commandHelp()
    {
        ::std::cout << "h: help\n";
        ::std::cout << "q: quit\n";
        ::std::cout << "u: message using UDP instead of TCP\n";
        ::std::cout << "n: rename\n";
        ::std::cout << "c: display connected clients" << ::std::endl;
    }

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

    void rename(
        const ::std::string& name
    )
    {
        this->setInformation<::network::SharableInformations::Index::name>(name);
    }

    void displayConnectedClients()
    {
        ::std::cout << m_connectedClientsInformations.size() << " clients connected :\n";
        for (const auto& [clientId, clientInformations] : m_connectedClientsInformations) {
            ::std::cout << '[' << clientId << "] " << clientInformations.name << ::std::endl;
        }
        ::std::cout << ::std::flush;
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
