#pragma once

#include <Detail/Concepts.hpp>



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
        publicKey,
        proposeHandshake,
        resolveHandshake,
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

    virtual ~Message();



    // ------------------------------------------------------------------ push
    // Insert any POD-like data into the body

    void push(
        const ::detail::isSendableData auto& data
    );

    void push(
        ::detail::isSendableData auto&& data
    );



    template <
        ::detail::isSendableData Type
    > void push(
        ::std::span<Type> data
    );

    template <
        ::detail::isNotSendableData Type
    > void push(
        ::std::span<Type> data
    );

    template <
        ::detail::isSendableData Type,
        ::std::size_t size
    > void push(
        ::std::span<Type, size> data
    );

    template <
        ::detail::isNotSendableData Type,
        ::std::size_t size
    > void push(
        ::std::span<Type, size> data
    );



    void push(
        const ::std::vector<auto>& data
    );

    void push(
        ::std::vector<auto>&& data
    );



    template <
        typename Type,
        ::std::size_t size
    > void push(
        const ::std::array<Type, size>& data
    );

    template <
        typename Type,
        ::std::size_t size
    > void push(
        ::std::array<Type, size>&& data
    );



    void push(
        const ::std::pair<auto, auto>& data
    );

    void push(
        ::std::pair<auto, auto>&& data
    );



    void push(
        const ::std::string& data
    );

    void push(
        ::std::string&& data
    );

    void push(
        const char* ptrToData
    );



    void pushRawMemory(
        auto* ptrToData,
        const ::std::size_t size
    );



    void pushAll(
        auto&&... args
    );

    auto operator<<(
        const auto& data
    ) -> Message<UserMessageType>&;

    auto operator<<(
        auto&& data
    ) -> Message<UserMessageType>&;



    // ------------------------------------------------------------------ pull
    // Extract any POD-like data from the end of the body

    void pull(
        ::detail::isSendableData auto& data
    );



    template <
        ::detail::isSendableData Type
    > void pull(
        ::std::span<Type> data
    );

    template <
        ::detail::isNotSendableData Type
    > void pull(
        ::std::span<Type> data
    );

    template <
        ::detail::isSendableData Type,
        ::std::size_t size
    > void pull(
        ::std::span<Type, size> data
    );

    template <
        ::detail::isNotSendableData Type,
        ::std::size_t size
    > void pull(
        ::std::span<Type, size> data
    );



    void pull(
        ::std::vector<auto>& data
    );

    template <
        typename Type,
        ::std::size_t size
    > void pull(
        ::std::array<Type, size>& data
    );

    void pull(
        ::std::pair<auto, auto>& data
    );

    void pull(
        ::std::string& data
    );



    void pullRawMemory(
        ::std::vector<::std::byte>& refToData,
        const ::std::size_t size
    );

    auto pullRawMemory(
        const ::std::size_t size
    ) -> ::std::vector<::std::byte>;



    template <
        typename DataType
    > auto pull()
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

    auto getTypeAsInt() const
        -> ::std::uint16_t;

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
