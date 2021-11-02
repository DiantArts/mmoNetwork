#pragma once



// ------------------------------------------------------------------ *structors

template <
    ::detail::isEnum UserMessageType
> ::network::Message<UserMessageType>::Message() = default;

template <
    typename UserMessageType
> ::network::Message<UserMessageType>::Message(
    Message<UserMessageType>::SystemType messageType,
    auto&&... args
)
    : m_header{
        .packetType = static_cast<::std::uint16_t>(messageType)
    }
{
    this->insertAll(::std::forward<decltype(args)>(args)...);
}

template <
    ::detail::isEnum UserMessageType
> ::network::Message<UserMessageType>::Message(
    UserMessageType messageType,
    auto&&... args
)
    : m_header{
        .packetType = static_cast<::std::uint16_t>(messageType)
    }
{
    this->insertAll(::std::forward<decltype(args)>(args)...);
}

template <
    ::detail::isEnum UserMessageType
> ::network::Message<UserMessageType>::~Message() = default;



// ------------------------------------------------------------------ insert
// Insert any POD-like data into the body

template <
    ::detail::isEnum UserMessageType
> void ::network::Message<UserMessageType>::insert(
    const ::detail::isSendableData auto& data
)
{
    // change size and alloc if needed
    m_header.bodySize += sizeof(data);
    m_body.resize(m_header.bodySize);

    // insert data into the end of the vector
    ::std::memcpy(m_body.data() + m_header.bodySize - sizeof(data), &data, sizeof(data));
}

template <
    ::detail::isEnum UserMessageType
> void ::network::Message<UserMessageType>::insert(
    ::detail::isSendableData auto&& data
)
{
    // change size and alloc if needed
    m_header.bodySize += sizeof(data);
    m_body.resize(m_header.bodySize);

    // insert data into the end of the vector
    ::std::memmove(m_body.data() + m_header.bodySize - sizeof(data), &data, sizeof(data));
}

template <
    ::detail::isEnum UserMessageType
> void ::network::Message<UserMessageType>::insert(
    const ::std::span<auto>& data
)
{
    // change size and alloc if needed
    m_header.bodySize += data.size();
    m_body.resize(m_header.bodySize);

    // insert data into the end of the vector
    ::std::memmove(m_body.data() + m_header.bodySize - data.size(), data.data(), data.size());
}

template <
    ::detail::isEnum UserMessageType
> void ::network::Message<UserMessageType>::insert(
    const ::std::pair<auto, auto>& data
)
{
    this->insert(data.first);
    this->insert(data.second);
}

template <
    ::detail::isEnum UserMessageType
> void ::network::Message<UserMessageType>::insert(
    const ::std::vector<auto>& data
)
{
    for (const auto& subdata : data | std::views::reverse) {
        this->insert(subdata);
    }
    this->insert<::std::uint16_t>(data.size());
}

template <
    ::detail::isEnum UserMessageType
> void ::network::Message<UserMessageType>::insertRawMemory(
    auto* ptrToData,
    const ::std::size_t size
)
{
    this->insert(::std::span{ ptrToData, ptrToData + size });
}

template <
    ::detail::isEnum UserMessageType
> void ::network::Message<UserMessageType>::insert(
    const ::std::string& data
)
{
    this->insertRawMemory(data.data(), data.size());
    this->insert<::std::uint16_t>(data.size());
}

template <
    ::detail::isEnum UserMessageType
> void ::network::Message<UserMessageType>::insert(
    const ::std::string_view data
)
{
    this->insertRawMemory(data.data(), data.size());
    this->insert<::std::uint16_t>(data.size());
}

template <
    ::detail::isEnum UserMessageType
> void ::network::Message<UserMessageType>::insert(
    const char* ptrToData
)
{
    this->insertRawMemory(ptrToData, ::std::strlen(ptrToData));
    this->insert<::std::uint16_t>(::std::strlen(ptrToData));
}



template <
    ::detail::isEnum UserMessageType
> void ::network::Message<UserMessageType>::insertAll(
    auto&&... args
)
{
    (this->insert(::std::forward<decltype(args)>(args)), ...);
}



template <
    ::detail::isEnum UserMessageType
> auto ::network::Message<UserMessageType>::operator<<(
    auto&& data
) -> Message<UserMessageType>&
{
    this->insert(::std::forward<decltype(data)>(data));
    return *this;
}

template <
    ::detail::isEnum UserMessageType
> auto ::network::Message<UserMessageType>::operator<<(
    const auto& data
) -> Message<UserMessageType>&
{
    decltype(data) copiedData{ data };
    this->insert(::std::move(copiedData));
    return *this;
}



// ------------------------------------------------------------------ extract
// Extract any POD-like data from the end of the body

template <
    ::detail::isEnum UserMessageType
> void ::network::Message<UserMessageType>::extractRawMemory(
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
    ::detail::isEnum UserMessageType
> auto ::network::Message<UserMessageType>::extractRawMemory(
    const ::std::size_t size
) -> ::std::vector<::std::byte>
{
    ::std::vector<::std::byte> data;
    this->extractRawMemory(data, size);
    return data;
}

template <
    ::detail::isEnum UserMessageType
> void ::network::Message<UserMessageType>::extract(
    ::detail::isSendableData auto& data
)
{
    m_header.bodySize -= sizeof(data);

    // extract data out of the end of the vector
    ::std::memmove(&data, m_body.data() + m_header.bodySize, sizeof(data));

    // resize so he doesn't actually yeet my data
    m_body.resize(m_header.bodySize);
}


template <
    ::detail::isEnum UserMessageType
> void ::network::Message<UserMessageType>::extract(
    ::std::vector<auto>& data
)
{
    data.resize(this->extract<::std::uint16_t>());
    for (auto& subdata : data) {
        this->extract(subdata);
    }
}

template <
    ::detail::isEnum UserMessageType
> void ::network::Message<UserMessageType>::extract(
    ::std::pair<auto, auto>& data
)
{
    this->extract(data.second);
    this->extract(data.first);
}

template <
    ::detail::isEnum UserMessageType
> void ::network::Message<UserMessageType>::extract(
    ::std::string& data
)
{
    auto size{ this->extract<::std::uint16_t>() };
    auto dataReceived{ this->extractRawMemory(size) };
    data.assign(reinterpret_cast<char*>(dataReceived.data()), size);
}

template <
    ::detail::isEnum UserMessageType
> template <
    typename DataType
> auto ::network::Message<UserMessageType>::extract()
    -> DataType
{
    DataType data;
    this->extract(data);
    return data;
}

template <
    ::detail::isEnum UserMessageType
> auto ::network::Message<UserMessageType>::operator>>(
    auto& data
) -> Message<UserMessageType>&
{
    this->extract(data);
    return *this;
}



// ------------------------------------------------------------------ informations

template <
    ::detail::isEnum UserMessageType
> constexpr auto ::network::Message<UserMessageType>::getHeaderSize()
    -> ::std::size_t
{
    return sizeof(Message<UserMessageType>::Header);
}

template <
    ::detail::isEnum UserMessageType
> constexpr auto ::network::Message<UserMessageType>::getSendingHeaderSize()
    -> ::std::size_t
{
    return sizeof(Message<UserMessageType>::Header) - sizeof(Message<UserMessageType>::TransmissionProtocol);
}

template <
    ::detail::isEnum UserMessageType
> auto ::network::Message<UserMessageType>::getBodySize() const
    -> ::std::size_t
{
    return m_header.bodySize;
}

template <
    ::detail::isEnum UserMessageType
> auto ::network::Message<UserMessageType>::getSize() const
    -> ::std::size_t
{
    return this->getHeaderSize() + this->getBodySize();
}

template <
    ::detail::isEnum UserMessageType
> auto ::network::Message<UserMessageType>::isBodyEmpty() const
    -> bool
{
    return m_header.bodySize == 0;
}

template <
    ::detail::isEnum UserMessageType
> auto ::network::Message<UserMessageType>::getHeaderAddr()
    -> void*
{
    return &m_header;
}

template <
    ::detail::isEnum UserMessageType
> auto ::network::Message<UserMessageType>::getType() const
    -> UserMessageType
{
    return static_cast<UserMessageType>(m_header.packetType);
}

template <
    ::detail::isEnum UserMessageType
> auto ::network::Message<UserMessageType>::getTypeAsSystemType() const
    -> Message<UserMessageType>::SystemType
{
    return static_cast<Message<UserMessageType>::SystemType>(m_header.packetType);
}

template <
    ::detail::isEnum UserMessageType
> void ::network::Message<UserMessageType>::setType(
    UserMessageType type
)
{
    m_header.packetType = static_cast<::std::uint16_t>(type);
}

template <
    ::detail::isEnum UserMessageType
> auto ::network::Message<UserMessageType>::getTransmissionProtocol()
    -> Message<UserMessageType>::TransmissionProtocol
{
    return m_header.transmissionProtocol;
}

template <
    ::detail::isEnum UserMessageType
> void ::network::Message<UserMessageType>::setTransmissionProtocol(
    Message<UserMessageType>::TransmissionProtocol protocol
)
{
    m_header.transmissionProtocol = protocol;
}



// ------------------------------------------------------------------ bodyManipulation

template <
    ::detail::isEnum UserMessageType
> void ::network::Message<UserMessageType>::updateBodySize()
{
    m_body.resize(m_header.bodySize);
}

template <
    ::detail::isEnum UserMessageType
> void ::network::Message<UserMessageType>::resize(
    ::std::size_t newSize
)
{
    m_body.resize(newSize);
}

template <
    ::detail::isEnum UserMessageType
> auto ::network::Message<UserMessageType>::getBodyAddr()
    -> void*
{
    return m_body.data();
}



// ------------------------------------------------------------------ debug

template <
    ::detail::isEnum UserMessageType
> void ::network::Message<UserMessageType>::displayHeader(
    const char direction[2]
) const
{
    ::std::cout << direction << ' '
        << (int)m_header.packetType << ' '
        << (int)m_header.bodySize << '\n';
}

template <
    ::detail::isEnum UserMessageType
> void ::network::Message<UserMessageType>::displayBody(
    const char direction[2]
) const
{
    ::std::cout << direction << " [body]\n";
    ::std::cout.write((char*)m_body.data(), m_header.bodySize);
}
