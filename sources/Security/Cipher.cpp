#include <pch.hpp>
#include <Security/Cipher.hpp>



#if ENABLE_ENCRYPTION



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

auto ::security::Cipher::getPublicKey() const
    -> const Cipher::PublicKey&
{
    return m_publicKey;
}

auto ::security::Cipher::getTargetPublicKey() const
    -> const Cipher::PublicKey&
{
    return m_targetPublicKey;
}

void ::security::Cipher::setTargetPublicKey(
    Cipher::PublicKey targetPublicKey
)
{
    m_targetPublicKey = ::std::move(targetPublicKey);
}



// ------------------------------------------------------------------ Data modification

void ::security::Cipher::generateRandomData(
    ::std::vector<::std::byte>& data
)
{
    if (data.size() > 0) {
        ::randombytes_buf(data.data(), data.size());
    }
}

auto ::security::Cipher::generateRandomData(
    ::std::size_t size
) -> ::std::vector<::std::byte>
{
    ::std::vector<::std::byte> data{ size };
    ::security::Cipher::generateRandomData(data);
    return data;
}

// TODO: SHA256
void ::security::Cipher::scramble(
    ::std::vector<::std::byte>& data
)
{
    // choose prime numbers
    // for (auto& dataElem : data) {
        // auto elem{ static_cast<::std::uint8_t>(dataElem) };
        // ::std::uint16_t p, q;
        // switch (elem % 4) {
        // case 0: p = static_cast<::std::uint16_t>(2069); q = static_cast<::std::uint16_t>(349); break;
        // case 1: p = static_cast<::std::uint16_t>(337); q = static_cast<::std::uint16_t>(839); break;
        // case 2: p = static_cast<::std::uint16_t>(73); q = static_cast<::std::uint16_t>(911); break;
        // case 3: p = static_cast<::std::uint16_t>(419); q = static_cast<::std::uint16_t>(181); break;
        // }

        // switch (elem % 5) {
        // case 0: elem ^= 0xDB; break;
        // case 1: elem ^= 0xE5; break;
        // case 2: elem ^= 0xFE; break;
        // case 3: elem ^= 0xA7; break;
        // case 4: elem ^= 0x8A; break;
        // case 5: elem ^= 0xFE; break;
        // case 6: elem ^= 0x5E; break;
        // case 7: elem ^= 0x15; break;
        // case 8: elem ^= 0xA9; break;
        // case 9: elem ^= 0x9B; break;
        // }
        // elem = (elem ^ (0x61 & (p * q))) % 0xFF;
    // }
}

void ::security::Cipher::encrypt(
    ::std::vector<::std::byte>& data
) const
{
    ::std::vector<::std::byte> dataCpy{ data };
    data.resize(crypto_box_SEALBYTES + dataCpy.size());
    ::crypto_box_seal(
        reinterpret_cast<unsigned char*>(data.data()),
        reinterpret_cast<const unsigned char*>(dataCpy.data()),
        dataCpy.size(),
        reinterpret_cast<const unsigned char*>(m_targetPublicKey.data())
    );
}

void ::security::Cipher::decrypt(
    ::std::vector<::std::byte>& data
) const
{
    ::std::vector<::std::byte> decrypted{ data.size() - crypto_box_SEALBYTES };
    if (
        ::crypto_box_seal_open(
            reinterpret_cast<unsigned char*>(decrypted.data()),
            reinterpret_cast<const unsigned char*>(data.data()),
            data.size(),
            reinterpret_cast<const unsigned char*>(m_publicKey.data()),
            reinterpret_cast<const unsigned char*>(m_privateKey.data())
        )
    ) {
        throw ::std::runtime_error("Decryption failed, invalid data.");
    }
    data = ::std::move(decrypted);
}



#endif // ENABLE_ENCRYPTION
