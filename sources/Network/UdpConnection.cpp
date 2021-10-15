#include <pch.hpp>
#include <Network/UdpConnection.hpp>
#include <Network/Client/AClient.hpp>


// ------------------------------------------------------------------ explicit instantiations

template class ::network::UdpConnection<::network::MessageType>;



// ------------------------------------------------------------------ *structors

template <
    ::detail::isEnum MessageType
> ::network::UdpConnection<MessageType>::UdpConnection(
    ::network::AClient<MessageType>& owner
)
    : m_owner{ owner }
    , m_socket{ owner.getAsioContext() }
{
    m_socket.open(::boost::asio::ip::udp::v4());
    m_socket.bind(boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 0));
}


template <
    ::detail::isEnum MessageType
> ::network::UdpConnection<MessageType>::~UdpConnection()
{
    if (m_socket.is_open()) {
        m_socket.cancel();
        m_socket.close();
        ::std::cout << "[Connection:UDP] Disconnected.\n";
    }
}



// ------------------------------------------------------------------ async - connection

template <
    ::detail::isEnum MessageType
> void ::network::UdpConnection<MessageType>::target(
    const ::std::string& host,
    const ::std::uint16_t port
)
{
    m_socket.async_connect(
        ::boost::asio::ip::udp::endpoint{ ::boost::asio::ip::address::from_string(host), port },
        [this](
            const boost::system::error_code& errorCode
        ) {
            if (errorCode) {
                if (errorCode == ::boost::asio::error::operation_aborted) {
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
    ::detail::isEnum MessageType
> void ::network::UdpConnection<MessageType>::close()
{
    if (m_socket.is_open()) {
        m_socket.cancel();
        m_socket.close();
        m_owner.onUdpDisconnect(this->shared_from_this());
        ::std::cout << "[Connection:UDP] Connection closed.\n";
    }
}

template <
    ::detail::isEnum MessageType
> auto ::network::UdpConnection<MessageType>::isOpen() const
    -> bool
{
    return m_socket.is_open();
}



// ------------------------------------------------------------------ async - out

template <
    ::detail::isEnum MessageType
> void ::network::UdpConnection<MessageType>::send(
    ::network::Message<MessageType>&& message
)
{
    ::boost::asio::post(m_owner.getAsioContext(), ::std::bind_front(
        [this](
            ::network::Message<MessageType> message
        )
        {
            auto wasOutQueueEmpty{ m_messagesOut.empty() };
            message.setTransmissionProtocol(::network::TransmissionProtocol::udp);
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
    ::detail::isEnum MessageType
> auto ::network::UdpConnection<MessageType>::getOwner() const
    -> const ::network::AClient<MessageType>&
{
    return m_owner;
}

template <
    ::detail::isEnum MessageType
> auto ::network::UdpConnection<MessageType>::getPort() const
    -> ::std::uint16_t
{
    return m_socket.local_endpoint().port();
}

template <
    ::detail::isEnum MessageType
> auto ::network::UdpConnection<MessageType>::getAddress() const
    -> ::std::string
{
    return m_socket.local_endpoint().address().to_string();
}



// ------------------------------------------------------------------ async - out

template <
    ::detail::isEnum MessageType
> void ::network::UdpConnection<MessageType>::writeHeader(
    ::std::size_t bytesAlreadySent /* = 0 */
)
{
    m_socket.async_send(
        ::boost::asio::buffer(
            m_messagesOut.front().getHeaderAddr() + bytesAlreadySent,
            m_messagesOut.front().getHeaderSize() - bytesAlreadySent
        ),
        [this, bytesAlreadySent](
            const boost::system::error_code& errorCode,
            const ::std::size_t length
        ) {
            if (errorCode) {
                if (errorCode == ::boost::asio::error::operation_aborted) {
                    ::std::cerr << "[Connection:UDP] Operation canceled\n";
                } else {
                    ::std::cerr << "[ERROR:UDP] Write header failed: " << errorCode.message() << ".\n";
                    this->close();
                }
            } else if (bytesAlreadySent + length < m_messagesOut.front().getHeaderSize()) {
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
    ::detail::isEnum MessageType
> void ::network::UdpConnection<MessageType>::writeBody(
    ::std::size_t bytesAlreadySent /* = 0 */
)
{
    m_socket.async_send(
        ::boost::asio::buffer(
            m_messagesOut.front().getBodyAddr() + bytesAlreadySent,
            m_messagesOut.front().getBodySize() - bytesAlreadySent
        ),
        [this, bytesAlreadySent](
            const boost::system::error_code& errorCode,
            const ::std::size_t length
        ) {
            if (errorCode) {
                if (errorCode == ::boost::asio::error::operation_aborted) {
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
    ::detail::isEnum MessageType
> void ::network::UdpConnection<MessageType>::readHeader(
    ::std::size_t bytesAlreadyRead /* = 0 */
)
{
    m_socket.async_receive(
        ::boost::asio::buffer(
            m_bufferIn.getHeaderAddr() + bytesAlreadyRead,
            m_bufferIn.getHeaderSize() - bytesAlreadyRead
        ),
        [this, bytesAlreadyRead](
            const boost::system::error_code& errorCode,
            const ::std::size_t length
        ) {
            if (errorCode) {
                if (errorCode == ::boost::asio::error::operation_aborted) {
                    ::std::cerr << "[Connection:UDP] Operation canceled\n";
                } else {
                    ::std::cerr << "[ERROR:UDP] Read header failed: " << errorCode.message() << ".\n";
                    this->close();
                }
            } else if (bytesAlreadyRead + length < m_bufferIn.getHeaderSize()) {
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
    ::detail::isEnum MessageType
> void ::network::UdpConnection<MessageType>::readBody(
    ::std::size_t bytesAlreadyRead /* = 0 */
)
{
    m_socket.async_receive(
        ::boost::asio::buffer(
            m_bufferIn.getBodyAddr() + bytesAlreadyRead,
            m_bufferIn.getBodySize() - bytesAlreadyRead
        ),
        [this, bytesAlreadyRead](
            const boost::system::error_code& errorCode,
            const ::std::size_t length
        ) {
            if (errorCode) {
                if (errorCode == ::boost::asio::error::operation_aborted) {
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
    ::detail::isEnum MessageType
> void ::network::UdpConnection<MessageType>::transferBufferToInQueue()
{
    m_owner.pushIncommingMessage(network::OwnedMessage<MessageType>{ m_bufferIn, nullptr });
    this->readHeader();
}
