#pragma once

#include <Detail/Concepts.hpp>



namespace network {



template <
    ::detail::constraint::isEnum UserMessageType
> class Message {

public:

    // ------------------------------------------------------------------ Header

    enum class TransmissionProtocol : ::std::uint16_t {
        unspecified,
        udp,
        tcp
    };
    using Protocol = TransmissionProtocol;

    enum class SystemType : ::std::uint16_t {
        error = static_cast<::std::uint16_t>(UserMessageType::last) + 1,
        publicKey,
        proposeHandshake,
        resolveHandshake,
        identificationAccepted,
        identificationDenied,
        authentification,
        authentificationAccepted,
        authentificationDenied,
        udpInformations,
        sharableInformations,
        allSharableInformations,
        newConnection,
        ping, // TODO imlement
        message, // TODO imlement
        messageAll,
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
        const auto& data
    );

    void push(
        auto&& data
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
        auto& data
    );

    template <
        typename DataType
    > [[ nodiscard ]] auto pull()
        -> DataType;

    void pullRawMemory(
        ::std::vector<::std::byte>& refToData,
        const ::std::size_t size
    );

    auto pullRawMemory(
        const ::std::size_t size
    ) -> ::std::vector<::std::byte>;

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

    [[ nodiscard ]] auto getHeaderAddr()
        -> void*;

    [[ nodiscard ]] auto getType() const
        -> UserMessageType;

    [[ nodiscard ]] auto getTypeAsSystemType() const
        -> Message<UserMessageType>::SystemType;

    [[ nodiscard ]] auto getTypeAsInt() const
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

    [[ nodiscard ]] auto getBodyAddr()
        -> void*;



    // ------------------------------------------------------------------ getters

    [[ nodiscard ]] auto getHeader() const
        -> const Message<UserMessageType>::Header&;

    [[ nodiscard ]] auto getHeader()
        -> Message<UserMessageType>::Header&;

    [[ nodiscard ]] auto getBody() const
        -> const ::std::vector<::std::byte>&;

    [[ nodiscard ]] auto getBody()
        -> ::std::vector<::std::byte>&;



private:

    Message<UserMessageType>::Header m_header;
    ::std::vector<::std::byte> m_body;

};



} // namespace network



// ------------------------------------------------------------------ push imlementation

void push(
    ::network::Message<auto>& message,
    const ::detail::constraint::isSendableData auto& data
);

void push(
    ::network::Message<auto>& message,
    ::detail::constraint::isSendableData auto&& data
);




template <
    ::detail::constraint::isEnum UserMessageType,
    ::detail::constraint::isSendableData Type
> void push(
    ::network::Message<UserMessageType>& message,
    ::std::span<Type> data
);

template <
    ::detail::constraint::isEnum UserMessageType,
    ::detail::constraint::isNotSendableData Type
> void push(
    ::network::Message<UserMessageType>& message,
    ::std::span<Type> data
);

template <
    ::detail::constraint::isEnum UserMessageType,
    ::detail::constraint::isSendableData Type,
    ::std::size_t size
> void push(
    ::network::Message<UserMessageType>& message,
    ::std::span<Type, size> data
);

template <
    ::detail::constraint::isEnum UserMessageType,
    ::detail::constraint::isNotSendableData Type,
    ::std::size_t size
> void push(
    ::network::Message<UserMessageType>& message,
    ::std::span<Type, size> data
);



void push(
    ::network::Message<auto>& message,
    const ::std::vector<auto>& data
);

void push(
    ::network::Message<auto>& message,
    ::std::vector<auto>&& data
);



void push(
    ::network::Message<auto>& message,
    const ::std::map<auto, auto>& data
);

void push(
    ::network::Message<auto>& message,
    ::std::map<auto, auto>&& data
);



void push(
    ::network::Message<auto>& message,
    const ::std::unordered_map<auto, auto>& data
);

void push(
    ::network::Message<auto>& message,
    ::std::unordered_map<auto, auto>&& data
);



template <
    ::detail::constraint::isEnum UserMessageType,
    typename Type,
    ::std::size_t size
> void push(
    ::network::Message<UserMessageType>& message,
    const ::std::array<Type, size>& data
);

template <
    ::detail::constraint::isEnum UserMessageType,
    typename Type,
    ::std::size_t size
> void push(
    ::network::Message<UserMessageType>& message,
    ::std::array<Type, size>&& data
);



void push(
    ::network::Message<auto>& message,
    const ::std::pair<auto, auto>& data
);

void push(
    ::network::Message<auto>& message,
    ::std::pair<auto, auto>&& data
);



template <
    ::detail::constraint::isEnum UserMessageType
> void push(
    ::network::Message<UserMessageType>& message,
    const ::std::string& data
);

template <
    ::detail::constraint::isEnum UserMessageType
> void push(
    ::network::Message<UserMessageType>& message,
    ::std::string&& data
);

template <
    ::detail::constraint::isEnum UserMessageType
> void push(
    ::network::Message<UserMessageType>& message,
    const char* ptrToData
);



// ------------------------------------------------------------------ pull implementation

template <
    ::detail::constraint::isEnum UserMessageType,
    typename Type
> [[ nodiscard ]] auto pull(
    ::network::Message<UserMessageType>& message
) -> Type;



void pull(
    ::network::Message<auto>& message,
    ::detail::constraint::isSendableData auto& data
);



template <
    ::detail::constraint::isEnum UserMessageType
> void pull(
    ::network::Message<UserMessageType>& message,
    ::std::span<auto> data
);

template <
    ::detail::constraint::isEnum UserMessageType
> void pull(
    ::network::Message<UserMessageType>& message,
    ::std::span<auto> data
);

template <
    ::detail::constraint::isEnum UserMessageType,
    ::detail::constraint::isSendableData Type,
    ::std::size_t size
> void pull(
    ::network::Message<UserMessageType>& message,
    ::std::span<Type, size> data
);

template <
    ::detail::constraint::isEnum UserMessageType,
    ::detail::constraint::isNotSendableData Type,
    ::std::size_t size
> void pull(
    ::network::Message<UserMessageType>& message,
    ::std::span<Type, size> data
);



template <
    ::detail::constraint::isEnum UserMessageType
> void pull(
    ::network::Message<UserMessageType>& message,
    ::std::vector<auto>& data
);

template <
    ::detail::constraint::isEnum UserMessageType,
    typename Type1,
    typename Type2
> void pull(
    ::network::Message<UserMessageType>& message,
    ::std::map<Type1, Type2>& data
);

template <
    ::detail::constraint::isEnum UserMessageType,
    typename Type1,
    typename Type2
> void pull(
    ::network::Message<UserMessageType>& message,
    ::std::unordered_map<Type1, Type2>& data
);

template <
    ::detail::constraint::isEnum UserMessageType,
    typename Type,
    ::std::size_t size
> void pull(
    ::network::Message<UserMessageType>& message,
    ::std::array<Type, size>& data
);



void pull(
    ::network::Message<auto>& message,
    ::std::pair<auto, auto>& data
);

template <
    ::detail::constraint::isEnum UserMessageType
> void pull(
    ::network::Message<UserMessageType>& message,
    ::std::string& data
);


// TODO: place that elsewhere if possible
#include <Network/Informations.hpp>
#include <Network/Message.impl.hpp>
