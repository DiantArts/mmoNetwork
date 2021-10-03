#pragma once

#include <Detail/Concepts.hpp>



namespace network {



template <
    ::network::detail::IsEnum MessageType
> class Message {

public:

    struct Header {

        MessageType packetType{};
        ::std::uint16_t bodySize{ 0 };

    };



public:

    // ------------------------------------------------------------------ *structors

    inline Message() = default;

    inline Message(
        MessageType&& messageType,
        auto&&... args
    )
        : m_header{ .packetType = ::std::forward<decltype(messageType)>(messageType) }
    {
        this->insert(::std::forward<decltype(args)>(args)...);
    }

    inline ~Message() = default;



    // ------------------------------------------------------------------ insert
    // Insert any POD-like data into the body

    inline void insert(
        ::network::detail::IsStandardLayout auto&& data
    )
    {
        // change size and alloc if needed
        m_header.bodySize += sizeof(data);
        m_body.resize(m_header.bodySize);

        // insert data into the end of the vector
        ::std::memmove(m_body.data() + m_header.bodySize - sizeof(data), &data, sizeof(data));
    }

    // Multiple insertions
    inline void insert(
        ::network::detail::IsStandardLayout auto&&... data
    )
    {
        (this->insert(::std::forward<decltype(data)>(data)), ...);
    }

    inline auto operator<<(
        ::network::detail::IsStandardLayout auto&& data
    ) -> Message<MessageType>&
    {
        this->insert(::std::forward<decltype(data)>(data));
        return *this;
    }

    inline auto operator<<(
        const ::network::detail::IsStandardLayout auto& data
    ) -> Message<MessageType>&
    {
        decltype(data) copiedData{ data };
        this->insert(::std::move(copiedData));
        return *this;
    }



    // ------------------------------------------------------------------ extract
    // Extract any POD-like data from the end of the body

    inline auto extract(
        ::network::detail::IsStandardLayout auto& data
    ) -> Message<MessageType>&
    {
        m_header.bodySize -= sizeof(data);

        // extract data out of the end of the vector
        ::std::memmove(&data, m_body.data() + m_header.bodySize, sizeof(data));

        // resize so he doesn't actually yeet my data
        m_body.resize(m_header.bodySize);

        return *this;
    }

    template <
        ::network::detail::IsStandardLayout DataType
    > inline auto extract()
        -> DataType
    {
        DataType data;

        m_header.bodySize -= sizeof(data);

        // extract data out of the end of the vector
        ::std::memmove(&data, m_body.data() + m_header.bodySize, sizeof(data));

        // resize so he doesn't actually yeet my data
        m_body.resize(m_header.bodySize);

        return data;
    }

    inline auto operator>>(
        ::network::detail::IsStandardLayout auto& data
    ) -> Message<MessageType>&
    {
        return this->extract(data);
    }



    // ------------------------------------------------------------------ informations

    [[ nodiscard ]] constexpr static inline auto getHeaderSize()
        -> ::std::size_t
    {
        return sizeof(Message<MessageType>::Header);
    }

    [[ nodiscard ]] inline auto getBodySize() const
        -> ::std::size_t
    {
        return m_header.bodySize;
    }

    [[ nodiscard ]] inline auto getSize() const
        -> ::std::size_t
    {
        return this->getHeaderSize() + this->getBodySize();
    }

    [[ nodiscard ]] inline auto isBodyEmpty() const
        -> bool
    {
        return m_header.bodySize == 0;
    }



    // ------------------------------------------------------------------ header

    auto getHeaderAddr()
        -> void*
    {
        return &m_header;
    }

    auto getType() const
        -> MessageType
    {
        return m_header.packetType;
    }

    void setType(
        MessageType type
    )
    {
        m_header.packetType = type;
    }



    // ------------------------------------------------------------------ bodyManipulation

    void updateBodySize()
    {
        m_body.resize(m_header.bodySize);
    }

    void resize(
        ::std::size_t newSize
    )
    {
        m_body.resize(newSize);
    }

    auto getBodyAddr()
        -> void*
    {
        return m_body.data();
    }



    // ------------------------------------------------------------------ debug

    void displayHeader(
        const char direction[2]
    ) const
    {
        ::std::cout << direction << ' '
            << (int)m_header.packetType << ' '
            << (int)m_header.bodySize << '\n';
    }

    void displayBody(
        const char direction[2]
    ) const
    {
        ::std::cout << direction << " [body]\n";
        // ::std::cout.write((char*)m_body.data(), m_header.bodySize);
    }




private:

    Message<MessageType>::Header m_header;
    ::std::vector<::std::byte> m_body;

};



} // namespace network
