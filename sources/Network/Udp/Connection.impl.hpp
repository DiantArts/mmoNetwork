#pragma once

// ------------------------------------------------------------------ *structors

template <
    ::detail::constraint::isEnum UserMessageType
> ::network::udp::Connection<UserMessageType>::Connection(
    ::asio::io_context& asioContext
)
    : m_socket{ asioContext }
{
    m_socket.open(::asio::ip::udp::v4());
    m_socket.bind(::asio::ip::udp::endpoint(::asio::ip::udp::v4(), 0));
    m_bufferIn.setTransmissionProtocol(::network::Message<UserMessageType>::TransmissionProtocol::udp);
}


template <
    ::detail::constraint::isEnum UserMessageType
> ::network::udp::Connection<UserMessageType>::~Connection()
{
    this->close();
}



// ------------------------------------------------------------------ async - connection

template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::udp::Connection<UserMessageType>::target(
    const ::std::string& host,
    const ::std::uint16_t port
)
{
    m_socket.async_connect(
        ::asio::ip::udp::endpoint{ ::asio::ip::address::from_string(host), port },
        [&](
            const ::std::error_code& errorCode
        ) {
            if (errorCode) {
                if (errorCode == ::asio::error::operation_aborted) {
                    ::std::cerr << "[Connection:UDP:target] Operation canceled\n";
                } else {
                    ::std::cerr << "[ERROR:Connection:UDP:target] "
                        << "Client failed to target " << host << ':' << port << ".\n";
                    m_connection->disconnect();
                }
            } else {
                m_isSendAllowed = true;
                if (this->hasSendingMessagesAwaiting()) {
                    this->sendAwaitingMessages();
                }
            }
        }
    );
    ::std::cout << "[Connection::UDP] Targetting " << host << ":" << port << ".\n";
}

template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::udp::Connection<UserMessageType>::close()
{
    if (m_socket.is_open()) {
        m_isSendAllowed = false;
        m_socket.cancel();
        m_socket.close();
        ::std::cout << "[Connection:UDP:" << m_connection->getId() << "] Connection closed.\n";
    }
    if (m_connection) {
        m_connection.reset();
    }
}

template <
    ::detail::constraint::isEnum UserMessageType
> auto ::network::udp::Connection<UserMessageType>::isOpen() const
    -> bool
{
    return m_socket.is_open();
}



// ------------------------------------------------------------------ async - out

template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::udp::Connection<UserMessageType>::send(
    ::network::Message<UserMessageType>::SystemType messageType,
    auto&&... args
)
{
    ::asio::post(m_connection->m_owner.getAsioContext(), ::std::bind_front(
        [this](
            ::network::Message<UserMessageType> message
        ) {
            auto needsToStartSending{ !this->hasSendingMessagesAwaiting() };
            m_messagesOut.push_back(::std::move(message));
            if (m_isSendAllowed && needsToStartSending) {
                this->sendAwaitingMessages();
            }
        },
        ::network::Message<UserMessageType>{
            ::std::forward<decltype(messageType)>(messageType),
            ::std::forward<decltype(args)>(args)...
        }
    ));;
}

template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::udp::Connection<UserMessageType>::send(
    UserMessageType messageType,
    auto&&... args
)
{
    ::asio::post(m_connection->m_owner.getAsioContext(), ::std::bind_front(
        [this](
            ::network::Message<UserMessageType> message
        ) {
            auto needsToStartSending{ !this->hasSendingMessagesAwaiting() };
            m_messagesOut.push_back(::std::move(message));
            if (m_isSendAllowed && needsToStartSending) {
                this->sendAwaitingMessages();
            }
        },
        ::network::Message{
            ::std::forward<decltype(messageType)>(messageType),
            ::std::forward<decltype(args)>(args)...
        }
    ));;
}

template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::udp::Connection<UserMessageType>::send(
    const ::network::Message<UserMessageType>& message
)
{
    ::asio::post(m_connection->m_owner.getAsioContext(), ::std::bind_front(
        [this](
            ::network::Message<UserMessageType> message
        ) {
            auto needsToStartSending{ !this->hasSendingMessagesAwaiting() };
            m_messagesOut.push_back(::std::move(message));
            if (m_isSendAllowed && needsToStartSending) {
                this->sendAwaitingMessages();
            }
        },
        ::network::Message<UserMessageType>{ message }
    ));
}

template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::udp::Connection<UserMessageType>::send(
    ::network::Message<UserMessageType>&& message
)
{
    ::asio::post(m_connection->m_owner.getAsioContext(), ::std::bind_front(
        [this](
            ::network::Message<UserMessageType> message
        ) {
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
    ::detail::constraint::isEnum UserMessageType
> bool ::network::udp::Connection<UserMessageType>::hasSendingMessagesAwaiting() const
{
    return !m_messagesOut.empty();
}

template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::udp::Connection<UserMessageType>::sendAwaitingMessages()
{
    this->sendQueueMessage<[](::std::shared_ptr<::network::Connection<UserMessageType>> connection){
        if (connection->udp.hasSendingMessagesAwaiting()) {
            connection->udp.sendAwaitingMessages();
        }
    }>();
}



// ------------------------------------------------------------------ async - in

template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::udp::Connection<UserMessageType>::startReceivingMessage()
{
    this->receiveMessage<[](::std::shared_ptr<::network::Connection<UserMessageType>> connection){
        connection->udp.transferBufferToInQueue();
        connection->udp.startReceivingMessage();
    }>();
}



// ------------------------------------------------------------------ other

template <
    ::detail::constraint::isEnum UserMessageType
> auto ::network::udp::Connection<UserMessageType>::getPort() const
    -> ::std::uint16_t
{
    return m_socket.local_endpoint().port();
}

template <
    ::detail::constraint::isEnum UserMessageType
> auto ::network::udp::Connection<UserMessageType>::getAddress() const
    -> ::std::string
{
    return m_socket.local_endpoint().address().to_string();
}

template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::udp::Connection<UserMessageType>::assignConnection(
    ::std::shared_ptr<::network::Connection<UserMessageType>> connection
)
{
    m_connection = ::std::move(connection);
}



// ------------------------------------------------------------------ async - out

template <
    ::detail::constraint::isEnum UserMessageType
> template <
    auto successCallback
> void ::network::udp::Connection<UserMessageType>::sendMessage(
    ::network::Message<UserMessageType> message,
    auto&&... args
)
{
    this->sendMessageHeader<successCallback>(::std::move(message), 0, ::std::forward<decltype(args)>(args)...);
}

template <
    ::detail::constraint::isEnum UserMessageType
> template <
    auto successCallback
> void ::network::udp::Connection<UserMessageType>::sendMessageHeader(
    ::network::Message<UserMessageType> message,
    ::std::size_t bytesAlreadySent,
    auto&&... args
)
{
    m_socket.async_send(
        ::asio::buffer(
            message.getHeaderAddr() + bytesAlreadySent,
            message.getSendingHeaderSize() - bytesAlreadySent
        ),
        ::std::bind(
            [this, bytesAlreadySent, id = m_connection->getId()](
                const ::std::error_code& errorCode,
                const ::std::size_t length [[ maybe_unused ]],
                ::network::Message<UserMessageType> message,
                auto&&... args
            ) {
                if (errorCode) {
                    if (errorCode == ::asio::error::operation_aborted) {
                        ::std::cerr << "[Connection:UDP:" << id << "] Operation canceled.\n";
                    } else if (errorCode == ::asio::error::eof) {
                        ::std::cerr << "[Connection:UDP:" << id << "] Node stopped the connection.\n";
                        m_connection->disconnect();
                    } else {
                        ::std::cerr << "[ERROR:UDP:" << id << "] Send header failed: "
                            << errorCode.message() << ".\n";
                        m_connection->disconnect();
                    }
                } else if (bytesAlreadySent + length < message.getSendingHeaderSize()) {
                    this->sendMessageHeader<successCallback>(::std::move(message), bytesAlreadySent + length, ::std::forward<decltype(args)>(args)...);
                } else {
                    if (!message.isBodyEmpty()) {
                        this->sendMessageBody<successCallback>(::std::move(message), 0, ::std::forward<decltype(args)>(args)...);
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

template <
    ::detail::constraint::isEnum UserMessageType
> template <
    auto successCallback
> void ::network::udp::Connection<UserMessageType>::sendMessageBody(
    ::network::Message<UserMessageType> message,
    ::std::size_t bytesAlreadySent,
    auto&&... args
)
{
    m_socket.async_send(
        ::asio::buffer(
            message.getBodyAddr() + bytesAlreadySent,
            message.getBodySize() - bytesAlreadySent
        ),
        ::std::bind(
            [this, bytesAlreadySent, id = m_connection->getId()](
                const ::std::error_code& errorCode,
                const ::std::size_t length [[ maybe_unused ]],
                ::network::Message<UserMessageType> message,
                auto&&... args
            ) {
                if (errorCode) {
                    if (errorCode == ::asio::error::operation_aborted) {
                        ::std::cerr << "[Connection:UDP:" << id << "] Operation canceled.\n";
                    } else if (errorCode == ::asio::error::eof) {
                        ::std::cerr << "[Connection:UDP:" << id << "] Node stopped the connection.\n";
                        m_connection->disconnect();
                    } else {
                        ::std::cerr << "[ERROR:UDP:" << id << "] Send body failed: "
                            << errorCode.message() << ".\n";
                        m_connection->disconnect();
                    }
                } else if (bytesAlreadySent + length < message.getBodySize()) {
                    this->sendMessageBody<successCallback>(::std::move(message), bytesAlreadySent + length, ::std::forward<decltype(args)>(args)...);
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
}



template <
    ::detail::constraint::isEnum UserMessageType
> template <
    auto successCallback
> void ::network::udp::Connection<UserMessageType>::sendQueueMessage(
    auto&&... args
)
{
    this->sendQueueMessageHeader<successCallback>(0, ::std::forward<decltype(args)>(args)...);
}

template <
    ::detail::constraint::isEnum UserMessageType
> template <
    auto successCallback
> void ::network::udp::Connection<UserMessageType>::sendQueueMessageHeader(
    ::std::size_t bytesAlreadySent,
    auto&&... args
)
{
    m_socket.async_send(
        ::asio::buffer(
            m_messagesOut.front().getHeaderAddr() + bytesAlreadySent,
            m_messagesOut.front().getSendingHeaderSize() - bytesAlreadySent
        ),
        ::std::bind(
            [this, bytesAlreadySent, id = m_connection->getId()](
                const ::std::error_code& errorCode,
                const ::std::size_t length [[ maybe_unused ]],
                auto&&... args
            ) {
                if (errorCode) {
                    if (errorCode == ::asio::error::operation_aborted) {
                        ::std::cerr << "[Connection:UDP:" << id << "] Operation canceled.\n";
                    } else if (errorCode == ::asio::error::eof) {
                        ::std::cerr << "[Connection:UDP:" << id << "] Node stopped the connection.\n";
                        m_connection->disconnect();
                    } else {
                        ::std::cerr << "[ERROR:UDP:" << id << "] Send header failed: "
                            << errorCode.message() << ".\n";
                        m_connection->disconnect();
                    }
                } else if (bytesAlreadySent + length < m_messagesOut.front().getSendingHeaderSize()) {
                    this->sendQueueMessageHeader<successCallback>(bytesAlreadySent + length, ::std::forward<decltype(args)>(args)...);
                } else {
                    if (!m_messagesOut.front().isBodyEmpty()) {
                        this->sendQueueMessageBody<successCallback>(0, ::std::forward<decltype(args)>(args)...);
                    } else {
                        m_messagesOut.remove_front();
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
    ::detail::constraint::isEnum UserMessageType
> template <
    auto successCallback
> void ::network::udp::Connection<UserMessageType>::sendQueueMessageBody(
    ::std::size_t bytesAlreadySent,
    auto&&... args
)
{
    m_socket.async_send(
        ::asio::buffer(
            m_messagesOut.front().getBodyAddr() + bytesAlreadySent,
            m_messagesOut.front().getBodySize() - bytesAlreadySent
        ),
        ::std::bind(
            [this, bytesAlreadySent, id = m_connection->getId()](
                const ::std::error_code& errorCode,
                const ::std::size_t length [[ maybe_unused ]],
                auto&&... args
            ) {
                if (errorCode) {
                    if (errorCode == ::asio::error::operation_aborted) {
                        ::std::cerr << "[Connection:UDP:" << id << "] Operation canceled.\n";
                    } else if (errorCode == ::asio::error::eof) {
                        ::std::cerr << "[Connection:UDP:" << id << "] Node stopped the connection.\n";
                        m_connection->disconnect();
                    } else {
                        ::std::cerr << "[ERROR:UDP:" << id << "] Send body failed: "
                            << errorCode.message() << ".\n";
                        m_connection->disconnect();
                    }
                } else if (bytesAlreadySent + length < m_messagesOut.front().getBodySize()) {
                    this->sendQueueMessageBody<successCallback>(bytesAlreadySent + length, ::std::forward<decltype(args)>(args)...);
                } else {
                    m_messagesOut.remove_front();
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
}



// ------------------------------------------------------------------ async - in

template <
    ::detail::constraint::isEnum UserMessageType
> template <
    auto successCallback
> void ::network::udp::Connection<UserMessageType>::receiveMessage(
    auto&&... args
)
{
    this->receiveMessageHeader<successCallback>(0, ::std::forward<decltype(args)>(args)...);
}

template <
    ::detail::constraint::isEnum UserMessageType
> template <
    auto successCallback
> void ::network::udp::Connection<UserMessageType>::receiveMessageHeader(
    ::std::size_t bytesAlreadyRead,
    auto&&... args
)
{
    m_socket.async_receive(
        ::asio::buffer(
            m_bufferIn.getHeaderAddr() + bytesAlreadyRead,
            m_bufferIn.getSendingHeaderSize() - bytesAlreadyRead
        ),
        ::std::bind(
            [this, bytesAlreadyRead, id = m_connection->getId()](
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
                        ::std::cerr << "[ERROR:TCP:" << id << "] Receive header failed: "
                            << errorCode.message() << ".\n";
                        m_connection->disconnect();
                    }
                } else if (bytesAlreadyRead + length < m_bufferIn.getSendingHeaderSize()) {
                    this->receiveMessageHeader<successCallback>(bytesAlreadyRead + length, ::std::forward<decltype(args)>(args)...);
                } else {
                    if (!m_bufferIn.isBodyEmpty()) {
                        m_bufferIn.updateBodySize();
                        this->receiveMessageBody<successCallback>(0, ::std::forward<decltype(args)>(args)...);
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
    ::detail::constraint::isEnum UserMessageType
> template <
    auto successCallback
> void ::network::udp::Connection<UserMessageType>::receiveMessageBody(
    ::std::size_t bytesAlreadyRead,
    auto&&... args
)
{
    m_socket.async_receive(
        ::asio::buffer(
            m_bufferIn.getBodyAddr() + bytesAlreadyRead,
            m_bufferIn.getBodySize() - bytesAlreadyRead
        ),
        ::std::bind(
            [this, bytesAlreadyRead, id = m_connection->getId()](
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
                        ::std::cerr << "[ERROR:TCP:" << id << "] Receive body failed: "
                            << errorCode.message() << ".\n";
                        m_connection->disconnect();
                    }
                } else if (bytesAlreadyRead + length < m_bufferIn.getBodySize()) {
                    this->receiveMessageBody<successCallback>(bytesAlreadyRead + length, ::std::forward<decltype(args)>(args)...);
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
}

template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::udp::Connection<UserMessageType>::transferBufferToInQueue()
{
    m_connection->m_owner.pushIncommingMessage(
        network::OwnedMessage<UserMessageType>{ m_bufferIn, m_connection->getPtr() }
    );
}
