#pragma once



// ------------------------------------------------------------------ *structors

template <
    ::detail::isEnum UserMessageType
> ::network::TcpConnection<UserMessageType>::TcpConnection(
    ::network::ANode<UserMessageType>& owner,
    ::asio::ip::tcp::socket socket
)
    : m_owner{ owner }
    , m_socket{ ::std::move(socket) }
{
    m_bufferIn.setTransmissionProtocol(::network::Message<UserMessageType>::TransmissionProtocol::tcp);
}


template <
    ::detail::isEnum UserMessageType
> ::network::TcpConnection<UserMessageType>::~TcpConnection()
{
    if (this->isConnected()) {
        m_socket.cancel();
        m_socket.close();
        ::std::cout << "[Connection:TCP:" << m_id << "] Disconnected.\n";
    }
}



// ------------------------------------------------------------------ async - connection

template <
    ::detail::isEnum UserMessageType
> auto ::network::TcpConnection<UserMessageType>::connectToClient(
    ::detail::Id id
)
    -> bool
{
    if (m_owner.getType() == ::network::ANode<UserMessageType>::Type::server) {
        if (m_socket.is_open()) {
            if (m_owner.onConnect(this->shared_from_this())) {
                m_id = id;
                this->identification();
                return true;
            } else {
                m_owner.onConnectionDenial(this->shared_from_this());
                this->disconnect();
                return false;
            }
        } else {
            ::std::cerr << "[ERROR:NewConnection] Invalid socket.\n";
            return false;
        }
    } else {
        ::std::cerr << "[ERROR:TcpConnection] A client cannot connect to another client.\n";
        return false;
    }
}

template <
    ::detail::isEnum UserMessageType
> void ::network::TcpConnection<UserMessageType>::connect(
    const ::std::string& host,
    const ::std::uint16_t port
)
{
    if (m_owner.getType() == ::network::ANode<UserMessageType>::Type::client) {
        m_socket.async_connect(
            ::asio::ip::tcp::endpoint{ ::asio::ip::address::from_string(host), port },
            [this](
                const ::std::error_code& errorCode
            ) {
                if (errorCode) {
                    if (errorCode == ::asio::error::operation_aborted) {
                        ::std::cerr << "[Connection:TCP] Operation canceled\n";
                    } else {
                        ::std::cerr << "[ERROR:TCP] Client failed to connect to the Server.\n";
                        this->disconnect();
                    }
                } else {
                    ::std::cout << "[Connection:TCP] Connection accepted." << ::std::endl;
                    if (!m_owner.onConnect(this->shared_from_this())) {
                        m_owner.onConnectionDenial(this->shared_from_this());
                        this->disconnect();
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
> void ::network::TcpConnection<UserMessageType>::disconnect()
{
    if (this->isConnected()) {
        m_socket.cancel();
        m_socket.close();
        auto id{ m_id };
        m_owner.onDisconnect(this->shared_from_this());
        ::std::cout << "[Connection:TCP:" << id << "] Disconnected.\n";
    }
}

template <
    ::detail::isEnum UserMessageType
> auto ::network::TcpConnection<UserMessageType>::isConnected() const
    -> bool
{
    return m_socket.is_open();
}



// ------------------------------------------------------------------ async - out

template <
    ::detail::isEnum UserMessageType
> void ::network::TcpConnection<UserMessageType>::send(
    UserMessageType messageType,
    auto&&... args
)
{
    ::network::Message message{
        ::std::forward<decltype(messageType)>(messageType),
        ::std::forward<decltype(args)>(args)...
    };
    ::asio::post(
        m_owner.getAsioContext(),
        [this, message]()
        {
            auto needsToStartSending{ !this->hasSendingMessagesAwaiting() };
            m_messagesOut.push_back(::std::move(message));
            if (m_isValid && needsToStartSending) {
                this->writeAwaitingMessages();
            }
        }
    );
}

template <
    ::detail::isEnum UserMessageType
> void ::network::TcpConnection<UserMessageType>::send(
    ::network::Message<UserMessageType> message
)
{
    ::asio::post(m_owner.getAsioContext(), ::std::bind_front(
        [this](
            ::network::Message<UserMessageType> message
        )
        {
            auto needsToStartSending{ !this->hasSendingMessagesAwaiting() };
            m_messagesOut.push_back(::std::move(message));
            if (m_isValid && needsToStartSending) {
                this->writeAwaitingMessages();
            }
        },
        ::std::move(message)
    ));
}

template <
    ::detail::isEnum UserMessageType
> bool ::network::TcpConnection<UserMessageType>::hasSendingMessagesAwaiting() const
{
    return !m_messagesOut.empty();
}

template <
    ::detail::isEnum UserMessageType
> void ::network::TcpConnection<UserMessageType>::writeAwaitingMessages()
{

    this->sendMessage<
        [](::network::TcpConnection<UserMessageType>& self){
            self.m_messagesOut.remove_front();
            if (self.hasSendingMessagesAwaiting()) {
                self.writeAwaitingMessages();
            }
        },
        [](::network::TcpConnection<UserMessageType>& self, const ::std::error_code& errorCode){
            ::std::cerr << "[ERROR:TCP:" << self.m_id << "] Write header failed: " << errorCode.message() << ".\n";
            self.disconnect();
        },
        [](::network::TcpConnection<UserMessageType>& self, const ::std::error_code& errorCode){
            ::std::cerr << "[ERROR:TCP:" << self.m_id << "] Write body failed: " << errorCode.message() << ".\n";
            self.disconnect();
        }
    >(m_messagesOut.front());
}



// ------------------------------------------------------------------ async - in

template <
    ::detail::isEnum UserMessageType
> void ::network::TcpConnection<UserMessageType>::startReadMessage()
{
    this->receiveMessage<
        [](::network::TcpConnection<UserMessageType>& self){
            self.transferBufferToInQueue();
            self.startReadMessage();
        },
        [](::network::TcpConnection<UserMessageType>& self, const ::std::error_code& errorCode){
            if (errorCode == ::asio::error::operation_aborted) {
                ::std::cerr << "[Connection:TCP] Operation canceled\n";
            } else if (errorCode == ::asio::error::eof) {
                ::std::cerr << "[Connection:TCP:" << self.m_id << "] Node stopped the connection.\n";
                self.disconnect();
            } else {
                ::std::cerr << "[ERROR:TCP:" << self.m_id << "] Read header failed: " << errorCode.message() << ".\n";
                self.disconnect();
            }
        },
        [](::network::TcpConnection<UserMessageType>& self, const ::std::error_code& errorCode){
            ::std::cerr << "[ERROR:TCP:" << self.m_id << "] Read body failed: " << errorCode.message() << ".\n";
            self.disconnect();
        }
    >();
}



// ------------------------------------------------------------------ other

template <
    ::detail::isEnum UserMessageType
> auto ::network::TcpConnection<UserMessageType>::getSharableInformations() const
    -> ::std::pair<::std::string, ::detail::Id>
{
    return ::std::make_pair(::std::string{ m_userName }, ::detail::Id{ m_id });
}

template <
    ::detail::isEnum UserMessageType
> auto ::network::TcpConnection<UserMessageType>::getId() const
    -> ::detail::Id
{
    return m_id;
}

template <
    ::detail::isEnum UserMessageType
> auto ::network::TcpConnection<UserMessageType>::getOwner() const
    -> const ::network::ANode<UserMessageType>&
{
    return m_owner;
}

template <
    ::detail::isEnum UserMessageType
> auto ::network::TcpConnection<UserMessageType>::getPort() const
    -> ::std::uint16_t
{
    return m_socket.local_endpoint().port();
}

template <
    ::detail::isEnum UserMessageType
> auto ::network::TcpConnection<UserMessageType>::getAddress() const
    -> ::std::string
{
    return m_socket.local_endpoint().address().to_string();
}

template <
    ::detail::isEnum UserMessageType
> auto ::network::TcpConnection<UserMessageType>::getUserName() const
    -> const ::std::string&
{
    return m_userName;
}

template <
    ::detail::isEnum UserMessageType
> void ::network::TcpConnection<UserMessageType>::setUserName(
    ::std::string newName
)
{
    m_userName = newName;
    ::std::cerr << "[Connection:TCP:" << m_id << "] New user name set: " << m_userName << ".\n";
}



// ------------------------------------------------------------------ async - out

template <
    ::detail::isEnum UserMessageType
> template <
    auto successCallback,
    auto failureHeaderCallback,
    auto failureBodyCallback
> void ::network::TcpConnection<UserMessageType>::sendMessage(
    ::network::Message<UserMessageType> message
)
{
    ::asio::async_write(
        m_socket,
        ::asio::buffer(message.getHeaderAddr(), message.getSendingHeaderSize()),
        ::std::bind_front(
            [this](
                ::network::Message<UserMessageType> message,
                const ::std::error_code& errorCode,
                const ::std::size_t length
            ) {
                if (errorCode) {
                    if (errorCode == ::asio::error::operation_aborted) {
                        ::std::cerr << "[Connection:TCP] Operation canceled\n";
                    } else {
                        failureHeaderCallback(::std::ref(*this), errorCode);
                    }
                } else {
                    if (!message.isBodyEmpty()) {
                        ::asio::async_write(
                            m_socket,
                            ::asio::buffer(message.getBodyAddr(), message.getBodySize()),
                            ::std::bind_front(
                                [this](
                                    ::network::Message<UserMessageType> message,
                                    const ::std::error_code& errorCode,
                                    const ::std::size_t length
                                ) {
                                    if (errorCode) {
                                        if (errorCode == ::asio::error::operation_aborted) {
                                            ::std::cerr << "[Connection:TCP] Operation canceled\n";
                                        } else {
                                            failureBodyCallback(::std::ref(*this), errorCode);
                                        }
                                    } else {
                                        successCallback(::std::ref(*this));
                                    }
                                },
                                ::std::move(message)
                            )
                        );
                    } else {
                        successCallback(::std::ref(*this));
                    }
                }
            },
            ::std::move(message)
        )
    );
}

template <
    ::detail::isEnum UserMessageType
> template <
    typename Type,
    auto successCallback,
    auto failureCallback
> void ::network::TcpConnection<UserMessageType>::sendRawData(
    auto&&... args
)
{
    auto pointerToData{ new Type{ ::std::forward<decltype(args)>(args)... } };
    ::asio::async_write(
        m_socket,
        ::asio::buffer(pointerToData, sizeof(Type)),
        [this, pointerToData](
            const ::std::error_code& errorCode,
            const ::std::size_t length
        ) {
            if (errorCode) {
                failureCallback(::std::ref(*this), errorCode);
            } else {
                successCallback(::std::ref(*this));
            }
            delete pointerToData;
        }
    );
}

template <
    ::detail::isEnum UserMessageType
> template <
    auto successCallback,
    auto failureCallback
> void ::network::TcpConnection<UserMessageType>::sendRawData(
    ::detail::isPointer auto pointerToData,
    ::std::size_t dataSize
)
{
    ::asio::async_write(
        m_socket,
        ::asio::buffer(pointerToData, dataSize),
        [this](
            const ::std::error_code& errorCode,
            const ::std::size_t length
        ) {
            if (errorCode) {
                failureCallback(::std::ref(*this), errorCode);
            } else {
                successCallback(::std::ref(*this));
            }
        }
    );
}



// ------------------------------------------------------------------ async - in

template <
    ::detail::isEnum UserMessageType
> template <
    auto successCallback,
    auto failureHeaderCallback,
    auto failureBodyCallback
> void ::network::TcpConnection<UserMessageType>::receiveMessage()
{
    ::asio::async_read(
        m_socket,
        ::asio::buffer(m_bufferIn.getHeaderAddr(), m_bufferIn.getSendingHeaderSize()),
        [this](
            const ::std::error_code& errorCode,
            const ::std::size_t length
        ) {
            if (errorCode) {
                if (errorCode == ::asio::error::operation_aborted) {
                    ::std::cerr << "[Connection:TCP] Operation canceled\n";
                } else {
                    failureHeaderCallback(::std::ref(*this), errorCode);
                }
            } else {
                if (!m_bufferIn.isBodyEmpty()) {
                    m_bufferIn.updateBodySize();
                    ::asio::async_read(
                        m_socket,
                        ::asio::buffer(m_bufferIn.getBodyAddr(), m_bufferIn.getBodySize()),
                        [this](
                            const ::std::error_code& errorCode,
                            const ::std::size_t length
                        ) {
                            if (errorCode) {
                                if (errorCode == ::asio::error::operation_aborted) {
                                    ::std::cerr << "[Connection:TCP] Operation canceled\n";
                                } else {
                                    failureBodyCallback(::std::ref(*this), errorCode);
                                }
                            } else {
                                successCallback(::std::ref(*this));
                            }
                        }
                    );
                } else {
                    successCallback(::std::ref(*this));
                }
            }
        }
    );
}

template <
    ::detail::isEnum UserMessageType
> template <
    typename Type,
    auto successCallback,
    auto failureCallback
> void ::network::TcpConnection<UserMessageType>::receiveRawData()
{
    auto pointerToData{ new ::std::array<::std::byte, sizeof(Type)> };
    ::asio::async_read(
        m_socket,
        ::asio::buffer(pointerToData->data(), sizeof(Type)),
        [this, pointerToData](
            const ::std::error_code& errorCode,
            const ::std::size_t length
        ) {
            if (errorCode) {
                failureCallback(::std::ref(*this), errorCode);
            } else {
                successCallback(::std::ref(*this), *static_cast<Type*>(pointerToData->data()));
            }
            delete pointerToData;
        }
    );
}

template <
    ::detail::isEnum UserMessageType
> template <
    auto successCallback,
    auto failureCallback
> void ::network::TcpConnection<UserMessageType>::receiveToRawData(
    ::detail::isPointer auto pointerToData,
    ::std::size_t dataSize
)
{
    ::asio::async_read(
        m_socket,
        ::asio::buffer(pointerToData, dataSize),
        [this, pointerToData](
            const ::std::error_code& errorCode,
            const ::std::size_t length
        ) {
            if (errorCode) {
                failureCallback(::std::ref(*this), errorCode);
            } else {
                successCallback(::std::ref(*this));
            }
        }
    );
}

template <
    ::detail::isEnum UserMessageType
> void ::network::TcpConnection<UserMessageType>::transferBufferToInQueue()
{
    if (m_owner.getType() == ::network::ANode<UserMessageType>::Type::server) {
        m_owner.pushIncommingMessage(
            network::OwnedMessage<UserMessageType>{ m_bufferIn, this->shared_from_this() }
        );
    } else {
        m_owner.pushIncommingMessage(network::OwnedMessage<UserMessageType>{ m_bufferIn, nullptr });
    }
}




// ------------------------------------------------------------------ includes

#include <Network/TcpConnection-identification.impl.hpp>
#include <Network/TcpConnection-authentification.impl.hpp>
