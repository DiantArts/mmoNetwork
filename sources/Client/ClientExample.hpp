#pragma once

#include <Network/AClient.hpp>
#include <MessageType.hpp>



class ClientExample
    : public ::network::AClient<::MessageType>
{

public:

    // ------------------------------------------------------------------ Other

    void messageServer(
        const ::std::string& message
    )
    {
        this->sendToServer(::MessageType::messageAll, message);
    }

    void messagePeer(
        const ::std::string& message
    )
    {
        this->sendToPeer(::MessageType::message, message, m_tcpConnectionToServer->getUserName());
    }

    void startCall(
        ::detail::Id target
    )
    {
        if (this->isConnectedToPeer()) {
            ::std::cerr << "[ERROR:Client:TCP:" << m_tcpConnectionToServer->getId() << "] Already in call.\n";
        } else if (target == m_tcpConnectionToServer->getId()) {
            ::std::cerr << "[ERROR:Client:TCP:" << m_tcpConnectionToServer->getId() << "] Cannot call yourself\n";
        } else {
            this->openUdpConnection();
            this->sendToServer(::MessageType::startCall, this->getUdpPort(), target);
            ::std::cout << "calling [" << target << "].\n";
        }
    }

    void answerCall(
        ::detail::Id callerId,
        ::std::string udpAddr,
        ::std::uint16_t udpPort
    )
    {
        ::std::cout << "Accept call? [y/N]" << ::std::endl;
        ::std::cout << "y" << ::std::endl;
        ::std::string str{ "y" };
        if (str == "y") {
            this->openUdpConnection();
            this->targetPeer(udpAddr, udpPort);
            this->sendToServer(::MessageType::acceptCall, this->getUdpPort(), ::std::move(callerId));
        } else {
            this->sendToServer(::MessageType::refuseCall, callerId);
            ::std::cout << "call refused\n";
        }
    }

    void stopCall()
    {
        this->closePeerConnection();
    }

    virtual void onReceive(
        ::network::Message<::MessageType>& message,
        ::std::shared_ptr<::network::tcp::Connection<::MessageType>> connection
    ) override
    {
        switch (message.getType()) {
        case ::MessageType::incommingCall: {
            ::detail::Id callerId;
            ::std::string udpAddr;
            ::std::uint16_t udpPort;
            message >> callerId >> udpAddr >> udpPort;
            ::std::cout << "Incomming call from [" << callerId << "] ("
                << udpAddr << ":" << udpPort << ").\n";
            if (!this->isConnectedToPeer()) {
                this->answerCall(callerId, udpAddr, udpPort);
            } else {
                ::std::cout << "call refused, a call is already ongoing.\n";
            }
            break;
        } case ::MessageType::acceptCall: {
            ::std::string udpAddr;
            ::std::uint16_t udpPort;
            message >> udpAddr >> udpPort;
            this->targetPeer(udpAddr, udpPort);
            ::std::cout << "call accepted: "<< udpAddr << ":" << udpPort << ".\n";
            break;
        } case ::MessageType::refuseCall: {
            this->closePeerConnection();
            ::std::cout << "call refused.\n";
            break;
        } case ::MessageType::message: {
            ::std::cout << "message from [Tcp:"
                << "" << message.pull<::std::string>() << "] "
                << message.pull<::std::string>() << ::std::endl;
            break;
        } default: break;
        }
    }

    virtual void onReceive(
        ::network::Message<::MessageType>& message,
        ::std::shared_ptr<::network::udp::Connection<::MessageType>> connection
    ) override
    {
        switch (message.getType()) {
        case ::MessageType::message:
            ::std::cout << "message from [Udp:" << "" << message.pull<::std::string>() << "] "
                << message.pull<::std::string>() << ::std::endl;
            break;
        default: break;
        }
    }

};
