#pragma once

#include <Detail/Concepts.hpp>
#include <Detail/Queue.hpp>
#include <Network/Message.hpp>
#include <Network/OwnedMessage.hpp>

#ifdef ENABLE_ENCRYPTION
#include <Security/Cipher.hpp>
#endif

namespace network { template <::detail::constraint::isEnum UserMessageType> class ANode; }
namespace network { template <::detail::constraint::isEnum UserMessageType> class Connection; }



namespace network::tcp {


////////////////////////////////////////////////////////////
/// \brief A TCP Connection using boost asio.
///
/// \include Connection.hpp <Network/Tcp/Connection.hpp>
///
/// ::network::tcp::Connection is one component of the ::network::Connection
/// class. It cannot be used outside of this class.
/// Its purpuse is to simplifie communications between two TCP endpoints.
/// It shares a lot of similarities with the class ::network::udp::Connection
/// They both use ::network::Message to communicate.
///
/// To use this class, the owner must call assignConnection() aswell as
/// one of the following methods:
///     \arg \c startConnectingToClient()
///     \arg \c startConnectingToServer()
///
/// depending on what they plan to connect to.
/// The call must match the socket sent to the constructor.
///
/// The owner of this class must have the following members:
///     \arg \c ::network::ANode<UserMessageType>& m_owner;
///     \arg \c ::security::Cipher m_cipher;
///     \arg \c ::network::Informations informations;
///     \arg \c ::network::udp::Connection<UserMessageType> udp;
///     \arg \c ::network::tcp::Connection<UserMessageType> tcp;
///
/// The owner of the class must also possess a disconnect() method.
///
/// The m_owner, m_cipher and informations must be initialised before the udp
/// member, which mush be initalised before the tcp member.
///
/// Both ::network::udp::Connection and ::network::tcp::Connection must be friend
/// of the owner class.
///
/// The owner class must be initialised as ::std::shared_ptr and inherit from
/// ::std::enable_shared_from_this
///
/// \see ::network::Message, ::network::Connection, ::network::udp::Connection
///
////////////////////////////////////////////////////////////
template <
    ::detail::constraint::isEnum UserMessageType
> class Connection {

public:

    ///////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // *structors
    //
    ///////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////
    /// \brief Construct a new tcp connection.
    ///
    /// This contructor creates basic tcp connection.
    ///
    /// \warning The class is imcomlete with just the contructor called.
    ///          In order to work, the user must call assignConnection().
    ///
    /// Once the class is fully initialized, the user can decide to
    /// either call:
    /// \arg \c startConnectingToClient()
    /// \arg \c startConnectingToServer()
    ///
    /// depending on what they plan to connect to.
    /// The call must match the socket sent to the constructor.
    ///
    /// \param socket The tcp socket from asio, generated beforehand.
    ///
    ////////////////////////////////////////////////////////////
    Connection(
        ::asio::ip::tcp::socket socket
    );

    ////////////////////////////////////////////////////////////
    /// \brief Destructor.
    ///
    /// Calls disconnect before deleting all the resources attached
    /// to the Connection.
    ///
    ////////////////////////////////////////////////////////////
    ~Connection();



    ///////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // async (asio thread) - connection
    //
    ///////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////
    /// \brief Connects to a client.
    ///
    /// Starts an asynchronous connection to a client on the asio thread.
    /// If the connection succeeds, identification() is called to start
    /// the identification process to identify the client.
    /// Calls ::network::ANode::onConnectionDenial() if ::network::ANode::onConnect()
    /// returns False. Calls disconnect() aswell if needed.
    ///
    /// \return False if the owner of the connection is a Client, if
    ///         the connection is already used or if the user returns
    ///         False with ::network::ANode::onConnect().
    ///         True if the identification process starts.
    ///
    /// \see ::network::Connection, ::network::ANode
    ///
    ////////////////////////////////////////////////////////////
    auto startConnectingToClient()
        -> bool;

    ////////////////////////////////////////////////////////////
    /// \brief Connects to a server.
    ///
    /// Starts an asynchronous connection to a server on the asio thread.
    /// If the connection succeeds, identification() is called to start
    /// the identification process to be identified by the server.
    /// Calls ::network::ANode::onConnectionDenial() if ::network::ANode::onConnect()
    /// returns false. Calls disconnect() aswell if needed.
    ///
    /// \throws std::logic_error Connection's owner is a server.
    ///
    /// \param host Address of the target server to connect to.
    /// \param port Port of the target server to connect to.
    ///
    /// \see ::network::Connection, ::network::ANode
    ///
    ////////////////////////////////////////////////////////////
    void startConnectingToServer(
        const ::std::string& host,
        ::std::uint16_t port
    );

    ////////////////////////////////////////////////////////////
    /// \brief Disconnects from the target.
    ///
    /// Cancels every asynchronous actions, close the socket, then calls
    /// notify() and owner's ::network::ANode::onDisconnect().
    /// It also resets the shared pointer to the connection assigned with
    /// assignConnection().
    /// Once disconnect has been called, the connection is unusable and
    /// needs to be destroyed (TODO: reconnect).
    ///
    /// \see ::network::Connection, ::network::ANode
    ///
    ////////////////////////////////////////////////////////////
    void disconnect();

    ////////////////////////////////////////////////////////////
    /// \brief Test the connection's status.
    ///
    /// The connection can be unusable due to disconnect calls, errors
    /// happening on other threads or the target disconnecting.
    ///
    /// \return True if the connection is still usable, false otherwise>
    ///
    ////////////////////////////////////////////////////////////
    [[ nodiscard ]] auto isConnected() const
        -> bool;

    ////////////////////////////////////////////////////////////
    /// \brief Notifies all waiting actions.
    ///
    /// Notifies all waiting actions such as waitNotification() calls.
    ///
    ////////////////////////////////////////////////////////////
    void notify();

    ////////////////////////////////////////////////////////////
    /// \brief Waits a notification.
    ///
    /// Waits a notification such as notify() calls.
    ///
    ////////////////////////////////////////////////////////////
    void waitNotification();



    ///////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // async (asio thread) - outgoing messages
    //
    ///////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////
    /// \brief Constructs and sends a message.
    ///
    /// Constructs any kind of ::network::Message calling its constructor
    /// and perfectly forwarding any argument given as parameter.
    /// Once constructed, the message is asynchronously pushed to the
    /// queue of outgoing messages.
    /// If the queue was empty before the push, it then calls
    /// sendAwaitingMessages().
    ///
    /// \param messageType Kind of message descibed in the template-given
    ///                    struct.
    /// \param args        Arguments perfectly forwarded to the constructor
    ///                    of ::network::Message.
    ///
    ////////////////////////////////////////////////////////////
    void send(
        ::network::Message<UserMessageType>::SystemType messageType,
        auto&&... args
    );

    ////////////////////////////////////////////////////////////
    /// \brief Constructs and sends a message.
    ///
    /// Constructs any kind of ::network::Message calling its constructor
    /// and perfectly forwarding any argument given as parameter.
    /// Once constructed, the message is asynchronously pushed to the
    /// queue of outgoing messages.
    /// If the queue was empty before the push, it then calls
    /// sendAwaitingMessages().
    ///
    /// \param messageType Kind of message descibed in the template-given
    ///                    struct.
    /// \param args        Arguments perfectly forwarded to the constructor
    ///                    of ::network::Message.
    ///
    ////////////////////////////////////////////////////////////
    void send(
        UserMessageType messageType,
        auto&&... args
    );

    ////////////////////////////////////////////////////////////
    /// \brief Sends a message.
    ///
    /// Asynchronously pushes the to the message into the queue of outgoing
    /// messages.
    /// If the queue was empty before the push, it then calls
    /// sendAwaitingMessages().
    ///
    /// \param message Rvalue reference to the message to send.
    ///
    ////////////////////////////////////////////////////////////
    void send(
        const ::network::Message<UserMessageType>& message
    );

    ////////////////////////////////////////////////////////////
    /// \brief Sends a message.
    ///
    /// Asynchronously pushes the to the message into the queue of outgoing
    /// messages.
    /// If the queue was empty before the push, it then calls
    /// sendAwaitingMessages().
    ///
    /// \param message Rvalue reference to the message to send.
    ///
    ////////////////////////////////////////////////////////////
    void send(
        ::network::Message<UserMessageType>&& message
    );

    ////////////////////////////////////////////////////////////
    /// \brief Tests whether there is Awaiting messages to be sent.
    ///
    /// Tests if the outgoing queue of messages is empty. If it is,
    /// it means no messages are awaiting to be sent.
    ///
    /// \return True if messages are awaiting to be sent.
    ///
    ////////////////////////////////////////////////////////////
    auto hasSendingMessagesAwaiting() const
        -> bool;

    ////////////////////////////////////////////////////////////
    /// \brief Sends awaiting messages.
    ///
    /// Asynchronously sends all the messages inside the outgoing queue of
    /// messages by calling sendMessage().
    /// The function recursivly clears the outgoing queue of messages.
    ///
    /// \warning This method assumes at least one message waits to be sent.
    /// Calling this methods without any message waiting leads to
    /// undefined behavior and/or potential segfaults.
    ///
    ////////////////////////////////////////////////////////////
    void sendAwaitingMessages();



    ///////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // async (asio thread) - incomming messages
    //
    ///////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////
    /// \brief Starts receiving messages
    ///
    /// Waits for incomming messages. On receive, the buffer of incomming
    /// messages is filled with the incomming message, it calls
    /// transferBufferToInQueue().
    ///
    ////////////////////////////////////////////////////////////
    void startReceivingMessage();



    ///////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // helpers
    //
    ///////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////
    /// \brief Gets the port.
    ///
    /// The port is choosen by the user when creating the Connection.
    ///
    /// \returns Port used by the socket.
    ///
    ////////////////////////////////////////////////////////////
    [[ nodiscard ]] auto getPort() const
        -> ::std::uint16_t;

    ////////////////////////////////////////////////////////////
    /// \brief Gets the address.
    ///
    /// The adderss is choosen by the user when creating the Connection.
    ///
    /// \returns Address used by the socket.
    ///
    ////////////////////////////////////////////////////////////
    [[ nodiscard ]] auto getAddress() const
        -> ::std::string;

    ////////////////////////////////////////////////////////////
    /// \brief Assigns a connection.
    ///
    /// The assigned connection is supposed to be the class that owns
    /// both the ::network::tcp::Connection and the ::network::udp::Connection.
    /// The current class is owned as following :
    /// \code
    /// m_connection->tcp
    /// \endcode
    /// \warning Not calling this function correctly (or not at all) leads to
    ///         undefined behavior.
    ///
    /// \note The constructor must call ::network::Connection::getPtr() that uses
    ///       shared_from_this(). Therefore, ::network::Connection's constructor
    ///       cannot call assignConnection().
    ///
    /// \see ::network::Connection
    ///
    ////////////////////////////////////////////////////////////
    void assignConnection(
        ::std::shared_ptr<::network::Connection<UserMessageType>> connection
    );


private:

    ///////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // async (asio thread) - outgoing messages
    //
    ///////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////
    /// \brief Sends a message.
    ///
    // Asynchronously sends a messages on the asio thread.
    // once sent, it calls the callback given as template parameter
    // In case of errors, Both Udp and Tcp connection are closed.
    //
    // \tparam successCallback Function, lambda or class called on
    //                         success.
    //
    // \param message Message to send.
    // \param args    Argument to pass to the successCallback
    ///
    ////////////////////////////////////////////////////////////
    template <
        auto successCallback
    > void sendMessage(
        ::network::Message<UserMessageType> message,
        auto&&... args
    );

    ////////////////////////////////////////////////////////////
    /// \brief Sends a message in the queue.
    ///
    // Same as sendMessage(), but does not copy the message, but
    // sends the message in front of the queue.
    // Once sent, the message is deleted
    //
    // \tparam successCallback Function, lambda or class called on
    //                         success.
    //
    // \param args    Argument to pass to the successCallback
    ///
    ////////////////////////////////////////////////////////////
    template <
        auto successCallback
    > void sendQueueMessage(
        auto&&... args
    );



    ///////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // async (asio thread) - incomming messages
    //
    ///////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////
    /// \brief Receives a message.
    ///
    // Asynchronously wait on the asio thread for a messages to arrive.
    // once a message arrived, it's tranfered to the incomming buffer
    // and the successCallback is called.
    // In case of errors, Both Udp and Tcp connection are closed.
    //
    // \tparam successCallback Function, lambda or class called on
    //                         success.
    //
    // \param message Message to send.
    // \param args    Argument to pass to the successCallback
    ///
    ////////////////////////////////////////////////////////////
    template <
        auto successCallback
    > void receiveMessage(
        auto&&... args
    );

    ////////////////////////////////////////////////////////////
    /// \brief Push incomming mesages to the incomming queue messages.
    ///
    // Convert the ::network::Message to ::network::OwnedMessage and
    // and assign the OwnedMessage's remote as ::network::Connection::getPtr().
    // Once the ::network::OwnedMessage is created, it is just pushed to the
    // queue of incomming messages that can later be pulled by the Owner.
    //
    // \see ::network::ANode
    ///
    ////////////////////////////////////////////////////////////
    void transferBufferToInQueue();



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

    ////////////////////////////////////////////////////////////
    /// \brief Starts the identification process.
    ///
    // Both client and server share their public key.
    //
    // Once done, calls either serverHandshake() or clientHandshake()
    // depending on the Owner of the connection.
    // if ENABLE_ENCRYPTION is turned to false, all the encryption
    // process is skipped and either serverAcceptIdentification() or
    // clientWaitIdentificationAcceptation() is direcly used.
    //
    // \see ::network::ANode
    ///
    ////////////////////////////////////////////////////////////
    void identification();



#ifdef ENABLE_ENCRYPTION

    ////////////////////////////////////////////////////////////
    /// \brief Perform a handshake with the client.
    ///
    // Uses ::security::Cipher::generateRandomData() to create a
    // random base data, encrypts it end send it to the client.
    // Once the data have been sent, it uses ::security::Cipher::scramble
    // on the data before calling receiveMessage().
    // Once the Client response arrives, the server decrypt the data before
    // comparing with his scrambled data.
    // If it matches, and ::network::ANode::onIdentification() call returns
    // true aswell, serverAcceptIdentification is called.
    // Else, it calls ::network::ANode::onIdentificationDenial,
    // sendIdentificationDenial(), and then ::network::Connection::disconnect().
    //
    // \see ::security::Cipher
    ///
    ////////////////////////////////////////////////////////////
    void serverHandshake();



    ////////////////////////////////////////////////////////////
    /// \brief Perform a handshake with the server.
    ///
    // Receives the encrypted data sent by the server, decrypts it,
    // Scrambles it and encrypts it back to send it to the server. If
    // ::network::ANode::onIdentification() call returns true,
    // clientWaitIdentificationAcceptation() is called. Else,
    // ::network::Connection::disconnect() is called
    //
    // \see ::security::Cipher
    ///
    ////////////////////////////////////////////////////////////
    void clientHandshake();



    ////////////////////////////////////////////////////////////
    /// \brief Sends an identification denial to the target.
    ///
    ////////////////////////////////////////////////////////////
    void sendIdentificationDenial();

#endif // ENABLE_ENCRYPTION



    ////////////////////////////////////////////////////////////
    /// \brief Accept the identification of the client and start the
    /// authentification
    ///
    // Sends the ID of the connection to the client, accepting its
    // identification.
    // Calls serverAuthentification()
    ///
    ////////////////////////////////////////////////////////////
    void serverAcceptIdentification();

    ////////////////////////////////////////////////////////////
    /// \brief If accepted, receives an ID and start the authentification
    ///
    // If the identification is successful, the client is assigned an ID, then
    // calls clientAuthentification(). Otherwise, it calls
    // ::network::Connection::disconnect().
    ///
    ////////////////////////////////////////////////////////////
    void clientWaitIdentificationAcceptation();



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

    ////////////////////////////////////////////////////////////
    /// \brief Performs the authentification process with the client
    ///
    // asynchronously wait to receive the authentification request
    // of the client supposed to contained a known (TODO) and valid
    // username and password.
    // if the message type is incorrect, it calls ::network::Connection::disconnect().
    // Else it sets the user name and
    // then ::network::ANode::onAuthentification. If it returns false,
    // calls sendAuthentificationDenial() before recursivly call itself
    // till a success (TODO: limit or something).
    // Else, it sends an authentificationAccepted before allowing the server
    // to send messages, calling ::network::ANode::onConnectionValidated()
    // and notify().
    ///
    ////////////////////////////////////////////////////////////
    void serverAuthentification();



    ////////////////////////////////////////////////////////////
    /// \brief Performs the authentification process with the client
    ///
    // Calls ::network::ANode::onAuthentification before asynchronously
    // send the authentification request with a user name and a password.
    // If the answer received is an authentification denied, it calls
    // ::network::ANode::onAuthentificationDenial and recursivly calls
    // clientAuthentification() again.
    // If the message type is not authentification accepted, it disconnects
    // the client, else, is allows the client to send messages, calls
    // ::network::ANode::onConnectionValidated() and notify().
    ///
    ////////////////////////////////////////////////////////////
    void clientAuthentification();



    ////////////////////////////////////////////////////////////
    /// \brief Sends an authentification denial to the target.
    ///
    ////////////////////////////////////////////////////////////
    void sendAuthentificationDenial();



    ///////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // async (asio thread) - set up UDP connection
    //
    ///////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////
    /// \brief Sends and receives UDP informations with the target
    ///
    ////////////////////////////////////////////////////////////
    void setupUdp();



private:

    ::std::shared_ptr<::network::Connection<UserMessageType>> m_connection;

    ::asio::ip::tcp::socket m_socket;
    ::network::Message<UserMessageType> m_bufferIn;
    ::detail::Queue<::network::Message<UserMessageType>> m_messagesOut;

    bool m_isSendAllowed{ false };

    ::std::mutex m_mutex;
    ::std::condition_variable m_blocker;

};



} // namespace network::tcp

#include <Network/Tcp/Connection.impl.hpp>
