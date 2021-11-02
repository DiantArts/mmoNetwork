#pragma once

#include <Detail/Concepts.hpp>
#include <Network/TransmissionProtocol.hpp>



namespace network {



template <
    ::detail::isEnum UserMessageType
> class Message {

public:

    // ------------------------------------------------------------------ Header

    enum class TransmissionProtocol : ::std::uint16_t {
        unspecified,
        udp,
        tcp
    };

    enum class SystemType : ::std::uint16_t {
        // error = static_cast<::std::uint16_t>(UserMessageType::last) + 1, // TODO: fix that
        error = 10000,
        identificationAccepted,
        identificationDenied,
        authentification,
        authentificationAccepted,
        authentificationDenied,
        ping, // TODO imlement
        message, // TODO imlement
        messageAll,
        startCall,
        incommingCall,
        acceptCall,
        refuseCall,
        setName,
    };

    // Describes the message. Always put the transmission protocol at the end since it isnt sent
    // Uses unused space cause padding
    struct Header {
        ::std::uint16_t packetType;
        ::std::uint16_t bodySize{ 0 };
        Message<UserMessageType>::TransmissionProtocol transmissionProtocol{
            Message<UserMessageType>::TransmissionProtocol::unspecified
        };
    };




public:

    // ------------------------------------------------------------------ *structors

    Message();

    Message(
        Message<UserMessageType>::SystemType messageType,
        auto&&... args
    );

    Message(
        UserMessageType messageType,
        auto&&... args
    );

    ~Message();



    // ------------------------------------------------------------------ insert
    // Insert any POD-like data into the body

    void insert(
        const ::detail::isSendableData auto& data
    );

    void insert(
        ::detail::isSendableData auto&& data
    );

    void insert(
        const ::std::span<auto>& data
    );

    void insert(
        const ::std::pair<auto, auto>& data
    );

    void insert(
        const ::std::vector<auto>& data
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



    void insertRawMemory(
        auto* ptrToData,
        const ::std::size_t size
    );


    void insertAll(
        auto&&... args
    );



    auto operator<<(
        const auto& data
    ) -> Message<UserMessageType>&;

    auto operator<<(
        auto&& data
    ) -> Message<UserMessageType>&;



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
        ::std::pair<auto, auto>& data
    );

    void extract(
        ::std::vector<auto>& data
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
    ) -> Message<UserMessageType>&;



    // ------------------------------------------------------------------ informations

    [[ nodiscard ]] constexpr static inline auto getHeaderSize()
        -> ::std::size_t;

    [[ nodiscard ]] constexpr static inline auto getSendingHeaderSize()
        -> ::std::size_t;

    [[ nodiscard ]] auto getBodySize() const
        -> ::std::size_t;

    [[ nodiscard ]] auto getSize() const
        -> ::std::size_t;

    [[ nodiscard ]] auto isBodyEmpty() const
        -> bool;

    auto getHeaderAddr()
        -> void*;

    auto getType() const
        -> UserMessageType;

    auto getTypeAsSystemType() const
        -> Message<UserMessageType>::SystemType;

    void setType(
        UserMessageType type
    );

    [[ nodiscard ]] auto getTransmissionProtocol()
        -> Message<UserMessageType>::TransmissionProtocol;

    void setTransmissionProtocol(
        Message<UserMessageType>::TransmissionProtocol protocol
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

    Message<UserMessageType>::Header m_header;
    ::std::vector<::std::byte> m_body;

};



} // namespace network

#include <Network/Message.impl.hpp>
