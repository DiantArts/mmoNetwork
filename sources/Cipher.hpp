#pragma once

#include <sodium.h>



namespace network::security {



class Cipher {

public:

    // ------------------------------------------------------------------ *structors

    explicit Cipher();

    ~Cipher();



    // ------------------------------------------------------------------ keyManagment

    // returns false if the public key is invalid
    [[ nodiscard ]] auto checkServerPublicKey()
        -> bool;

    [[ nodiscard ]] auto checkClientPublicKey()
        -> bool;

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

    // TODO: encrypt implementation
    void encrypt(
        void* rawMemory,
        size_t size
    ) const;



    // ------------------------------------------------------------------ Decrypt

    // TODO: decrypt implementation
    void decrypt(
        void* rawMemory,
        size_t size
    ) const;




private:

    // personal keys
    ::std::array<::std::byte, crypto_kx_PUBLICKEYBYTES> m_publicKey; // must bo shared to communicate
    ::std::array<::std::byte, crypto_kx_SECRETKEYBYTES> m_secretKey; // must remain secret
    ::std::array<::std::byte, crypto_kx_SESSIONKEYBYTES> m_receiverKey; // used to decrypt
    ::std::array<::std::byte, crypto_kx_SESSIONKEYBYTES> m_senderKey; // used to encrypt

    // target key
    ::std::array<::std::byte, crypto_kx_PUBLICKEYBYTES> m_targetPublicKey; // shared key of the target

};



} // namespace network::security
