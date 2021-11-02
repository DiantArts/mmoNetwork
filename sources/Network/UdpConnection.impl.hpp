#pragma once

// ------------------------------------------------------------------ *structors

template <
    ::detail::isEnum UserMessageType
> ::network::UdpConnection<UserMessageType>::UdpConnection(
    ::network::AClient<UserMessageType>& owner
)
    : m_owner{ owner }
    , m_socket{ owner.getAsioContext() }
{
    m_socket.open(::asio::ip::udp::v4());
    m_socket.bind(::asio::ip::udp::endpoint(::asio::ip::udp::v4(), 0));
    m_bufferIn.setTransmissionProtocol(::network::Message<UserMessageType>::TransmissionProtocol::udp);
}


template <
    ::detail::isEnum UserMessageType
> ::network::UdpConnection<UserMessageType>::~UdpConnection()
{
    if (m_socket.is_open()) {
        m_socket.cancel();
        m_socket.close();
        ::std::cout << "[Connection:UDP] Disconnected.\n";
    }
}



// ------------------------------------------------------------------ async - connection

template <
    ::detail::isEnum UserMessageType
> void ::network::UdpConnection<UserMessageType>::target(
    const ::std::string& host,
    const ::std::uint16_t port
)
{
    m_socket.async_connect(
        ::asio::ip::udp::endpoint{ ::asio::ip::address::from_string(host), port },
        [this](
            const ::std::error_code& errorCode
        ) {
            if (errorCode) {
                if (errorCode == ::asio::error::operation_aborted) {
                    ::std::cerr << "[Connection:UDP] Operation canceled\n";
                } else {
                    ::std::cerr << "[ERROR:UDP] Client failed to connect to the tcp Server.\n";
                    this->close();
                }
            } else {
                this->readHeader();
            }
        }
    );
        ::std::cout << "[Client:UDP] Targetting " << host << ":" << port << ".\n";
}

template <
    ::detail::isEnum UserMessageType
> void ::network::UdpConnection<UserMessageType>::close()
{
    if (m_socket.is_open()) {
        m_socket.cancel();
        m_socket.close();
        m_owner.onUdpDisconnect(this->shared_from_this());
        ::std::cout << "[Connection:UDP] Connection closed.\n";
    }
}

template <
    ::detail::isEnum UserMessageType
> auto ::network::UdpConnection<UserMessageType>::isOpen() const
    -> bool
{
    return m_socket.is_open();
}



// ------------------------------------------------------------------ async - out

template <
    ::detail::isEnum UserMessageType
> void ::network::UdpConnection<UserMessageType>::send(
    ::network::Message<UserMessageType>&& message
)
{
    ::asio::post(m_owner.getAsioContext(), ::std::bind_front(
        [this](
            ::network::Message<UserMessageType> message
        )
        {
            auto wasOutQueueEmpty{ m_messagesOut.empty() };
            m_messagesOut.push_back(::std::move(message));
            if (wasOutQueueEmpty) {
                this->writeHeader();
            }
        },
        ::std::move(message)
    ));
}



// ------------------------------------------------------------------ other

template <
    ::detail::isEnum UserMessageType
> auto ::network::UdpConnection<UserMessageType>::getOwner() const
    -> const ::network::AClient<UserMessageType>&
{
    return m_owner;
}

template <
    ::detail::isEnum UserMessageType
> auto ::network::UdpConnection<UserMessageType>::getPort() const
    -> ::std::uint16_t
{
    return m_socket.local_endpoint().port();
}

template <
    ::detail::isEnum UserMessageType
> auto ::network::UdpConnection<UserMessageType>::getAddress() const
    -> ::std::string
{
    return m_socket.local_endpoint().address().to_string();
}



// ------------------------------------------------------------------ async - out

template <
    ::detail::isEnum UserMessageType
> void ::network::UdpConnection<UserMessageType>::writeHeader(
    ::std::size_t bytesAlreadySent /* = 0 */
)
{
    m_socket.async_send(
        ::asio::buffer(
            m_messagesOut.front().getHeaderAddr() + bytesAlreadySent,
            m_messagesOut.front().getSendingHeaderSize() - bytesAlreadySent
        ),
        [this, bytesAlreadySent](
            const ::std::error_code& errorCode,
            const ::std::size_t length
        ) {
            if (errorCode) {
                if (errorCode == ::asio::error::operation_aborted) {
                    ::std::cerr << "[Connection:UDP] Operation canceled\n";
                } else {
                    ::std::cerr << "[ERROR:UDP] Write header failed: " << errorCode.message() << ".\n";
                    this->close();
                }
            } else if (bytesAlreadySent + length < m_messagesOut.front().getSendingHeaderSize()) {
                this->writeHeader(bytesAlreadySent + length);
            } else {
                if (!m_messagesOut.front().isBodyEmpty()) {
                    this->writeBody();
                } else {
                    m_messagesOut.remove_front();
                    if (!m_messagesOut.empty()) {
                        this->writeHeader();
                    }
                }
            }
        }
    );
}

template <
    ::detail::isEnum UserMessageType
> void ::network::UdpConnection<UserMessageType>::writeBody(
    ::std::size_t bytesAlreadySent /* = 0 */
)
{
    m_socket.async_send(
        ::asio::buffer(
            m_messagesOut.front().getBodyAddr() + bytesAlreadySent,
            m_messagesOut.front().getBodySize() - bytesAlreadySent
        ),
        [this, bytesAlreadySent](
            const ::std::error_code& errorCode,
            const ::std::size_t length
        ) {
            if (errorCode) {
                if (errorCode == ::asio::error::operation_aborted) {
                    ::std::cerr << "[Connection:UDP] Operation canceled\n";
                } else {
                    ::std::cerr << "[ERROR:UDP] Write body failed: " << errorCode.message() << ".\n";
                    this->close();
                }
            } else if (bytesAlreadySent + length < m_messagesOut.front().getBodySize()) {
                this->writeBody(bytesAlreadySent + length);
            } else {
                m_messagesOut.remove_front();
                if (!m_messagesOut.empty()) {
                    this->writeHeader();
                }
            }
        }
    );
}




// ------------------------------------------------------------------ async - in

template <
    ::detail::isEnum UserMessageType
> void ::network::UdpConnection<UserMessageType>::readHeader(
    ::std::size_t bytesAlreadyRead /* = 0 */
)
{
    m_socket.async_receive(
        ::asio::buffer(
            m_bufferIn.getHeaderAddr() + bytesAlreadyRead,
            m_bufferIn.getSendingHeaderSize() - bytesAlreadyRead
        ),
        [this, bytesAlreadyRead](
            const ::std::error_code& errorCode,
            const ::std::size_t length
        ) {
            if (errorCode) {
                if (errorCode == ::asio::error::operation_aborted) {
                    ::std::cerr << "[Connection:UDP] Operation canceled\n";
                } else {
                    ::std::cerr << "[ERROR:UDP] Read header failed: " << errorCode.message() << ".\n";
                    this->close();
                }
            } else if (bytesAlreadyRead + length < m_bufferIn.getSendingHeaderSize()) {
                this->readHeader(bytesAlreadyRead + length);
            } else {
                if (!m_bufferIn.isBodyEmpty()) {
                    m_bufferIn.updateBodySize();
                    this->readBody();
                } else {
                    this->transferBufferToInQueue();
                }
            }
        }
    );
}

template <
    ::detail::isEnum UserMessageType
> void ::network::UdpConnection<UserMessageType>::readBody(
    ::std::size_t bytesAlreadyRead /* = 0 */
)
{
    m_socket.async_receive(
        ::asio::buffer(
            m_bufferIn.getBodyAddr() + bytesAlreadyRead,
            m_bufferIn.getBodySize() - bytesAlreadyRead
        ),
        [this, bytesAlreadyRead](
            const ::std::error_code& errorCode,
            const ::std::size_t length
        ) {
            if (errorCode) {
                if (errorCode == ::asio::error::operation_aborted) {
                    ::std::cerr << "[Connection:UDP] Operation canceled\n";
                } else {
                    ::std::cerr << "[ERROR:UDP] Read body failed: " << errorCode.message() << ".\n";
                    this->close();
                }
            } else if (bytesAlreadyRead + length < m_bufferIn.getBodySize()) {
                this->readBody(bytesAlreadyRead + length);
            } else {
                this->transferBufferToInQueue();
            }
        }
    );
}

template <
    ::detail::isEnum UserMessageType
> void ::network::UdpConnection<UserMessageType>::transferBufferToInQueue()
{
    m_owner.pushIncommingMessage(network::OwnedMessage<UserMessageType>{ m_bufferIn, nullptr });
    this->readHeader();
}
