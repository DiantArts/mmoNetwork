#pragma once



// ------------------------------------------------------------------ *structors

template <
    ::detail::isEnum MessageType
> ::network::Message<MessageType>::Message(
    MessageType&& messageType,
    auto&&... args
)
    : m_header{
        .packetType = ::std::forward<decltype(messageType)>(messageType)
    }
{
    this->insertAll(::std::forward<decltype(args)>(args)...);
}



// ------------------------------------------------------------------ insert
// Insert any POD-like data into the body

template <
    ::detail::isEnum MessageType
> void ::network::Message<MessageType>::insert(
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
    ::detail::isEnum MessageType
> void ::network::Message<MessageType>::insert(
    const ::std::span<auto> data
)
{
    // change size and alloc if needed
    m_header.bodySize += data.size();
    m_body.resize(m_header.bodySize);

    // insert data into the end of the vector
    ::std::memmove(m_body.data() + m_header.bodySize - data.size(), data.data(), data.size());
}

template <
    ::detail::isEnum MessageType
> void ::network::Message<MessageType>::insertRawData(
    auto* ptrToData,
    const ::std::size_t size
)
{
    this->insert(::std::span{ ptrToData, ptrToData + size });
}



template <
    ::detail::isEnum MessageType
> void ::network::Message<MessageType>::insertAll(
    auto&&... args
)
{
    (this->insert(::std::forward<decltype(args)>(args)), ...);
}



template <
    ::detail::isEnum MessageType
> auto ::network::Message<MessageType>::operator<<(
    auto&& data
) -> Message<MessageType>&
{
    this->insert(::std::forward<decltype(data)>(data));
    return *this;
}

template <
    ::detail::isEnum MessageType
> auto ::network::Message<MessageType>::operator<<(
    const auto& data
) -> Message<MessageType>&
{
    decltype(data) copiedData{ data };
    this->insert(::std::move(copiedData));
    return *this;
}



// ------------------------------------------------------------------ extract
// Extract any POD-like data from the end of the body

template <
    ::detail::isEnum MessageType
> void ::network::Message<MessageType>::extract(
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
    ::detail::isEnum MessageType
> void ::network::Message<MessageType>::extract(
    ::std::string& data
)
{
    auto size{ this->extract<::std::uint16_t>() };
    auto dataReceived{ this->extractRawMemory(size) };
    data.assign(reinterpret_cast<char*>(dataReceived.data()), size);
}

template <
    ::detail::isEnum MessageType
> template <
    typename DataType
> auto ::network::Message<MessageType>::extract()
    -> DataType
{
    DataType data;
    this->extract(data);
    return data;
}

template <
    ::detail::isEnum MessageType
> auto ::network::Message<MessageType>::operator>>(
    auto& data
) -> Message<MessageType>&
{
    this->extract(data);
    return *this;
}



// ------------------------------------------------------------------ informations

template <
    ::detail::isEnum MessageType
> constexpr auto ::network::Message<MessageType>::getHeaderSize()
    -> ::std::size_t
{
    return sizeof(Message<MessageType>::Header);
}

template <
    ::detail::isEnum MessageType
> template <
    typename... Types
> [[ nodiscard ]] auto ::network::Message<MessageType>::hasEnoughSizeFor() const
    -> bool
{
    ::std::size_t size{ (sizeof(Types) + ...) };
    return m_header.bodySize >= size;
}
