#pragma once

// ------------------------------------------------------------------ *structors

template <
    ::detail::isEnum UserMessageType
> ::network::tcp::Connection<UserMessageType>::Connection(
    ::asio::ip::tcp::socket socket
)
    : m_socket{ ::std::move(socket) }
{
    m_bufferIn.setTransmissionProtocol(::network::Message<UserMessageType>::TransmissionProtocol::tcp);
}


template <
    ::detail::isEnum UserMessageType
> ::network::tcp::Connection<UserMessageType>::~Connection()
{
    this->disconnect();
}



// ------------------------------------------------------------------ async - connection

template <
    ::detail::isEnum UserMessageType
> auto ::network::tcp::Connection<UserMessageType>::startConnectingToClient()
    -> bool
{
    if (m_connection->m_owner.getType() == ::network::ANode<UserMessageType>::Type::server) {
        if (m_socket.is_open()) {
            if (m_connection->m_owner.onConnect(m_connection->getPtr())) {
                this->identification();
                return true;
            } else {
                m_connection->m_owner.onConnectionDenial(m_connection->getPtr());
                m_connection->disconnect();
                return false;
            }
        } else {
            ::std::cerr << "[ERROR:Connection:TCP:connect] Invalid socket.\n";
            return false;
        }
    } else {
        ::std::cerr << "[ERROR:Connection:TCP:connect] A client cannot connect to another client.\n";
        return false;
    }
}

template <
    ::detail::isEnum UserMessageType
> void ::network::tcp::Connection<UserMessageType>::startConnectingToServer(
    const ::std::string& host,
    const ::std::uint16_t port
)
{
    if (m_connection->m_owner.getType() == ::network::ANode<UserMessageType>::Type::client) {
        m_socket.async_connect(
            ::asio::ip::tcp::endpoint{ ::asio::ip::address::from_string(host), port },
            [this](
                const ::std::error_code& errorCode
            ) {
                if (errorCode) {
                    if (errorCode == ::asio::error::operation_aborted) {
                        ::std::cerr << "[Connection:TCP:connect] Operation canceled\n";
                    } else {
                        ::std::cerr << "[ERROR:Connection:TCP:connect] "
                            << "Client failed to connect to the Server.\n";
                        m_connection->disconnect();
                    }
                } else {
                    ::std::cout << "[Connection:TCP] Connection accepted." << ::std::endl;
                    if (!m_connection->m_owner.onConnect(m_connection->getPtr())) {
                        m_connection->m_owner.onConnectionDenial(m_connection->getPtr());
                        m_connection->disconnect();
                    } else {
                        this->identification();
                    }
                }
            }
        );
    } else {
        throw ::std::runtime_error("A server cannot connect to another server.\n");
    }
}

template <
    ::detail::isEnum UserMessageType
> void ::network::tcp::Connection<UserMessageType>::disconnect()
{
    if (this->isConnected()) {
        m_socket.cancel();
        m_socket.close();
        this->notify();
        m_connection->m_owner.onDisconnect(m_connection->getPtr());
        ::std::cout << "[Connection:TCP:" << m_connection->informations.id << "] Disconnected.\n";
    }
    if (m_connection) {
        m_connection.reset();
    }
}

template <
    ::detail::isEnum UserMessageType
> auto ::network::tcp::Connection<UserMessageType>::isConnected() const
    -> bool
{
    return m_socket.is_open();
}

template <
    ::detail::isEnum UserMessageType
> void ::network::tcp::Connection<UserMessageType>::notify()
{
    m_blocker.notify_all();
}

template <
    ::detail::isEnum UserMessageType
> void ::network::tcp::Connection<UserMessageType>::waitNotification()
{
    ::std::unique_lock locker{ m_mutex };
    m_blocker.wait(locker);
}



// ------------------------------------------------------------------ async - out

template <
    ::detail::isEnum UserMessageType
> void ::network::tcp::Connection<UserMessageType>::send(
    UserMessageType messageType,
    auto&&... args
)
{
    ::network::Message message{
        ::std::forward<decltype(messageType)>(messageType),
        ::std::forward<decltype(args)>(args)...
    };
    ::asio::post(
        m_connection->m_owner.getAsioContext(),
        [this, message]()
        {
            auto needsToStartSending{ !this->hasSendingMessagesAwaiting() };
            m_messagesOut.push_back(::std::move(message));
            if (m_isSendAllowed && needsToStartSending) {
                this->sendAwaitingMessages();
            }
        }
    );
}

template <
    ::detail::isEnum UserMessageType
> void ::network::tcp::Connection<UserMessageType>::send(
    ::network::Message<UserMessageType> message
)
{
    ::asio::post(m_connection->m_owner.getAsioContext(), ::std::bind_front(
        [this](
            ::network::Message<UserMessageType> message
        )
        {
            auto needsToStartSending{ !this->hasSendingMessagesAwaiting() };
            m_messagesOut.push_back(::std::move(message));
            if (m_isSendAllowed && needsToStartSending) {
                this->sendAwaitingMessages();
            }
        },
        ::std::move(message)
    ));
}

template <
    ::detail::isEnum UserMessageType
> bool ::network::tcp::Connection<UserMessageType>::hasSendingMessagesAwaiting() const
{
    return !m_messagesOut.empty();
}

template <
    ::detail::isEnum UserMessageType
> void ::network::tcp::Connection<UserMessageType>::sendAwaitingMessages()
{

    this->sendMessage<[](::std::shared_ptr<::network::Connection<UserMessageType>> connection){
        connection->tcp.m_messagesOut.remove_front();
        if (connection->tcp.hasSendingMessagesAwaiting()) {
            connection->tcp.sendAwaitingMessages();
        }
    }>(m_messagesOut.front());
}



// ------------------------------------------------------------------ async - in

template <
    ::detail::isEnum UserMessageType
> void ::network::tcp::Connection<UserMessageType>::startReceivingMessage()
{
    this->receiveMessage<[](::std::shared_ptr<::network::Connection<UserMessageType>> connection){
        connection->tcp.transferBufferToInQueue();
        connection->tcp.startReceivingMessage();
    }>();
}



// ------------------------------------------------------------------ other

template <
    ::detail::isEnum UserMessageType
> auto ::network::tcp::Connection<UserMessageType>::getPort() const
    -> ::std::uint16_t
{
    return m_socket.local_endpoint().port();
}

template <
    ::detail::isEnum UserMessageType
> auto ::network::tcp::Connection<UserMessageType>::getAddress() const
    -> ::std::string
{
    return m_socket.local_endpoint().address().to_string();
}

template <
    ::detail::isEnum UserMessageType
> void ::network::tcp::Connection<UserMessageType>::assignConnection(
    ::std::shared_ptr<::network::Connection<UserMessageType>> connection
)
{
    m_connection = ::std::move(connection);
}



// ------------------------------------------------------------------ async - out

template <
    ::detail::isEnum UserMessageType
> template <
    auto successCallback
> void ::network::tcp::Connection<UserMessageType>::sendMessage(
    ::network::Message<UserMessageType> message,
    auto&&... args
)
{
    ::asio::async_write(
        m_socket,
        ::asio::buffer(message.getHeaderAddr(), message.getSendingHeaderSize()),
        ::std::bind(
            [this, id = m_connection->informations.id](
                const ::std::error_code& errorCode,
                const ::std::size_t length [[ maybe_unused ]],
                ::network::Message<UserMessageType> message,
                auto&&... args
            ) {
                if (errorCode) {
                    if (errorCode == ::asio::error::operation_aborted) {
                        ::std::cerr << "[Connection:TCP:" << id << "] Operation canceled.\n";
                    } else if (errorCode == ::asio::error::eof) {
                        ::std::cerr << "[Connection:TCP:" << id << "] Node stopped the connection.\n";
                        m_connection->disconnect();
                    } else {
                        ::std::cerr << "[ERROR:TCP:" << id << "] Write header failed: "
                            << errorCode.message() << ".\n";
                        m_connection->disconnect();
                    }
                } else {
                    if (!message.isBodyEmpty()) {
                        ::asio::async_write(
                            m_socket,
                            ::asio::buffer(message.getBodyAddr(), message.getBodySize()),
                            ::std::bind(
                                [this, id = m_connection->informations.id](
                                    const ::std::error_code& errorCode,
                                    const ::std::size_t length [[ maybe_unused ]],
                                    ::network::Message<UserMessageType> message,
                                    auto&&... args
                                ) {
                                    if (errorCode) {
                                        if (errorCode == ::asio::error::operation_aborted) {
                                            ::std::cerr << "[Connection:TCP:" << id
                                                << "] Operation canceled.\n";
                                        } else if (errorCode == ::asio::error::eof) {
                                            ::std::cerr << "[Connection:TCP:" << id
                                                << "] Node stopped the connection.\n";
                                            m_connection->disconnect();
                                        } else {
                                            ::std::cerr << "[ERROR:TCP:" << id
                                                << "] Write body failed: " << errorCode.message() << ".\n";
                                            m_connection->disconnect();
                                        }
                                    } else {
                                        successCallback(
                                            ::std::ref(m_connection),
                                            ::std::forward<decltype(args)>(args)...
                                        );
                                    }
                                },
                                ::std::placeholders::_1,
                                ::std::placeholders::_2,
                                ::std::move(message),
                                ::std::forward<decltype(args)>(args)...
                            )
                        );
                    } else {
                        successCallback(
                            ::std::ref(m_connection),
                            ::std::forward<decltype(args)>(args)...
                        );
                    }
                }
            },
            ::std::placeholders::_1,
            ::std::placeholders::_2,
            ::std::move(message),
            ::std::forward<decltype(args)>(args)...
        )
    );
}



// ------------------------------------------------------------------ async - in

template <
    ::detail::isEnum UserMessageType
> template <
    auto successCallback
> void ::network::tcp::Connection<UserMessageType>::receiveMessage(
    auto&&... args
)
{
    ::asio::async_read(
        m_socket,
        ::asio::buffer(m_bufferIn.getHeaderAddr(), m_bufferIn.getSendingHeaderSize()),
        ::std::bind(
            [this, id = m_connection->informations.id](
                const ::std::error_code& errorCode,
                const ::std::size_t length,
                auto&&... args
            ) {
                if (errorCode) {
                    if (errorCode == ::asio::error::operation_aborted) {
                        ::std::cerr << "[Connection:TCP:" << id << "] Operation canceled.\n";
                    } else if (errorCode == ::asio::error::eof) {
                        ::std::cerr << "[Connection:TCP:" << id << "] Node stopped the connection.\n";
                        m_connection->disconnect();
                    } else {
                        ::std::cerr << "[ERROR:TCP:" << id << "] Read header failed: "
                            << errorCode.message() << ".\n";
                        m_connection->disconnect();
                    }
                } else {
                    if (!m_bufferIn.isBodyEmpty()) {
                        m_bufferIn.updateBodySize();
                        ::asio::async_read(
                            m_socket,
                            ::asio::buffer(m_bufferIn.getBodyAddr(), m_bufferIn.getBodySize()),
                            ::std::bind(
                                [this, id = m_connection->informations.id](
                                    const ::std::error_code& errorCode,
                                    const ::std::size_t length,
                                    auto&&... args
                                ) {
                                    if (errorCode) {
                                        if (errorCode == ::asio::error::operation_aborted) {
                                            ::std::cerr << "[Connection:TCP:" << id
                                                << "] Operation canceled.\n";
                                        } else if (errorCode == ::asio::error::eof) {
                                            ::std::cerr << "[Connection:TCP:" << id
                                                << "] Node stopped the connection.\n";
                                            m_connection->disconnect();
                                        } else {
                                            ::std::cerr << "[ERROR:TCP:" << id << "] Read body failed: "
                                                << errorCode.message() << ".\n";
                                            m_connection->disconnect();
                                        }
                                    } else {
                                        successCallback(
                                            ::std::ref(m_connection),
                                            ::std::forward<decltype(args)>(args)...
                                        );
                                    }
                                },
                                ::std::placeholders::_1,
                                ::std::placeholders::_2,
                                ::std::forward<decltype(args)>(args)...
                            )
                        );
                    } else {
                        successCallback(
                            ::std::ref(m_connection),
                            ::std::forward<decltype(args)>(args)...
                        );
                    }
                }
            },
            ::std::placeholders::_1,
            ::std::placeholders::_2,
            ::std::forward<decltype(args)>(args)...
        )
    );
}

template <
    ::detail::isEnum UserMessageType
> void ::network::tcp::Connection<UserMessageType>::transferBufferToInQueue()
{
    if (m_connection->m_owner.getType() == ::network::ANode<UserMessageType>::Type::server) {
        m_connection->m_owner.pushIncommingMessage(
            network::OwnedMessage<UserMessageType>{ m_bufferIn, m_connection->getPtr() }
        );
    } else {
        m_connection->m_owner.pushIncommingMessage(network::OwnedMessage<UserMessageType>{ m_bufferIn, nullptr });
    }
}




// ------------------------------------------------------------------ includes

#include <Network/Tcp/Connection-identification.impl.hpp>
#include <Network/Tcp/Connection-authentification.impl.hpp>
