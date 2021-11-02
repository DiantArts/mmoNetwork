#pragma once

#include <Network/Server/AServer.hpp>
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
    virtual void onTcpReceive(
        ::network::Message<::MessageType>& message,
        ::std::shared_ptr<::network::TcpConnection<::MessageType>> connection
    ) override
    {
        switch (message.getType()) {
        case ::MessageType::startCall: {
            ::detail::Id targetId;
            message >> targetId;
            ::std::cout << "[" << connection->getUserName() << "] start call with [" << targetId << "].\n";
            message.setType(::MessageType::incommingCall);
            message << connection->getAddress() << connection->getId();
            try {
                this->getConnection(targetId)->send(message);
            } catch (...) {
                connection->send(::MessageType::invalidTarget);
                ::std::cerr << "[ERROR:Server:TCP:" << connection->getUserName() << "] no connection at this ID.\n";
            }
            break;
        } case ::MessageType::acceptCall: {
            ::detail::Id targetId;
            message >> targetId;
            ::std::cout << "[" << connection->getUserName() << "] accepted [" << targetId << "]'s call.\n";
            message << connection->getAddress();
            try {
                this->getConnection(targetId)->send(message);
            } catch (...) {
                connection->send(::MessageType::invalidTarget);
                ::std::cerr << "[ERROR:Server:TCP:" << connection->getUserName() << "] no connection at this ID.\n";
            }
            break;
        } case ::MessageType::refuseCall: {
            ::detail::Id targetId;
            message >> targetId;
            ::std::cout << "[" << connection->getUserName() << "] refused [" << targetId << "]'s call.\n";
            try {
                this->getConnection(targetId)->send(message);
            } catch (...) {
                connection->send(::MessageType::invalidTarget);
                ::std::cerr << "[ERROR:Server:TCP:" << connection->getUserName() << "] no connection at this ID.\n";
            }
            break;
        } case ::MessageType::messageAll: {
            ::std::cout << "[" << connection->getUserName() << "] Message All.\n";
            this->sendToAllClients(
                ::network::Message{
                    ::MessageType::message,
                    ::network::TransmissionProtocol::tcp,
                    message.extract<::std::string>(),
                    connection->getUserName()
                },
                connection
            );
            break;
        } default: break;
        }
    }

};
