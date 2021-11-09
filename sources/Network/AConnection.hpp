#pragma once

#include <Detail/Id.hpp>
#include <Detail/Queue.hpp>
#include <Detail/Concepts.hpp>
#include <Network/Message.hpp>
#include <Network/OwnedMessage.hpp>

#ifdef ENABLE_ENCRYPTION
#include <Security/Cipher.hpp>
#endif

namespace network { template <::detail::isEnum UserMessageType> class ANode; }



namespace network {



template <
    ::detail::isEnum UserMessageType
> class AConnection {

public:

    // ------------------------------------------------------------------ *structors

    AConnection(
        ::network::ANode<UserMessageType>& owner
    );

    virtual ~AConnection();



protected:

    ::network::ANode<UserMessageType>& m_owner;

    ::network::Message<UserMessageType> m_bufferIn;
    ::detail::Queue<::network::Message<UserMessageType>> m_messagesOut;

#ifdef ENABLE_ENCRYPTION
    ::security::Cipher m_cipher;
#endif // ENABLE_ENCRYPTION

};



} // namespace network

#include <Network/AConnection.impl.hpp>
