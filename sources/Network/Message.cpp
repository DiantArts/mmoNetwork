#include <pch.hpp>
#include <Network/Message.hpp>
#include <Network/MessageType.hpp>



// ------------------------------------------------------------------ explicit instantiations

template class ::network::Message<::network::MessageType>;



// ------------------------------------------------------------------ *structors

template <
    ::detail::isEnum MessageType
> ::network::Message<MessageType>::Message() = default;

template <
    ::detail::isEnum MessageType
> ::network::Message<MessageType>::Message(
    MessageType&& messageType
)
    : m_header{
        .packetType = ::std::forward<decltype(messageType)>(messageType)
    }
{}

template <
    ::detail::isEnum MessageType
> ::network::Message<MessageType>::~Message() = default;



// ------------------------------------------------------------------ insert
// Insert any POD-like data into the body

template <
    ::detail::isEnum MessageType
> void ::network::Message<MessageType>::insert(
    const ::std::string& data
)
{
    this->insertRawData(data.data(), data.size());
    this->insert<::std::uint16_t>(data.size());
}

template <
    ::detail::isEnum MessageType
> void ::network::Message<MessageType>::insert(
    const ::std::string_view data
)
{
    this->insertRawData(data.data(), data.size());
    this->insert<::std::uint16_t>(data.size());
}

template <
    ::detail::isEnum MessageType
> void ::network::Message<MessageType>::insert(
    const char* ptrToData
)
{
    this->insertRawData(ptrToData, ::std::strlen(ptrToData));
    this->insert<::std::uint16_t>(::std::strlen(ptrToData));
}



// ------------------------------------------------------------------ extract
// Extract any POD-like data from the end of the body

template <
    ::detail::isEnum MessageType
> void ::network::Message<MessageType>::extractRawMemory(
    ::std::vector<::std::byte>& refToData,
    const ::std::size_t size
)
{
    refToData.resize(size);

    m_header.bodySize -= size;

    // extract data out of the end of the vector
    ::std::memmove(refToData.data(), m_body.data() + m_header.bodySize, size);

    // resize so he doesn't actually yeet my data
    m_body.resize(m_header.bodySize);
}

template <
    ::detail::isEnum MessageType
> auto ::network::Message<MessageType>::extractRawMemory(
    const ::std::size_t size
) -> ::std::vector<::std::byte>
{
    ::std::vector<::std::byte> data;
    this->extractRawMemory(data, size);
    return data;
}



// ------------------------------------------------------------------ informations


template <
    ::detail::isEnum MessageType
> auto ::network::Message<MessageType>::getBodySize() const
    -> ::std::size_t
{
    return m_header.bodySize;
}

template <
    ::detail::isEnum MessageType
> auto ::network::Message<MessageType>::getSize() const
    -> ::std::size_t
{
    return this->getHeaderSize() + this->getBodySize();
}

template <
    ::detail::isEnum MessageType
> void ::network::Message<MessageType>::setTransmissionProtocol(
    ::network::TransmissionProtocol protocol
)
{
    m_header.transmissionProtocol = protocol;
}

template <
    ::detail::isEnum MessageType
> auto ::network::Message<MessageType>::getTransmissionProtocol() const
    -> ::network::TransmissionProtocol
{
    return m_header.transmissionProtocol;
}

template <
    ::detail::isEnum MessageType
> auto ::network::Message<MessageType>::isBodyEmpty() const
    -> bool
{
    return m_header.bodySize == 0;
}



// ------------------------------------------------------------------ header

template <
    ::detail::isEnum MessageType
> auto ::network::Message<MessageType>::getHeaderAddr()
    -> void*
{
    return &m_header;
}

template <
    ::detail::isEnum MessageType
> auto ::network::Message<MessageType>::getType() const
    -> MessageType
{
    return m_header.packetType;
}

template <
    ::detail::isEnum MessageType
> void ::network::Message<MessageType>::setType(
    MessageType type
)
{
    m_header.packetType = type;
}



// ------------------------------------------------------------------ bodyManipulation

template <
    ::detail::isEnum MessageType
> void ::network::Message<MessageType>::updateBodySize()
{
    m_body.resize(m_header.bodySize);
}

template <
    ::detail::isEnum MessageType
> void ::network::Message<MessageType>::resize(
    ::std::size_t newSize
)
{
    m_body.resize(newSize);
}

template <
    ::detail::isEnum MessageType
> auto ::network::Message<MessageType>::getBodyAddr()
    -> void*
{
    return m_body.data();
}



// ------------------------------------------------------------------ debug

template <
    ::detail::isEnum MessageType
> void ::network::Message<MessageType>::displayHeader(
    const char direction[2]
) const
{
    ::std::cout << direction << ' '
        << (int)m_header.packetType << ' '
        << (int)m_header.bodySize << '\n';
}

template <
    ::detail::isEnum MessageType
> void ::network::Message<MessageType>::displayBody(
    const char direction[2]
) const
{
    ::std::cout << direction << " [body]\n";
    ::std::cout.write((char*)m_body.data(), m_header.bodySize);
}
