#pragma once

#include <Network/AServer.hpp>
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
    virtual void onReceive(
        ::network::Message<::MessageType>& message,
        ::std::shared_ptr<::network::tcp::Connection<::MessageType>> connection
    ) override
    {
        switch (message.getType()) {
        case ::MessageType::startCall: {
            ::detail::Id targetId;
            message.extract(targetId);
            ::std::cout << "[" << connection->getUserName() << "] start call with [" << targetId << "].\n";
            message.setType(::MessageType::incommingCall);
            message.insert(connection->getAddress());
            message.insert(connection->getId());
            try {
                this->getConnection(targetId)->send(message);
            } catch (...) {
                connection->send(::MessageType::invalidTarget);
                ::std::cerr << "[ERROR:Server:TCP:" << connection->getUserName() << "] no connection at this ID.\n";
            }
            break;
        } case ::MessageType::acceptCall: {
            ::detail::Id targetId;
            message.extract(targetId);
            ::std::cout << "[" << connection->getUserName() << "] accepted [" << targetId << "]'s call.\n";
            message.insert(connection->getAddress());
            try {
                this->getConnection(targetId)->send(message);
            } catch (...) {
                connection->send(::MessageType::invalidTarget);
                ::std::cerr << "[ERROR:Server:TCP:" << connection->getUserName() << "] no connection at this ID.\n";
            }
            break;
        } case ::MessageType::refuseCall: {
            ::detail::Id targetId;
            message.extract(targetId);
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
