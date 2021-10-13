#pragma once

#include <Detail/Concepts.hpp>
#include <Network/TransmissionProtocol.hpp>



namespace network {



template <
    ::detail::isEnum MessageType
> class Message {

public:

    // ------------------------------------------------------------------ Header

    struct Header {
        MessageType packetType;
        ::network::TransmissionProtocol transmissionProtocol;
        ::std::uint16_t bodySize{ 0 };
    };



public:

    // ------------------------------------------------------------------ *structors

    Message();

    Message(
        MessageType&& messageType,
        ::network::TransmissionProtocol&& transmissionProtocol
    );

    Message(
        MessageType&& messageType,
        ::network::TransmissionProtocol&& transmissionProtocol,
        auto&&... args
    );

    ~Message();



    // ------------------------------------------------------------------ insert
    // Insert any POD-like data into the body

    void insert(
        ::detail::isSendableData auto&& data
    );

    void insert(
        const ::std::span<auto> data
    );

    void insert(
        const ::std::string& data
    );

    void insert(
        ::std::string_view data
    );

    void insert(
        const char* ptrToData
    );

    void insertRawData(
        auto* ptrToData,
        const ::std::size_t size
    );


    void insertAll(
        auto&&... args
    );



    auto operator<<(
        auto&& data
    ) -> Message<MessageType>&;

    auto operator<<(
        const auto& data
    ) -> Message<MessageType>&;



    // ------------------------------------------------------------------ extract
    // Extract any POD-like data from the end of the body

    void extractRawMemory(
        ::std::vector<::std::byte>& refToData,
        const ::std::size_t size
    );

    auto extractRawMemory(
        const ::std::size_t size
    ) -> ::std::vector<::std::byte>;

    void extract(
        ::detail::isSendableData auto& data
    );

    void extract(
        ::std::string& data
    );

    template <
        typename DataType
    > auto extract()
        -> DataType;

    auto operator>>(
        auto& data
    ) -> Message<MessageType>&;



    // ------------------------------------------------------------------ informations

    [[ nodiscard ]] constexpr static inline auto getHeaderSize()
        -> ::std::size_t;

    [[ nodiscard ]] auto getBodySize() const
        -> ::std::size_t;

    [[ nodiscard ]] auto getSize() const
        -> ::std::size_t;

    [[ nodiscard ]] auto getTransmissionProtocol() const
        -> ::network::TransmissionProtocol;

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

#include <Network/Message.impl.hpp>
