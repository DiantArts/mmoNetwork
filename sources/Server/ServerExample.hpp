#pragma once

#include <Network/Server/AServer.hpp>



class ServerExample
    : public ::network::AServer<::network::MessageType>
{

public:

public:

    // ------------------------------------------------------------------ *structors

    ServerExample(
        const ::std::uint16_t port
    )
        : ::network::AServer<::network::MessageType>{ port }
    {}

    ~ServerExample() = default;



private:

    // ------------------------------------------------------------------ user methods

    // after receiving
    virtual void onTcpReceive(
        ::network::Message<::network::MessageType>& message,
        ::std::shared_ptr<::network::TcpConnection<::network::MessageType>> connection
    ) override
    {
        switch (message.getType()) {
        case ::network::MessageType::startCall: {
            ::detail::Id targetId;
            message >> targetId;
            ::std::cout << "[" << connection->getId() << "] start call with [" << targetId << "].\n";
            message.setType(::network::MessageType::incommingCall);
            message << connection->getAddress() << connection->getId();
            try {
                this->getConnection(targetId)->send(message);
            } catch (...) {
                connection->send(::network::MessageType::invalidTarget);
                ::std::cerr << "[ERROR:Server:TCP:" << connection->getId() << "] no connection at this ID.\n";
            }
            break;
        } case ::network::MessageType::acceptCall: {
            ::detail::Id targetId;
            message >> targetId;
            ::std::cout << "[" << connection->getId() << "] accepted [" << targetId << "]'s call.\n";
            message << connection->getAddress();
            try {
                this->getConnection(targetId)->send(message);
            } catch (...) {
                connection->send(::network::MessageType::invalidTarget);
                ::std::cerr << "[ERROR:Server:TCP:" << connection->getId() << "] no connection at this ID.\n";
            }
            break;
        } case ::network::MessageType::refuseCall: {
            ::detail::Id targetId;
            message >> targetId;
            ::std::cout << "[" << connection->getId() << "] refused [" << targetId << "]'s call.\n";
            try {
                this->getConnection(targetId)->send(message);
            } catch (...) {
                connection->send(::network::MessageType::invalidTarget);
                ::std::cerr << "[ERROR:Server:TCP:" << connection->getId() << "] no connection at this ID.\n";
            }
            break;
        } case ::network::MessageType::messageAll: {
            ::std::cout << "[" << connection->getId() << "] Message All.\n";
            this->sendToAllClients(
                ::network::Message{
                    ::network::MessageType::message,
                    ::network::TransmissionProtocol::tcp,
                    message.extract<::std::string>(),
                    connection->getId()
                },
                connection
            );
            break;
        } default: break;
        }
    }

};
