#pragma once

#include <Detail/Id.hpp>



namespace network {



template <
    typename UserMessageType
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

    template <
        typename... Args
    > Message(
        Message<UserMessageType>::SystemType messageType,
        Args&&... args
    );

    template <
        typename... Args
    > Message(
        UserMessageType messageType,
        Args&&... args
    );

    virtual ~Message();



    // ------------------------------------------------------------------ insert
    // Insert any POD-like data into the body

    template <
        typename Type,
        typename = typename ::std::enable_if<
                !::std::is_same<char*, ::std::remove_cv_t<::std::remove_reference_t<Type>>>::value &&
                (
                    ::std::is_trivial<Type>::value ||
                    ::std::is_same<::detail::Id, ::std::remove_cv_t<::std::remove_reference_t<Type>>>::value
                )
            >::type
    > void insert(
        const Type& data
    )
    {
        // change size and alloc if needed
        m_header.bodySize += sizeof(data);
        m_body.resize(m_header.bodySize);

        // insert data into the end of the vector
        ::std::memcpy(m_body.data() + m_header.bodySize - sizeof(data), &data, sizeof(data));
    }

    template <
        typename Type,
        typename = typename ::std::enable_if<
                !::std::is_same<char*, ::std::remove_cv_t<::std::remove_reference_t<Type>>>::value &&
                (
                    ::std::is_trivial<Type>::value ||
                    ::std::is_same<::detail::Id, ::std::remove_cv_t<::std::remove_reference_t<Type>>>::value
                )
            >::type
    > void insert(
        Type&& data
    )
    {
        // change size and alloc if needed
        m_header.bodySize += sizeof(data);
        m_body.resize(m_header.bodySize);

        // insert data into the end of the vector
        ::std::memmove(m_body.data() + m_header.bodySize - sizeof(data), &data, sizeof(data));
    }

    template <
        typename Type1,
        typename Type2
    > void insert(
        const ::std::pair<Type1, Type2>& data
    );

    template <
        typename Type1,
        typename Type2
    > void insert(
        ::std::pair<Type1, Type2>& data
    );

    template <
        typename Type
    > void insert(
        const ::std::vector<Type>& data
    );

    template <
        typename Type
    > void insert(
        ::std::vector<Type>& data
    );

    void insert(
        const ::std::string& data
    );

    void insert(
        ::std::string& data
    );

    void insert(
        ::std::string_view data
    );

    void insert(
        const char* ptrToData
    );



    template <
        typename Type
    > void insertRawMemory(
        Type* ptrToData,
        const ::std::size_t size
    );

    template <
        typename... Types
    > void insertAll(
        Types&&... types
    );



    // ------------------------------------------------------------------ extract
    // Extract any POD-like data from the end of the body

    void extractRawMemory(
        ::std::vector<::std::byte>& refToData,
        const ::std::size_t size
    );

    auto extractRawMemory(
        const ::std::size_t size
    ) -> ::std::vector<::std::byte>;

    template <
        typename Type
    > void extract(
        Type& data
    );

    template <
        typename Type1,
        typename Type2
    > void extract(
        ::std::pair<Type1, Type2>& data
    );

    template <
        typename Type
    > void extract(
        ::std::vector<Type>& data
    );

    void extract(
        ::std::string& data
    );

    template <
        typename DataType
    > auto extract()
        -> DataType;



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
