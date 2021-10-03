#pragma once

#include <Detail/Concepts.hpp>



namespace network {



template <
    ::network::detail::IsEnum MessageEnumType
> class Message {

public:

    struct Header {

        MessageEnumType packetType{};
        ::std::uint16_t bodySize{ 0 };

    };



public:

    // ------------------------------------------------------------------ *structors

    inline Message() = default;

    inline Message(
        MessageEnumType messageType
    )
        : m_header{ .packetType = messageType }
    {}

    inline ~Message() = default;



    // ------------------------------------------------------------------ extract/insert

    // Insert any POD-like data into the body
    inline auto insert(
        ::network::detail::IsStandardLayout auto&& data
    ) -> Message<MessageEnumType>&
    {
        // change size and alloc if needed
        m_header.bodySize += sizeof(data);
        m_body.resize(m_header.bodySize);

        // insert data into the end of the vector
        ::std::memmove(m_body.data() + m_header.bodySize - sizeof(data), &data, sizeof(data));

        return *this;
    }

    inline auto operator<<(
        ::network::detail::IsStandardLayout auto&& data
    ) -> Message<MessageEnumType>&
    {
        return this->insert(::std::forward<decltype(data)>(data));
    }

    inline auto operator<<(
        const ::network::detail::IsStandardLayout auto& data
    ) -> Message<MessageEnumType>&
    {
        decltype(data) copiedData{ data };
        return this->insert(::std::move(copiedData));
    }



    // Extract any POD-like data into the body
    inline auto extract(
        ::network::detail::IsStandardLayout auto& data
    ) -> Message<MessageEnumType>&
    {
        m_header.bodySize -= sizeof(data);

        // extract data out of the end of the vector
        ::std::memmove(&data, m_body.data() + m_header.bodySize, sizeof(data));

        // resize so he doesn't actually yeet my data
        m_body.resize(m_header.bodySize);

        return *this;
    }

    inline auto operator>>(
        ::network::detail::IsStandardLayout auto& data
    ) -> Message<MessageEnumType>&
    {
        return this->extract(data);
    }



    // ------------------------------------------------------------------ informations

    [[ nodiscard ]] constexpr static inline auto getHeaderSize()
        -> ::std::size_t
    {
        return sizeof(Message<MessageEnumType>::Header);
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
        -> MessageEnumType
    {
        return m_header.packetType;
    }

    void setType(
        MessageEnumType type
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

    Message<MessageEnumType>::Header m_header;
    ::std::vector<::std::byte> m_body;

};



} // namespace network
