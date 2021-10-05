#pragma once

#include <sodium.h>



namespace security {



class Cipher {

public:

    // ------------------------------------------------------------------ *structors

    explicit Cipher();

    ~Cipher();



    // ------------------------------------------------------------------ keyManagment

    [[ nodiscard ]] auto getPublicKeyAddr()
        -> void*;

    [[ nodiscard ]] auto getTargetPublicKeyAddr()
        -> void*;

    [[ nodiscard ]] auto getPublicKeySize() const
        -> ::std::size_t;



    // ------------------------------------------------------------------ Handshake

    [[ nodiscard ]] static auto scramble(
        ::std::uint64_t data
    ) -> ::std::uint64_t;




    // ------------------------------------------------------------------ Encrypt

    [[ nodiscard ]] auto encrypt(
        const void* rawMemory,
        ::std::size_t size
    ) const
        -> ::std::vector<::std::byte>;

    [[ nodiscard ]] static inline constexpr auto getEncryptedSize(
        ::std::size_t size
    ) -> ::std::size_t
    {
        return crypto_box_SEALBYTES + size;
    }



    // ------------------------------------------------------------------ Decrypt

    [[ nodiscard ]] auto decrypt(
        const void* rawMemory,
        ::std::size_t size
    ) const
        -> ::std::vector<::std::byte>;




private:

    // personal keys
    ::std::array<::std::byte, crypto_box_PUBLICKEYBYTES> m_publicKey; // must bo shared to communicate
    ::std::array<::std::byte, crypto_box_SECRETKEYBYTES> m_privateKey; // must remain secret

    // target key
    ::std::array<::std::byte, crypto_box_PUBLICKEYBYTES> m_targetPublicKey; // shared key of the target

};



} // namespace security
