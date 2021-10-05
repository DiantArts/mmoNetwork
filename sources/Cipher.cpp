#include <pch.hpp>
#include <Cipher.hpp>



// ------------------------------------------------------------------ *structors

::network::security::Cipher::Cipher()
{
    if (::sodium_init() < 0) {
        throw ::std::runtime_error("Sodium lib couldn't be initialized");
    }

    /* Generate key pair */
    ::crypto_kx_keypair(
        reinterpret_cast<unsigned char*>(m_publicKey.data()),
        reinterpret_cast<unsigned char*>(m_secretKey.data())
    );
}

::network::security::Cipher::~Cipher() = default;



// ------------------------------------------------------------------ keyManagment

// returns false if the public key is invalid
auto ::network::security::Cipher::checkServerPublicKey()
    -> bool
{
    // Compute two shared keys using the server's public key and the client's secret key.
    if (::crypto_kx_client_session_keys(
        reinterpret_cast<unsigned char*>(m_receiverKey.data()),
        reinterpret_cast<unsigned char*>(m_senderKey.data()),
        reinterpret_cast<const unsigned char*>(m_publicKey.data()),
        reinterpret_cast<const unsigned char*>(m_secretKey.data()),
        reinterpret_cast<const unsigned char*>(m_targetPublicKey.data())
    ) != 0) {
        ::std::cerr << "[CIPHER] Invalid server public key." << ::std::endl;
        return false;
    }
    return true;
}

auto ::network::security::Cipher::checkClientPublicKey()
    -> bool
{
    // Compute two shared keys using the client's public key and the server's secret key.
    if (::crypto_kx_server_session_keys(
        reinterpret_cast<unsigned char*>(m_receiverKey.data()),
        reinterpret_cast<unsigned char*>(m_senderKey.data()),
        reinterpret_cast<const unsigned char*>(m_publicKey.data()),
        reinterpret_cast<const unsigned char*>(m_secretKey.data()),
        reinterpret_cast<const unsigned char*>(m_targetPublicKey.data())
    ) != 0) {
        ::std::cerr << "[CIPHER] Invalid client public key." << ::std::endl;
        return false;
    }
    return true;
}

auto ::network::security::Cipher::getPublicKeyAddr()
    -> void*
{
    return m_publicKey.data();
}

auto ::network::security::Cipher::getTargetPublicKeyAddr()
    -> void*
{
    return m_targetPublicKey.data();
}

auto ::network::security::Cipher::getPublicKeySize() const
    -> ::std::size_t
{
    return crypto_kx_PUBLICKEYBYTES;
}



// ------------------------------------------------------------------ Handshake

auto ::network::security::Cipher::scramble(
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

void ::network::security::Cipher::encrypt(
    void* rawMemory,
    size_t size
) const
{}



// ------------------------------------------------------------------ Decrypt

void ::network::security::Cipher::decrypt(
    void* rawMemory,
    size_t size
) const
{}
