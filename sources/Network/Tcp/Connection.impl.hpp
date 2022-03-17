#pragma once

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// *structors
//
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
template <
    ::detail::constraint::isEnum UserMessageType
> ::network::tcp::Connection<UserMessageType>::Connection(
    ::asio::ip::tcp::socket socket
)
    : m_socket{ ::std::move(socket) }
{
    m_bufferIn.setTransmissionProtocol(::network::Message<UserMessageType>::TransmissionProtocol::tcp);
}



///////////////////////////////////////////////////////////////////////////////
template <
    ::detail::constraint::isEnum UserMessageType
> ::network::tcp::Connection<UserMessageType>::~Connection()
{
    this->disconnect();
}



///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// async (asio thread) - connection
//
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
template <
    ::detail::constraint::isEnum UserMessageType
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



///////////////////////////////////////////////////////////////////////////////
template <
    ::detail::constraint::isEnum UserMessageType
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
        throw ::std::logic_error("A server cannot connect to another server.\n");
    }
}



///////////////////////////////////////////////////////////////////////////////
template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::tcp::Connection<UserMessageType>::disconnect()
{
    if (this->isConnected()) {
        m_socket.cancel();
        m_socket.close();
        this->notify();
        m_connection->m_owner.onDisconnect(m_connection->getPtr());
        ::std::cout << "[Connection:TCP:" << m_connection->getId() << "] Disconnected.\n";
    }
    if (m_connection) {
        m_connection.reset();
    }
}



///////////////////////////////////////////////////////////////////////////////
template <
    ::detail::constraint::isEnum UserMessageType
> auto ::network::tcp::Connection<UserMessageType>::isConnected() const
    -> bool
{
    return m_socket.is_open();
}



///////////////////////////////////////////////////////////////////////////////
template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::tcp::Connection<UserMessageType>::notify()
{
    m_blocker.notify_all();
}



///////////////////////////////////////////////////////////////////////////////
template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::tcp::Connection<UserMessageType>::waitNotification()
{
    ::std::unique_lock locker{ m_mutex };
    m_blocker.wait(locker);
}



///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// async (asio thread) - outgoing messages
//
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::tcp::Connection<UserMessageType>::send(
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

///////////////////////////////////////////////////////////////////////////////
template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::tcp::Connection<UserMessageType>::send(
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



///////////////////////////////////////////////////////////////////////////////
template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::tcp::Connection<UserMessageType>::send(
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



///////////////////////////////////////////////////////////////////////////////
template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::tcp::Connection<UserMessageType>::send(
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



///////////////////////////////////////////////////////////////////////////////
template <
    ::detail::constraint::isEnum UserMessageType
> bool ::network::tcp::Connection<UserMessageType>::hasSendingMessagesAwaiting() const
{
    return !m_messagesOut.empty();
}



///////////////////////////////////////////////////////////////////////////////
template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::tcp::Connection<UserMessageType>::sendAwaitingMessages()
{
    this->sendQueueMessage<[](::std::shared_ptr<::network::Connection<UserMessageType>> connection){
        if (connection->tcp.hasSendingMessagesAwaiting()) {
            connection->tcp.sendAwaitingMessages();
        }
    }>();
}



///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// async (asio thread) - incomming messages
//
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::tcp::Connection<UserMessageType>::startReceivingMessage()
{
    this->receiveMessage<[](::std::shared_ptr<::network::Connection<UserMessageType>> connection){
        connection->tcp.transferBufferToInQueue();
        connection->tcp.startReceivingMessage();
    }>();
}



///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// helpers
//
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
template <
    ::detail::constraint::isEnum UserMessageType
> auto ::network::tcp::Connection<UserMessageType>::getPort() const
    -> ::std::uint16_t
{
    return m_socket.local_endpoint().port();
}



///////////////////////////////////////////////////////////////////////////////
template <
    ::detail::constraint::isEnum UserMessageType
> auto ::network::tcp::Connection<UserMessageType>::getAddress() const
    -> ::std::string
{
    return m_socket.local_endpoint().address().to_string();
}



///////////////////////////////////////////////////////////////////////////////
template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::tcp::Connection<UserMessageType>::assignConnection(
    ::std::shared_ptr<::network::Connection<UserMessageType>> connection
)
{
    m_connection = ::std::move(connection);
}



///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// async (asio thread) - outgoing messages
//
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
template <
    ::detail::constraint::isEnum UserMessageType
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
            [this, id = m_connection->getId()](
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
                                [this, id = m_connection->getId()](
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

template <
    ::detail::constraint::isEnum UserMessageType
> template <
    auto successCallback
> void ::network::tcp::Connection<UserMessageType>::sendQueueMessage(
    auto&&... args
)
{
    ::asio::async_write(
        m_socket,
        ::asio::buffer(m_messagesOut.front().getHeaderAddr(), m_messagesOut.front().getSendingHeaderSize()),
        ::std::bind(
            [this, id = m_connection->getId()](
                const ::std::error_code& errorCode,
                const ::std::size_t length [[ maybe_unused ]],
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
                    if (!m_messagesOut.front().isBodyEmpty()) {
                        ::asio::async_write(
                            m_socket,
                            ::asio::buffer(m_messagesOut.front().getBodyAddr(), m_messagesOut.front().getBodySize()),
                            ::std::bind(
                                [this, id = m_connection->getId()](
                                    const ::std::error_code& errorCode,
                                    const ::std::size_t length [[ maybe_unused ]],
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



///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// async (asio thread) - incomming messages
//
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
template <
    ::detail::constraint::isEnum UserMessageType
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
            [this, id = m_connection->getId()](
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
                                [this, id = m_connection->getId()](
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



///////////////////////////////////////////////////////////////////////////////
template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::tcp::Connection<UserMessageType>::transferBufferToInQueue()
{
    m_connection->m_owner.pushIncommingMessage(
        network::OwnedMessage<UserMessageType>{ m_bufferIn, m_connection->getPtr() }
    );
}



///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// async (asio thread) - identification
//
// Identification (Client claiming to identify as a client of the protocol):
//     1. Both send the public key
//     2. The server sends an handshake encrypted
//     3. The client resolves and sends the handshake back encrypted
//
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::tcp::Connection<UserMessageType>::identification()
{
#ifdef ENABLE_ENCRYPTION
    // send public key
    this->sendMessage<[](...){}>(::network::Message<UserMessageType>{
        ::network::Message<UserMessageType>::SystemType::publicKey, m_connection->m_cipher.getPublicKey()
    });

    // read public key
    this->receiveMessage<[](::std::shared_ptr<::network::Connection<UserMessageType>> connection){
        if (
            connection->tcp.m_bufferIn.getTypeAsSystemType() !=
            ::network::Message<UserMessageType>::SystemType::publicKey
        ) {
            ::std::cerr << "[ERROR:Identification:TCP:" << connection->getId() << "] Identification failed, "
                << "unexpected message received: " << connection->tcp.m_bufferIn.getTypeAsInt() << ".\n";
            connection->disconnect();
        } else {
            connection->m_cipher.setTargetPublicKey(connection->tcp.m_bufferIn.template pull<::security::Cipher::PublicKey>());
            if (connection->m_owner.getType() == ::network::ANode<UserMessageType>::Type::server) {
                connection->tcp.serverHandshake();
            } else {
                connection->tcp.clientHandshake();
            }
        }
    }>();

#else // ENABLE_ENCRYPTION

    if (m_owner.getType() == ::network::ANode<UserMessageType>::Type::server) {
        this->serverAcceptIdentification();
    } else {
        this->clientWaitIdentificationAcceptation();
    }
    ::std::cerr << "[Connection:TCP:" << m_connection->getId() << "] Identification ignored.\n";

#endif // ENABLE_ENCRYPTION
}



#ifdef ENABLE_ENCRYPTION



///////////////////////////////////////////////////////////////////////////////
template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::tcp::Connection<UserMessageType>::serverHandshake()
{
    auto baseValue{ m_connection->m_cipher.generateRandomData(1024) };
    auto baseValueCpy{ baseValue };

    // m_connection->m_cipher.encrypt(baseValueCpy);
    this->sendMessage<[](...){}>(::network::Message<UserMessageType>{
        ::network::Message<UserMessageType>::SystemType::proposeHandshake, ::std::move(baseValueCpy)
    });

    m_connection->m_cipher.scramble(baseValue);
    this->receiveMessage<[](
        ::std::shared_ptr<::network::Connection<UserMessageType>> connection,
        ::std::vector<::std::byte> baseValue
    ){
        if (
            connection->tcp.m_bufferIn.getTypeAsSystemType() !=
            ::network::Message<UserMessageType>::SystemType::resolveHandshake
        ) {
            ::std::cerr << "[ERROR:Identification:TCP:" << connection->getId() << "] Handshake failed, "
                << "unexpected message received: " << connection->tcp.m_bufferIn.getTypeAsInt() << ".\n";
            connection->disconnect();
        } else {
            auto receivedValue{ connection->tcp.m_bufferIn.template pull<::std::vector<::std::byte>>() };
            // connection->m_cipher.decrypt(receivedValue);
            if (receivedValue != baseValue) {
                ::std::cerr << "[ERROR:Identification:TCP:" << connection->getId()
                    << "] Handshake failed, incorrect value\n";
                connection->m_owner.onIdentificationDenial(connection);
                connection->tcp.sendIdentificationDenial();
                connection->disconnect();
            } else if (!connection->m_owner.onIdentification(connection)) {
                connection->m_owner.onIdentificationDenial(connection);
                connection->tcp.sendIdentificationDenial();
                connection->disconnect();
            } else {
                connection->tcp.serverAcceptIdentification();
                ::std::cerr << "[Connection:TCP:" << connection->getId() << "] Identification successful.\n";
            }
        }
    }>(::std::move(baseValue));
}



///////////////////////////////////////////////////////////////////////////////
template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::tcp::Connection<UserMessageType>::clientHandshake()
{
    this->receiveMessage<[](::std::shared_ptr<::network::Connection<UserMessageType>> connection){
        if (
            connection->tcp.m_bufferIn.getTypeAsSystemType() !=
            ::network::Message<UserMessageType>::SystemType::proposeHandshake
        ) {
            ::std::cerr << "[ERROR:Identification:TCP:" << connection->getId() << "] Handshake failed, "
                << "unexpected message received: " << connection->tcp.m_bufferIn.getTypeAsInt() << ".\n";
            connection->disconnect();
        } else {
            auto receivedValue{ connection->tcp.m_bufferIn.template pull<::std::vector<::std::byte>>() };
            // connection->m_cipher.decrypt(receivedValue);
            connection->m_cipher.scramble(receivedValue);
            // m_cipher.encrypt(receivedValue);
            connection->tcp.template sendMessage<
                [](
                    ::std::shared_ptr<::network::Connection<UserMessageType>> connection
                ) {
                    if (connection->m_owner.onIdentification(connection)) {
                        connection->tcp.clientWaitIdentificationAcceptation();
                    } else {
                        connection->disconnect();
                    }
                }
            >(::network::Message<UserMessageType>{
                ::network::Message<UserMessageType>::SystemType::resolveHandshake,
                ::std::move(receivedValue)
            });
        }
    }>();
}



///////////////////////////////////////////////////////////////////////////////
template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::tcp::Connection<UserMessageType>::sendIdentificationDenial()
{
    this->sendMessage<[](::std::shared_ptr<::network::Connection<UserMessageType>> connection){
        connection->disconnect();
    }>(::network::Message<UserMessageType>{
        ::network::Message<UserMessageType>::SystemType::identificationDenied, m_connection->getId()
    });
}



#endif // ENABLE_ENCRYPTION



///////////////////////////////////////////////////////////////////////////////
template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::tcp::Connection<UserMessageType>::serverAcceptIdentification()
{

    this->sendMessage<[](::std::shared_ptr<::network::Connection<UserMessageType>> connection){
        connection->tcp.serverAuthentification();
    }>(::network::Message<UserMessageType>{
        ::network::Message<UserMessageType>::SystemType::identificationAccepted,
        m_connection->getId()
    });
}



///////////////////////////////////////////////////////////////////////////////
template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::tcp::Connection<UserMessageType>::clientWaitIdentificationAcceptation()
{
    this->receiveMessage<[](::std::shared_ptr<::network::Connection<UserMessageType>> connection){
        if (
            connection->tcp.m_bufferIn.getTypeAsSystemType() ==
            ::network::Message<UserMessageType>::SystemType::identificationDenied
        ) {
            connection->m_owner.onIdentificationDenial(connection);
            connection->disconnect();
        } else if (
            connection->tcp.m_bufferIn.getTypeAsSystemType() !=
            ::network::Message<UserMessageType>::SystemType::identificationAccepted
        ) {
            ::std::cerr << "[ERROR:Identification:TCP:" << connection->getId() << "] Identification acceptance failed"
                << ", unexpected message received: " << connection->tcp.m_bufferIn.getTypeAsInt() << ".\n";
            connection->disconnect();
        } else {
            connection->setId(connection->tcp.m_bufferIn.template pull<::detail::Id>());
#ifdef ENABLE_ENCRYPTION
            ::std::cerr << "[Connection:TCP:" << connection->getId() << "] Identification successful.\n";
#endif // ENABLE_ENCRYPTION
            connection->tcp.clientAuthentification();
        }
    }>();
}



///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// async (asio thread) - authentification
//
// Authentification (Client registering with some provable way that they are who the claim to be):
//     1. Username
//     2. TODO: password
//
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::tcp::Connection<UserMessageType>::serverAuthentification()
{
    this->receiveMessage<[](::std::shared_ptr<::network::Connection<UserMessageType>> connection){
        if (
            connection->tcp.m_bufferIn.getTypeAsSystemType() !=
            ::network::Message<UserMessageType>::SystemType::authentification
        ) {
            ::std::cerr << "[ERROR:TCP:" << connection->getId() << "] Authentification failed, "
                << "unexpected message received.\n";
            return connection->disconnect();
        }

        auto password{ connection->tcp.m_bufferIn.template pull<::std::string>() };
        connection->setName(connection->tcp.m_bufferIn.template pull<::std::string>());

        if (!connection->m_owner.onAuthentification(connection)) {
            ::std::cerr << "[ERROR:TCP:" << connection->getId() << "] Authentification failed, "
                << "onAuthentification returned false.\n";
            connection->m_owner.onAuthentificationDenial(connection);
            connection->tcp.sendAuthentificationDenial();
            return connection->tcp.serverAuthentification();
        }

        connection->tcp.template sendMessage<[](::std::shared_ptr<::network::Connection<UserMessageType>> connection){
            ::std::cerr << "[Connection:TCP:" << connection->getId() << "] Authentification successful.\n";
            connection->tcp.setupUdp();
        }>(::network::Message<UserMessageType>{
            ::network::Message<UserMessageType>::SystemType::authentificationAccepted
        });
    }>();
}



///////////////////////////////////////////////////////////////////////////////
// TODO: mem error when closing the client after authentification denial
template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::tcp::Connection<UserMessageType>::clientAuthentification()
{
    m_connection->m_owner.onAuthentification(m_connection);
    this->sendMessage<[](::std::shared_ptr<::network::Connection<UserMessageType>> connection){
        connection->tcp.template receiveMessage<[](::std::shared_ptr<::network::Connection<UserMessageType>> connection){
            if (
                connection->tcp.m_bufferIn.getTypeAsSystemType() ==
                ::network::Message<UserMessageType>::SystemType::authentificationDenied
            ) {
                connection->m_owner.onAuthentificationDenial(connection);
                connection->tcp.clientAuthentification();
            } else if (
                connection->tcp.m_bufferIn.getTypeAsSystemType() !=
                ::network::Message<UserMessageType>::SystemType::authentificationAccepted
            ) {
                ::std::cerr << "[Connection:TCP:" << connection->getId() << "] invalid authentification acceptance\n";
                connection->disconnect();
            } else {
                connection->tcp.setupUdp();
            }
        }>();
    }>(::network::Message<UserMessageType>{
        ::network::Message<UserMessageType>::SystemType::authentification,
        m_connection->getName(),
        "password"s
    });
}


///////////////////////////////////////////////////////////////////////////////
template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::tcp::Connection<UserMessageType>::sendAuthentificationDenial()
{
    this->sendMessage<[](...){}>(::network::Message<UserMessageType>{
        ::network::Message<UserMessageType>::SystemType::authentificationDenied
    });
}



///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// async (asio thread) - set up UDP connection
//
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::tcp::Connection<UserMessageType>::setupUdp()
{
    // send udp informations
    this->sendMessage<[](...){}>(::network::Message<UserMessageType>{
        ::network::Message<UserMessageType>::SystemType::udpInformations,
        m_connection->udp.getPort()
    });

    // receive
    this->receiveMessage<[](::std::shared_ptr<::network::Connection<UserMessageType>> connection){
        if (
            connection->tcp.m_bufferIn.getTypeAsSystemType() !=
            ::network::Message<UserMessageType>::SystemType::udpInformations
        ) {
            ::std::cerr << "[ERROR:TCP:" << connection->getId() << "] Failed to setup Udp, "
                << "unexpected message received: " << connection->tcp.m_bufferIn.getTypeAsInt() << ".\n";
            connection->disconnect();
        } else {
            connection->udp.target(
                connection->tcp.getAddress(),
                connection->tcp.m_bufferIn.template pull<::std::uint16_t>()
            );
            connection->tcp.m_isSendAllowed = true;
            connection->m_owner.onConnectionValidated(connection);
            connection->tcp.m_blocker.notify_all();
        }
    }>();
}
