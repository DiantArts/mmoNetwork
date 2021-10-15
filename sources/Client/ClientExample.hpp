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
        this->sendToServer(::network::MessageType::ping);
    }

    void messageServer(
        ::std::string_view message
    )
    {
        this->sendToServer(::network::MessageType::messageAll, message);
    }

    void messagePeer(
        ::std::string_view message
    )
    {
        this->sendToPeer(::network::MessageType::message, message, m_connectionToServer->getId());
    }

    void startCall(
        ::detail::Id target
    )
    {
        if (this->isConnectedToPeer()) {
            ::std::cerr << "[ERROR:Client:TCP:" << m_connectionToServer->getId() << "] Already in call.\n";
        } else if (target == m_connectionToServer->getId()) {
            ::std::cerr << "[ERROR:Client:TCP:" << m_connectionToServer->getId() << "] Cannot call yourself\n";
        } else {
            this->openUdpConnection();
            this->sendToServer(::network::MessageType::startCall, this->getUdpPort(), target);
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
            this->sendToServer(::network::MessageType::acceptCall, this->getUdpPort(), ::std::move(callerId));
        } else {
            this->sendToServer(::network::MessageType::refuseCall, callerId);
            ::std::cout << "call refused\n";
        }
    }

    void handleMessagesIn()
    {
        while (!this->getIncommingMessages().empty()) {
            auto message{ this->getIncommingMessages().pop_front() };
            switch (message.getType()) {
            case ::network::MessageType::incommingCall: {
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
            } case ::network::MessageType::acceptCall: {
                ::std::string udpAddr;
                ::std::uint16_t udpPort;
                message >> udpAddr >> udpPort;
                this->targetPeer(udpAddr, udpPort);
                ::std::cout << "call accepted: "<< udpAddr << ":" << udpPort << ".\n";
                break;
            } case ::network::MessageType::refuseCall: {
                this->closePeerConnection();
                ::std::cout << "call refused.\n";
                break;
            } case ::network::MessageType::message: {
                ::std::cout << "message from ["
                    << (message.getTransmissionProtocol() == ::network::TransmissionProtocol::tcp ? "TCP" : "UDP")
                    << ":" << message.extract<::detail::Id>() << "] "
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
