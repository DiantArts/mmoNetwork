#pragma once

#include <Message.hpp>
#include <OwnedMessage.hpp>
#include <Queue.hpp>



namespace network {



template <
    ::network::detail::IsEnum MessageEnumType
> class Connection
    : public ::std::enable_shared_from_this<Connection<MessageEnumType>>
{

public:

    enum class owner : ::std::uint8_t {
        client,
        server
    };



public:

    // ------------------------------------------------------------------ *structors

    Connection(
        Connection<MessageEnumType>::owner ownerType,
        ::boost::asio::io_context& asioContext,
        ::boost::asio::ip::tcp::socket socket,
        ::network::Queue<::network::OwnedMessage<MessageEnumType>>& messagesIn
    )
        : m_asioContext{ asioContext }
        , m_socket{ ::std::move(socket) }
        , m_messagesIn{ messagesIn }
        , m_ownerType{ ownerType }
    {}

    ~Connection() = default;




    // ------------------------------------------------------------------ connectionHandling

    auto connectToClient(
        ::network::Id id
    )
        -> bool
    {
        if (m_ownerType == Connection<MessageEnumType>::owner::server) {
            if (m_socket.is_open()) {
                m_id = id;
                this->readHeader();
                return true;
            } else {
                ::std::cerr << "[CONNECTION] Invalid socket, connection failed.\n";
                return false;
            }
        } else {
            ::std::cerr << "[CONNECTION] A client cannot connect to another client.\n";
            return false;
        }
    }

    auto connectToServer(
        const ::std::string& host,
        const ::std::uint16_t port
    )
        -> bool
    {
        if (m_ownerType == Connection<MessageEnumType>::owner::client) {
            // resolve host/ip addr into a physical addr
            ::boost::asio::ip::tcp::resolver resolver{ m_asioContext };
            auto endpoints = resolver.resolve(host, ::std::to_string(port));

            ::boost::asio::async_connect(
                m_socket,
                endpoints,
                [this](
                    const boost::system::error_code& errorCode,
                    const ::boost::asio::ip::tcp::endpoint endpoints
                ) {
                    if (!errorCode) {
                        this->readHeader();
                    } else {
                        ::std::cerr << "[CONNECTION] Client failed to connect to the Server.\n";
                    }
                }
            );
            return true;
        } else {
            ::std::cerr << "[CONNECTION] A server cannot connect to another server.\n";
            return false;
        }
    }

    void disconnect()
    {
        if (this->isConnected()) {
            ::boost::asio::post(m_asioContext, [this](){ m_socket.close(); });
        }
    }

    auto isConnected() const
        -> bool
    {
        return m_socket.is_open();
    }



    // ------------------------------------------------------------------ in - async

    void readHeader()
    {
        ::boost::asio::async_read(
            m_socket,
            ::boost::asio::buffer(m_bufferIn.getHeaderAddr(), m_bufferIn.getHeaderSize()),
            [this](
                const boost::system::error_code& errorCode,
                const ::std::size_t length
            ) {
                if (!errorCode) {
                    // m_bufferIn.displayHeader("<-");
                    if (!m_bufferIn.isBodyEmpty()) {
                        m_bufferIn.updateBodySize();
                        this->readBody();
                    } else {
                        this->transferBufferToInQueue();
                    }
                } else {
                    ::std::cerr << "[CONNECTION:" << m_id << "] - Read header failed: "
                        << errorCode.message() << ".\n";
                    m_socket.close();
                    m_id = 0;
                }
            }
        );
    }

    void readBody()
    {
        ::boost::asio::async_read(
            m_socket,
            ::boost::asio::buffer(m_bufferIn.getBodyAddr(), m_bufferIn.getBodySize()),
            [this](
                const boost::system::error_code& errorCode,
                const ::std::size_t length
            ) {
                if (!errorCode) {
                    // m_bufferIn.displayBody("<-");
                    this->transferBufferToInQueue();
                } else {
                    ::std::cerr << "[CONNECTION:" << m_id << "] Read body failed: "
                        << errorCode.message() << ".\n";
                    m_socket.close();
                    m_id = 0;
                }
            }
        );
    }

    void transferBufferToInQueue()
    {
        if (m_ownerType == Connection<MessageEnumType>::owner::server) {
            m_messagesIn.push_back(
                network::OwnedMessage<MessageEnumType>{ this->shared_from_this(), m_bufferIn }
            );
        } else {
            m_messagesIn.push_back(network::OwnedMessage<MessageEnumType>{ nullptr, m_bufferIn });
        }
        this->readHeader();
    }



    // ------------------------------------------------------------------ out - async

    void send(
        ::network::Message<MessageEnumType> message
    )
    {
        ::boost::asio::post(
            m_asioContext,
            [this, message]()
            {
                auto wasOutQueueEmpty{ m_messagesOut.empty() };
                m_messagesOut.push_back(::std::move(message));
                if (wasOutQueueEmpty) {
                    this->writeHeader();
                }
            }
        );
    }

    // TODO: client memory error
    void writeHeader()
    {
        ::boost::asio::async_write(
            m_socket,
            ::boost::asio::buffer(m_messagesOut.front().getHeaderAddr(), m_messagesOut.front().getHeaderSize()),
            [this](
                const boost::system::error_code& errorCode,
                const ::std::size_t length
            ) {
                if (!errorCode) {
                    // m_messagesOut.front().displayHeader("->");
                    if (!m_messagesOut.front().isBodyEmpty()) {
                        this->writeBody();
                    } else {
                        m_messagesOut.remove_front();
                        if (!m_messagesOut.empty()) {
                            this->writeHeader();
                        }
                    }
                } else {
                    ::std::cerr << "[CONNECTION:" << m_id << "] Write header failed: "
                        << errorCode.message() << ".\n";
                    m_socket.close();
                    m_id = 0;
                }
            }
        );
    }

    void writeBody()
    {
        ::boost::asio::async_write(
            m_socket,
            ::boost::asio::buffer(m_messagesOut.front().getBodyAddr(), m_messagesOut.front().getBodySize()),
            [this](
                const boost::system::error_code& errorCode,
                const ::std::size_t length
            ) {
                if (!errorCode) {
                    // m_messagesOut.front().displayBody("->");
                    m_messagesOut.remove_front();
                    if (!m_messagesOut.empty()) {
                        this->writeHeader();
                    }
                } else {
                    ::std::cerr << "[CONNECTION:" << m_id << "] Write body failed: "
                        << errorCode.message() << ".\n";
                    m_socket.close();
                    m_id = 0;
                }
            }
        );
    }



    // ------------------------------------------------------------------ other

    [[ nodiscard ]] auto getId() const
        -> ::network::Id
    {
        return m_id;
    }



private:

    // context shared by the whole asio instance
    ::boost::asio::io_context& m_asioContext;
    ::boost::asio::ip::tcp::socket m_socket;

    // in
    ::network::Queue<::network::OwnedMessage<MessageEnumType>>& m_messagesIn;
    ::network::Message<MessageEnumType> m_bufferIn;

    // out
    ::network::Queue<::network::Message<MessageEnumType>> m_messagesOut;

    // modifies the behavior of the connection
    Connection<MessageEnumType>::owner m_ownerType;

    ::network::Id m_id{ 1 };

};



} // namespace network
