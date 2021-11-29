#pragma once



// ------------------------------------------------------------------ push imlementation

void push(
    ::network::Message<auto>& message,
    const ::detail::constraint::isSendableData auto& data
)
{
    // change size and alloc if needed
    message.getHeader().bodySize += sizeof(data);
    message.getBody().resize(message.getHeader().bodySize);

    // push data into the end of the vector
    ::std::memcpy(message.getBody().data() + message.getHeader().bodySize - sizeof(data), &data, sizeof(data));
}

void push(
    ::network::Message<auto>& message,
    ::detail::constraint::isSendableData auto&& data
)
{
    // change size and alloc if needed
    message.getHeader().bodySize += sizeof(data);
    message.getBody().resize(message.getHeader().bodySize);

    // push data into the end of the vector
    ::std::memmove(message.getBody().data() + message.getHeader().bodySize - sizeof(data), &data, sizeof(data));
}



template <
    ::detail::constraint::isEnum UserMessageType,
    ::detail::constraint::isSendableData Type
> void push(
    ::network::Message<UserMessageType>& message,
    ::std::span<Type> data
)
{
    if (data.size() > 0) {
        // change size and alloc if needed
        const ::std::size_t memSize{ sizeof(Type) * data.size() };
        message.getHeader().bodySize += memSize;
        message.getBody().resize(message.getHeader().bodySize);

        // push data into the end of the vector
        ::std::memmove(message.getBody().data() + message.getHeader().bodySize - memSize, data.data(), memSize);
    }
    ::push<UserMessageType, ::std::uint16_t>(message, data.size());
}

template <
    ::detail::constraint::isEnum UserMessageType,
    ::detail::constraint::isNotSendableData Type
> void push(
    ::network::Message<UserMessageType>& message,
    ::std::span<Type> data
)
{
    if (data.size() > 0) {
        for (auto&& elem : data) {
            push(message, ::std::move(elem));
        }
    }
    ::push<UserMessageType, ::std::uint16_t>(message, data.size());
}

template <
    ::detail::constraint::isEnum UserMessageType,
    ::detail::constraint::isSendableData Type,
    ::std::size_t size
> void push(
    ::network::Message<UserMessageType>& message,
    ::std::span<Type, size> data
)
{
    if (data.size() > 0) {
        // change size and alloc if needed
        const ::std::size_t memSize{ sizeof(Type) * size };
        message.getHeader().bodySize += memSize;
        message.getBody().resize(message.getHeader().bodySize);

        // push data into the end of the vector
        ::std::memmove(message.getBody().data() + message.getHeader().bodySize - memSize, data.data(), memSize);
    }
}

template <
    ::detail::constraint::isEnum UserMessageType,
    ::detail::constraint::isNotSendableData Type,
    ::std::size_t size
> void push(
    ::network::Message<UserMessageType>& message,
    ::std::span<Type, size> data
)
{
    if (data.size() > 0) {
        for (auto&& elem : data) {
            ::push(message, ::std::move(elem));
        }
    }
}


void push(
    ::network::Message<auto>& message,
    const ::std::vector<auto>& data
)
{
    auto dataCpy{ data };
    ::push(message, ::std::span{ dataCpy });
}

void push(
    ::network::Message<auto>& message,
    ::std::vector<auto>&& data
)
{
    ::push(message, ::std::span{ data });
}



void push(
    ::network::Message<auto>& message,
    const ::std::map<auto, auto>& data
)
{
    if (data.size() > 0) {
        for (const auto& [key, val] : data) {
            ::push(message, ::std::make_pair(key, val));
        }
    }
    ::push(message, static_cast<::std::uint16_t>(data.size()));
}

void push(
    ::network::Message<auto>& message,
    ::std::map<auto, auto>&& data
)
{
    if (data.size() > 0) {
        for (auto&& [key, val] : data) {
            ::push(message, ::std::make_pair(key, val));
        }
    }
    ::push(message, static_cast<::std::uint16_t>(data.size()));
}



void push(
    ::network::Message<auto>& message,
    const ::std::unordered_map<auto, auto>& data
)
{
    if (data.size() > 0) {
        for (const auto& [key, val] : data) {
            ::push(message, ::std::make_pair(key, val));
        }
    }
    ::push(message, static_cast<::std::uint16_t>(data.size()));
}

void push(
    ::network::Message<auto>& message,
    ::std::unordered_map<auto, auto>&& data
)
{
    if (data.size() > 0) {
        for (auto&& [key, val] : data) {
            ::push(message, ::std::make_pair(key, val));
        }
    }
    ::push(message, static_cast<::std::uint16_t>(data.size()));
}



template <
    ::detail::constraint::isEnum UserMessageType,
    typename Type,
    ::std::size_t size
> void push(
    ::network::Message<UserMessageType>& message,
    const ::std::array<Type, size>& data
)
{
    auto dataCpy{ data };
    ::push(message, ::std::span{ dataCpy });
}

template <
    ::detail::constraint::isEnum UserMessageType,
    typename Type,
    ::std::size_t size
> void push(
    ::network::Message<UserMessageType>& message,
    ::std::array<Type, size>&& data
)
{
    ::push(message, ::std::span{ data });
}


void push(
    ::network::Message<auto>& message,
    const ::std::pair<auto, auto>& data
)
{
    ::push(message, data.first);
    ::push(message, data.second);
}

void push(
    ::network::Message<auto>& message,
    ::std::pair<auto, auto>&& data
)
{
    ::push(message, ::std::move(data.first));
    ::push(message, ::std::move(data.second));
}


template <
    ::detail::constraint::isEnum UserMessageType
> void push(
    ::network::Message<UserMessageType>& message,
    const ::std::string& data
)
{
    ::std::string dataCpy{ data };
    message.pushRawMemory(dataCpy.data(), dataCpy.size());
}

template <
    ::detail::constraint::isEnum UserMessageType
> void push(
    ::network::Message<UserMessageType>& message,
    ::std::string&& data
)
{
    ::push(message, ::std::span{ data.data(), data.data() + data.size() });
}
template <
    ::detail::constraint::isEnum UserMessageType
> void push(
    ::network::Message<UserMessageType>& message,
    const char* ptrToData
)
{
    ::push(message, ::std::span{ ptrToData, ptrToData + ::std::strlen(ptrToData) });
}




// ------------------------------------------------------------------ pull implementation

template <
    ::detail::constraint::isEnum UserMessageType,
    typename Type
> [[ nodiscard ]] auto pull(
    ::network::Message<UserMessageType>& message
) -> Type
{
    Type data;
    ::pull(message, data);
    return data;
}



void pull(
    ::network::Message<auto>& message,
    ::detail::constraint::isSendableData auto& data
)
{
    message.getHeader().bodySize -= sizeof(data);

    // pull data out of the end of the vector
    ::std::memmove(&data, message.getBody().data() + message.getHeader().bodySize, sizeof(data));

    // resize so he doesn't actually yeet my data
    message.getBody().resize(message.getHeader().bodySize);
}



template <
    ::detail::constraint::isEnum UserMessageType,
    ::detail::constraint::isSendableData Type
> void pull(
    ::network::Message<UserMessageType>& message,
    ::std::span<Type> data
)
{
    if (data.size() > 0) {
        // change size and alloc if needed
        message.getHeader().bodySize -= sizeof(Type) * data.size();

        // pull data out of the end of the vector
        ::std::memmove(data.data(), message.getBody().data() + message.getHeader().bodySize, sizeof(Type) * data.size());

        // resize so he doesn't actually yeet my data
        message.getBody().resize(message.getHeader().bodySize);
    }
}

template <
    ::detail::constraint::isEnum UserMessageType,
    ::detail::constraint::isNotSendableData Type
> void pull(
    ::network::Message<UserMessageType>& message,
    ::std::span<Type> data
)
{
    if (data.size() > 0) {
        for (auto& elem : data | ::std::views::reverse) {
            ::pull(message, elem);
        }
    }
}

template <
    ::detail::constraint::isEnum UserMessageType,
    ::detail::constraint::isSendableData Type,
    ::std::size_t size
> void pull(
    ::network::Message<UserMessageType>& message,
    ::std::span<Type, size> data
)
{
    if constexpr (size > 0) {
        // change size and alloc if needed
        message.getHeader().bodySize -= sizeof(Type) * size;

        // pull data out of the end of the vector
        ::std::memmove(data.data(), message.getBody().data() + message.getHeader().bodySize, sizeof(Type) * size);

        // resize so he doesn't actually yeet my data
        message.getBody().resize(message.getHeader().bodySize);
    }
}

template <
    ::detail::constraint::isEnum UserMessageType,
    ::detail::constraint::isNotSendableData Type,
    ::std::size_t size
> void pull(
    ::network::Message<UserMessageType>& message,
    ::std::span<Type, size> data
)
{
    if (data.size() > 0) {
        for (auto& elem : data | ::std::views::reverse) {
            ::pull(message, elem);
        }
    }
}



template <
    ::detail::constraint::isEnum UserMessageType
> void pull(
    ::network::Message<UserMessageType>& message,
    ::std::vector<auto>& data
)
{
    auto oldSize{ data.size() };
    auto pullSize{ ::pull<UserMessageType, ::std::uint16_t>(message) };
    data.resize(oldSize + pullSize);
    ::pull(message, ::std::span{ data.begin() + oldSize, pullSize });
}

template <
    ::detail::constraint::isEnum UserMessageType,
    typename Type1,
    typename Type2
> void pull(
    ::network::Message<UserMessageType>& message,
    ::std::map<Type1, Type2>& data
)
{
    auto size{ ::pull<UserMessageType, ::std::uint16_t>(message) };
    for (::std::uint16_t i{ 0 }; i < size; ++i) {
        data.insert(::pull<UserMessageType, ::std::pair<Type1, Type2>>(message));
    }
}

template <
    ::detail::constraint::isEnum UserMessageType,
    typename Type1,
    typename Type2
> void pull(
    ::network::Message<UserMessageType>& message,
    ::std::unordered_map<Type1, Type2>& data
)
{
    auto size{ ::pull<UserMessageType, ::std::uint16_t>(message) };
    for (::std::uint16_t i{ 0 }; i < size; ++i) {
        data.insert(::pull<UserMessageType, ::std::pair<Type1, Type2>>(message));
    }
}

template <
    ::detail::constraint::isEnum UserMessageType,
    typename Type,
    ::std::size_t size
> void pull(
    ::network::Message<UserMessageType>& message,
    ::std::array<Type, size>& data
)
{
    ::pull(message, ::std::span{ data });
}



void pull(
    ::network::Message<auto>& message,
    ::std::pair<auto, auto>& data
)
{
    ::pull(message, data.second);
    ::pull(message, data.first);
}

template <
    ::detail::constraint::isEnum UserMessageType
> void pull(
    ::network::Message<UserMessageType>& message,
    ::std::string& data
)
{
    auto size{ ::pull<UserMessageType, ::std::uint16_t>(message) };
    data.assign(reinterpret_cast<char*>(message.pullRawMemory(size).data()), size);
}



// ------------------------------------------------------------------ *structors

template <
    ::detail::constraint::isEnum UserMessageType
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
    ::detail::constraint::isEnum UserMessageType
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
    ::detail::constraint::isEnum UserMessageType
> ::network::Message<UserMessageType>::~Message() = default;



// ------------------------------------------------------------------ push

template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::Message<UserMessageType>::push(
    const auto& data
)
{
    ::push(*this, data);
}

template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::Message<UserMessageType>::push(
    auto&& data
)
{
    ::push(*this, ::std::forward<decltype(data)>(data));
}

template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::Message<UserMessageType>::pushRawMemory(
    auto* ptrToData,
    const ::std::size_t size
)
{
    ::push(*this, ::std::span{ ptrToData, ptrToData + size });
}

template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::Message<UserMessageType>::pushAll(
    auto&&... args
)
{
    (::push(*this, ::std::forward<decltype(args)>(args)), ...);
}

template <
    ::detail::constraint::isEnum UserMessageType
> auto ::network::Message<UserMessageType>::operator<<(
    auto&& data
) -> Message<UserMessageType>&
{
    ::push(*this, ::std::forward<decltype(data)>(data));
    return *this;
}

template <
    ::detail::constraint::isEnum UserMessageType
> auto ::network::Message<UserMessageType>::operator<<(
    const auto& data
) -> Message<UserMessageType>&
{
    decltype(data) copiedData{ data };
    ::push(*this, ::std::move(copiedData));
    return *this;
}



// ------------------------------------------------------------------ pull

template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::Message<UserMessageType>::pull(
    auto& data
)
{
    ::pull(*this, data);
}

template <
    ::detail::constraint::isEnum UserMessageType
> template <
    typename DataType
> auto ::network::Message<UserMessageType>::pull()
    -> DataType
{
    DataType data;
    ::pull(*this, data);
    return data;
}

template <
    ::detail::constraint::isEnum UserMessageType
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
    ::detail::constraint::isEnum UserMessageType
> auto ::network::Message<UserMessageType>::pullRawMemory(
    const ::std::size_t size
) -> ::std::vector<::std::byte>
{
    ::std::vector<::std::byte> data;
    this->pullRawMemory(data, size);
    return data;
}

template <
    ::detail::constraint::isEnum UserMessageType
> auto ::network::Message<UserMessageType>::operator>>(
    auto& data
) -> Message<UserMessageType>&
{
    ::pull(*this, data);
    return *this;
}



// ------------------------------------------------------------------ informations

template <
    ::detail::constraint::isEnum UserMessageType
> constexpr auto ::network::Message<UserMessageType>::getHeaderSize()
    -> ::std::size_t
{
    return sizeof(Message<UserMessageType>::Header);
}

template <
    ::detail::constraint::isEnum UserMessageType
> constexpr auto ::network::Message<UserMessageType>::getSendingHeaderSize()
    -> ::std::size_t
{
    return sizeof(Message<UserMessageType>::Header) - sizeof(Message<UserMessageType>::TransmissionProtocol);
}

template <
    ::detail::constraint::isEnum UserMessageType
> auto ::network::Message<UserMessageType>::getBodySize() const
    -> ::std::size_t
{
    return m_header.bodySize;
}

template <
    ::detail::constraint::isEnum UserMessageType
> auto ::network::Message<UserMessageType>::getSize() const
    -> ::std::size_t
{
    return this->getHeaderSize() + this->getBodySize();
}

template <
    ::detail::constraint::isEnum UserMessageType
> auto ::network::Message<UserMessageType>::isBodyEmpty() const
    -> bool
{
    return m_header.bodySize == 0;
}

template <
    ::detail::constraint::isEnum UserMessageType
> auto ::network::Message<UserMessageType>::getHeaderAddr()
    -> void*
{
    return &m_header;
}

template <
    ::detail::constraint::isEnum UserMessageType
> auto ::network::Message<UserMessageType>::getType() const
    -> UserMessageType
{
    return static_cast<UserMessageType>(m_header.packetType);
}

template <
    ::detail::constraint::isEnum UserMessageType
> auto ::network::Message<UserMessageType>::getTypeAsSystemType() const
    -> Message<UserMessageType>::SystemType
{
    return static_cast<Message<UserMessageType>::SystemType>(m_header.packetType);
}

template <
    ::detail::constraint::isEnum UserMessageType
> auto ::network::Message<UserMessageType>::getTypeAsInt() const
    -> ::std::uint16_t
{
    return m_header.packetType;
}

template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::Message<UserMessageType>::setType(
    UserMessageType type
)
{
    m_header.packetType = static_cast<::std::uint16_t>(type);
}

template <
    ::detail::constraint::isEnum UserMessageType
> auto ::network::Message<UserMessageType>::getTransmissionProtocol()
    -> Message<UserMessageType>::TransmissionProtocol
{
    return m_header.transmissionProtocol;
}

template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::Message<UserMessageType>::setTransmissionProtocol(
    Message<UserMessageType>::TransmissionProtocol protocol
)
{
    m_header.transmissionProtocol = protocol;
}



// ------------------------------------------------------------------ bodyManipulation

template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::Message<UserMessageType>::updateBodySize()
{
    m_body.resize(m_header.bodySize);
}

template <
    ::detail::constraint::isEnum UserMessageType
> void ::network::Message<UserMessageType>::resize(
    ::std::size_t newSize
)
{
    m_body.resize(newSize);
}

template <
    ::detail::constraint::isEnum UserMessageType
> auto ::network::Message<UserMessageType>::getBodyAddr()
    -> void*
{
    return m_body.data();
}



// ------------------------------------------------------------------ getters

template <
    ::detail::constraint::isEnum UserMessageType
> auto ::network::Message<UserMessageType>::getHeader() const
    -> const Message<UserMessageType>::Header&
{
    return m_header;
}

template <
    ::detail::constraint::isEnum UserMessageType
> auto ::network::Message<UserMessageType>::getHeader()
    -> Message<UserMessageType>::Header&
{
    return m_header;
}

template <
    ::detail::constraint::isEnum UserMessageType
> auto ::network::Message<UserMessageType>::getBody() const
    -> const ::std::vector<::std::byte>&
{
    return m_body;
}

template <
    ::detail::constraint::isEnum UserMessageType
> auto ::network::Message<UserMessageType>::getBody()
    -> ::std::vector<::std::byte>&
{
    return m_body;
}
