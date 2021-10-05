#include <pch.hpp>
#include <Cipher.hpp>



// ------------------------------------------------------------------ *structors

::security::Cipher::Cipher()
{
    if (::sodium_init() < 0) {
        throw ::std::runtime_error("Sodium lib couldn't be initialized");
    }

    /* Generate key pair */
    ::crypto_box_keypair(
        reinterpret_cast<unsigned char*>(m_publicKey.data()),
        reinterpret_cast<unsigned char*>(m_privateKey.data())
    );
}

::security::Cipher::~Cipher() = default;



// ------------------------------------------------------------------ keyManagment

auto ::security::Cipher::getPublicKeyAddr()
    -> void*
{
    return m_publicKey.data();
}

auto ::security::Cipher::getTargetPublicKeyAddr()
    -> void*
{
    return m_targetPublicKey.data();
}

auto ::security::Cipher::getPublicKeySize() const
    -> ::std::size_t
{
    return m_publicKey.size();
}



// ------------------------------------------------------------------ Handshake

auto ::security::Cipher::scramble(
    ::std::uint64_t data
) -> ::std::uint64_t
{
    // choose prime numbers
    ::std::uint16_t p, q;
    switch (data % 4) {
    case 0: p = 2069; q = 349; break;
    case 1: p = 337; q = 839; break;
    case 2: p = 73; q = 911; break;
    default: p = 419; q = 181; break;
    }

    switch (data % 5) {
    case 0: data ^= 0xDBF45A4B5C378C; break;
    case 1: data ^= 0xE561AF40B4C687; break;
    case 2: data ^= 0xFEBC43AA150FFF; break;
    case 3: data ^= 0xA74BBC654E13BC; break;
    case 4: data ^= 0x8A4B6B124F37BB; break;
    }
    data ^= (0xE5A1BC46F38049 & (p * q));

    return data;
}




// ------------------------------------------------------------------ Encrypt

auto ::security::Cipher::encrypt(
    const void* rawMemory,
    size_t size
) const
    -> ::std::vector<::std::byte>
{
    ::std::vector<::std::byte> encrypted{ crypto_box_SEALBYTES + size };
    ::crypto_box_seal(
        reinterpret_cast<unsigned char*>(encrypted.data()),
        reinterpret_cast<const unsigned char*>(rawMemory),
        size,
        reinterpret_cast<const unsigned char*>(m_targetPublicKey.data())
    );
    return encrypted;
}



// ------------------------------------------------------------------ Decrypt

auto ::security::Cipher::decrypt(
    const void* rawMemory,
    size_t size
) const
    -> ::std::vector<::std::byte>
{
    ::std::vector<::std::byte> decrypted{ size - crypto_box_SEALBYTES };
    if (
        ::crypto_box_seal_open(
            reinterpret_cast<unsigned char*>(decrypted.data()),
            reinterpret_cast<const unsigned char*>(rawMemory),
            size,
            reinterpret_cast<const unsigned char*>(m_publicKey.data()),
            reinterpret_cast<const unsigned char*>(m_privateKey.data())
        )
    ) {
        ::std::runtime_error("Decryption failed, invalid data.");
    }
    return decrypted;
}
