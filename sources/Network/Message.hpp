#pragma once

#include <Detail/Concepts.hpp>



namespace network {



template <
    ::detail::IsEnum MessageType
> class Message {

public:

    // ------------------------------------------------------------------ Header

    struct Header {
        MessageType packetType{};
        ::std::uint32_t bodySize{ 0 };
    };



public:

    // ------------------------------------------------------------------ *structors

    Message();

    Message(
        MessageType&& messageType,
        auto&&... args
    )
        : m_header{ .packetType = ::std::forward<decltype(messageType)>(messageType) }
    {
        this->insert(::std::forward<decltype(args)>(args)...);
    }

    ~Message();



    // ------------------------------------------------------------------ insert
    // Insert any POD-like data into the body

    void insert(
        ::detail::IsStandardLayout auto&& data
    )
    {
        // change size and alloc if needed
        m_header.bodySize += sizeof(data);
        m_body.resize(m_header.bodySize);

        // insert data into the end of the vector
        ::std::memmove(m_body.data() + m_header.bodySize - sizeof(data), &data, sizeof(data));
    }

    // Multiple insertions
    void insert(
        ::detail::IsStandardLayout auto&&... data
    )
    {
        (this->insert(::std::forward<decltype(data)>(data)), ...);
    }

    auto operator<<(
        ::detail::IsStandardLayout auto&& data
    ) -> Message<MessageType>&
    {
        this->insert(::std::forward<decltype(data)>(data));
        return *this;
    }

    auto operator<<(
        const ::detail::IsStandardLayout auto& data
    ) -> Message<MessageType>&
    {
        decltype(data) copiedData{ data };
        this->insert(::std::move(copiedData));
        return *this;
    }



    // ------------------------------------------------------------------ extract
    // Extract any POD-like data from the end of the body

    auto extract(
        ::detail::IsStandardLayout auto& data
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
        ::detail::IsStandardLayout DataType
    > auto extract()
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

    auto operator>>(
        ::detail::IsStandardLayout auto& data
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

    [[ nodiscard ]] auto getBodySize() const
        -> ::std::size_t;

    [[ nodiscard ]] auto getSize() const
        -> ::std::size_t;

    [[ nodiscard ]] auto isBodyEmpty() const
        -> bool;



    // ------------------------------------------------------------------ header

    auto getHeaderAddr()
        -> void*;

    auto getType() const
        -> MessageType;

    void setType(
        MessageType type
    );



    // ------------------------------------------------------------------ bodyManipulation

    void updateBodySize();

    void resize(
        ::std::size_t newSize
    );

    auto getBodyAddr()
        -> void*;



    // ------------------------------------------------------------------ debug

    void displayHeader(
        const char direction[2]
    ) const;

    void displayBody(
        const char direction[2]
    ) const;




private:

    Message<MessageType>::Header m_header;
    ::std::vector<::std::byte> m_body;

};



} // namespace network
