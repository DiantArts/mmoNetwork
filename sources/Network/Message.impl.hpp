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
    this->pushAll(::std::forward<decltype(args)>(args)...);
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
    this->pushAll(::std::forward<decltype(args)>(args)...);
}

template <
    ::detail::isEnum UserMessageType
> ::network::Message<UserMessageType>::~Message() = default;



// ------------------------------------------------------------------ push
// Insert any POD-like data into the body

template <
    ::detail::isEnum UserMessageType
> void ::network::Message<UserMessageType>::push(
    const ::detail::isSendableData auto& data
)
{
    // change size and alloc if needed
    m_header.bodySize += sizeof(data);
    m_body.resize(m_header.bodySize);

    // push data into the end of the vector
    ::std::memcpy(m_body.data() + m_header.bodySize - sizeof(data), &data, sizeof(data));
}

template <
    ::detail::isEnum UserMessageType
> void ::network::Message<UserMessageType>::push(
    ::detail::isSendableData auto&& data
)
{
    // change size and alloc if needed
    m_header.bodySize += sizeof(data);
    m_body.resize(m_header.bodySize);

    // push data into the end of the vector
    ::std::memmove(m_body.data() + m_header.bodySize - sizeof(data), &data, sizeof(data));
}

template <
    ::detail::isEnum UserMessageType
> template <
    ::detail::isSendableData Type
> void ::network::Message<UserMessageType>::push(
    ::std::span<Type> data
)
{
    if (data.size() > 0) {
        // change size and alloc if needed
        const ::std::size_t memSize{ sizeof(Type) * data.size() };
        m_header.bodySize += memSize;
        m_body.resize(m_header.bodySize);

        // push data into the end of the vector
        ::std::memmove(m_body.data() + m_header.bodySize - memSize, data.data(), memSize);
    }
    this->push<::std::uint16_t>(data.size());
}

template <
    ::detail::isEnum UserMessageType
> template <
    ::detail::isSendableData Type,
    ::std::size_t size
> void ::network::Message<UserMessageType>::push(
    ::std::span<Type, size> data
)
{
    if (data.size() > 0) {
        // change size and alloc if needed
        const ::std::size_t memSize{ sizeof(Type) * size };
        m_header.bodySize += memSize;
        m_body.resize(m_header.bodySize);

        // push data into the end of the vector
        ::std::memmove(m_body.data() + m_header.bodySize - memSize, data.data(), memSize);
    }
    this->push<::std::uint16_t>(size);
}

template <
    ::detail::isEnum UserMessageType
> void ::network::Message<UserMessageType>::push(
    ::std::span<auto> data
)
{
    if (data.size() > 0) {
        for (auto&& elem : data) {
            this->push(::std::move(elem));
        }
    }
    this->push<::std::uint16_t>(data.size());
}

template <
    ::detail::isEnum UserMessageType
> template <
    ::std::size_t size
> void ::network::Message<UserMessageType>::push(
    ::std::span<auto, size> data
)
{
    if (data.size() > 0) {
        for (auto&& elem : data) {
            this->push(::std::move(elem));
        }
    }
}

template <
    ::detail::isEnum UserMessageType
> void ::network::Message<UserMessageType>::push(
    const ::std::vector<auto>& data
)
{
    auto dataCpy{ data };
    this->push(::std::span{ dataCpy });
}

template <
    ::detail::isEnum UserMessageType
> void ::network::Message<UserMessageType>::push(
    ::std::vector<auto>&& data
)
{
    this->push(::std::span{ data });
}

template <
    ::detail::isEnum UserMessageType
> template <
    typename Type,
    ::std::size_t size
> void ::network::Message<UserMessageType>::push(
    const ::std::array<Type, size>& data
)
{
    auto dataCpy{ data };
    this->push(::std::span{ dataCpy });
}

template <
    ::detail::isEnum UserMessageType
> template <
    typename Type,
    ::std::size_t size
> void ::network::Message<UserMessageType>::push(
    ::std::array<Type, size>&& data
)
{
    this->push(::std::span{ data });
}

template <
    ::detail::isEnum UserMessageType
> void ::network::Message<UserMessageType>::push(
    const ::std::pair<auto, auto>& data
)
{
    this->push(data.first);
    this->push(data.second);
}

template <
    ::detail::isEnum UserMessageType
> void ::network::Message<UserMessageType>::push(
    ::std::pair<auto, auto>&& data
)
{
    this->push(::std::move(data.first));
    this->push(::std::move(data.second));
}

template <
    ::detail::isEnum UserMessageType
> void ::network::Message<UserMessageType>::push(
    const ::std::string& data
)
{
    ::std::string dataCpy{ data };
    this->pushRawMemory(dataCpy.data(), dataCpy.size());
}

template <
    ::detail::isEnum UserMessageType
> void ::network::Message<UserMessageType>::push(
    ::std::string&& data
)
{
    this->push(::std::span{ data.data(), data.data() + data.size() });
}



template <
    ::detail::isEnum UserMessageType
> void ::network::Message<UserMessageType>::pushRawMemory(
    auto* ptrToData,
    const ::std::size_t size
)
{
    this->push(::std::span{ ptrToData, ptrToData + size });
}

template <
    ::detail::isEnum UserMessageType
> void ::network::Message<UserMessageType>::push(
    const char* ptrToData
)
{
    this->push(::std::span{ ptrToData, ptrToData + ::std::strlen(ptrToData) });
}



template <
    ::detail::isEnum UserMessageType
> void ::network::Message<UserMessageType>::pushAll(
    auto&&... args
)
{
    (this->push(::std::forward<decltype(args)>(args)), ...);
}



template <
    ::detail::isEnum UserMessageType
> auto ::network::Message<UserMessageType>::operator<<(
    auto&& data
) -> Message<UserMessageType>&
{
    this->push(::std::forward<decltype(data)>(data));
    return *this;
}

template <
    ::detail::isEnum UserMessageType
> auto ::network::Message<UserMessageType>::operator<<(
    const auto& data
) -> Message<UserMessageType>&
{
    decltype(data) copiedData{ data };
    this->push(::std::move(copiedData));
    return *this;
}



// ------------------------------------------------------------------ pull
// Extract any POD-like data from the end of the body

template <
    ::detail::isEnum UserMessageType
> void ::network::Message<UserMessageType>::pull(
    ::detail::isSendableData auto& data
)
{
    m_header.bodySize -= sizeof(data);

    // pull data out of the end of the vector
    ::std::memmove(&data, m_body.data() + m_header.bodySize, sizeof(data));

    // resize so he doesn't actually yeet my data
    m_body.resize(m_header.bodySize);
}

template <
    ::detail::isEnum UserMessageType
> template <
    ::detail::isSendableData Type
> void ::network::Message<UserMessageType>::pull(
    ::std::span<Type> data
)
{
    if (data.size() > 0) {
        // change size and alloc if needed
        m_header.bodySize -= sizeof(Type) * data.size();

        // pull data out of the end of the vector
        ::std::memmove(data.data(), m_body.data() + m_header.bodySize, sizeof(Type) * data.size());

        // resize so he doesn't actually yeet my data
        m_body.resize(m_header.bodySize);
    }
}

template <
    ::detail::isEnum UserMessageType
> template <
    ::detail::isSendableData Type,
    ::std::size_t size
> void ::network::Message<UserMessageType>::pull(
    ::std::span<Type, size> data
)
{
    if constexpr (size > 0) {
        // change size and alloc if needed
        m_header.bodySize -= sizeof(Type) * size;

        // pull data out of the end of the vector
        ::std::memmove(data.data(), m_body.data() + m_header.bodySize, sizeof(Type) * size);

        // resize so he doesn't actually yeet my data
        m_body.resize(m_header.bodySize);
    }
}

template <
    ::detail::isEnum UserMessageType
> void ::network::Message<UserMessageType>::pull(
    ::std::span<auto> data
)
{
    if (data.size() > 0) {
        for (auto& elem : data | ::std::views::reverse) {
            this->pull(elem);
        }
    }
}

template <
    ::detail::isEnum UserMessageType
> template <
    ::std::size_t size
> void ::network::Message<UserMessageType>::pull(
    ::std::span<auto, size> data
)
{
    if (data.size() > 0) {
        for (auto& elem : data | ::std::views::reverse) {
            this->pull(elem);
        }
    }
}

template <
    ::detail::isEnum UserMessageType
> void ::network::Message<UserMessageType>::pull(
    ::std::vector<auto>& data
)
{
    data.resize(this->pull<::std::uint16_t>());
    this->pull(::std::span{ data });
}

template <
    ::detail::isEnum UserMessageType
> template <
    typename Type,
    ::std::size_t size
> void ::network::Message<UserMessageType>::pull(
    ::std::array<Type, size>& data
)
{
    this->pull(::std::span{ data });
}

template <
    ::detail::isEnum UserMessageType
> void ::network::Message<UserMessageType>::pull(
    ::std::pair<auto, auto>& data
)
{
    this->pull(data.second);
    this->pull(data.first);
}

template <
    ::detail::isEnum UserMessageType
> void ::network::Message<UserMessageType>::pull(
    ::std::string& data
)
{
    auto size{ this->pull<::std::uint16_t>() };
    data.assign(reinterpret_cast<char*>(this->pullRawMemory(size).data()), size);
}



template <
    ::detail::isEnum UserMessageType
> template <
    typename DataType
> auto ::network::Message<UserMessageType>::pull()
    -> DataType
{
    DataType data;
    this->pull(data);
    return data;
}



template <
    ::detail::isEnum UserMessageType
> void ::network::Message<UserMessageType>::pullRawMemory(
    ::std::vector<::std::byte>& refToData,
    const ::std::size_t size
)
{
    refToData.resize(size);
    if (size > 0) {
        m_header.bodySize -= size;

        // pull data out of the end of the vector
        ::std::memmove(refToData.data(), m_body.data() + m_header.bodySize, size);

        // resize so he doesn't actually yeet my data
        m_body.resize(m_header.bodySize);
    }
}

template <
    ::detail::isEnum UserMessageType
> auto ::network::Message<UserMessageType>::pullRawMemory(
    const ::std::size_t size
) -> ::std::vector<::std::byte>
{
    ::std::vector<::std::byte> data;
    this->pullRawMemory(data, size);
    return data;
}



template <
    ::detail::isEnum UserMessageType
> auto ::network::Message<UserMessageType>::operator>>(
    auto& data
) -> Message<UserMessageType>&
{
    this->pull(data);
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
