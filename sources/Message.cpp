#include <pch.hpp>
#include <Message.hpp>
#include <MessageType.hpp>



// ------------------------------------------------------------------ explicit instantiations

template class ::network::Message<::network::MessageType>;



// ------------------------------------------------------------------ *structors

template <
    ::network::detail::IsEnum MessageType
> ::network::Message<MessageType>::Message() = default;

template <
    ::network::detail::IsEnum MessageType
> ::network::Message<MessageType>::~Message() = default;



// ------------------------------------------------------------------ informations


template <
    ::network::detail::IsEnum MessageType
> [[ nodiscard ]] auto ::network::Message<MessageType>::getBodySize() const
    -> ::std::size_t
{
    return m_header.bodySize;
}

template <
    ::network::detail::IsEnum MessageType
> [[ nodiscard ]] auto ::network::Message<MessageType>::getSize() const
    -> ::std::size_t
{
    return this->getHeaderSize() + this->getBodySize();
}

template <
    ::network::detail::IsEnum MessageType
> [[ nodiscard ]] auto ::network::Message<MessageType>::isBodyEmpty() const
    -> bool
{
    return m_header.bodySize == 0;
}



// ------------------------------------------------------------------ header

template <
    ::network::detail::IsEnum MessageType
> auto ::network::Message<MessageType>::getHeaderAddr()
    -> void*
{
    return &m_header;
}

template <
    ::network::detail::IsEnum MessageType
> auto ::network::Message<MessageType>::getType() const
    -> MessageType
{
    return m_header.packetType;
}

template <
    ::network::detail::IsEnum MessageType
> void ::network::Message<MessageType>::setType(
    MessageType type
)
{
    m_header.packetType = type;
}



// ------------------------------------------------------------------ bodyManipulation

template <
    ::network::detail::IsEnum MessageType
> void ::network::Message<MessageType>::updateBodySize()
{
    m_body.resize(m_header.bodySize);
}

template <
    ::network::detail::IsEnum MessageType
> void ::network::Message<MessageType>::resize(
    ::std::size_t newSize
)
{
    m_body.resize(newSize);
}

template <
    ::network::detail::IsEnum MessageType
> auto ::network::Message<MessageType>::getBodyAddr()
    -> void*
{
    return m_body.data();
}



// ------------------------------------------------------------------ debug

template <
    ::network::detail::IsEnum MessageType
> void ::network::Message<MessageType>::displayHeader(
    const char direction[2]
) const
{
    ::std::cout << direction << ' '
        << (int)m_header.packetType << ' '
        << (int)m_header.bodySize << '\n';
}

template <
    ::network::detail::IsEnum MessageType
> void ::network::Message<MessageType>::displayBody(
    const char direction[2]
) const
{
    ::std::cout << direction << " [body]\n";
    // ::std::cout.write((char*)m_body.data(), m_header.bodySize);
}
