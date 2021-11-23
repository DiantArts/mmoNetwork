#pragma once

#include <sodium.h>



#if ENABLE_ENCRYPTION



namespace security {



class Cipher {

public:

    using PublicKey = ::std::array<::std::byte, crypto_box_PUBLICKEYBYTES>;



public:

    // ------------------------------------------------------------------ *structors

    explicit Cipher();

    ~Cipher();



    // ------------------------------------------------------------------ keyManagment

    [[ nodiscard ]] auto getPublicKey() const
        -> const Cipher::PublicKey&;

    [[ nodiscard ]] auto getTargetPublicKey() const
        -> const Cipher::PublicKey&;

    void setTargetPublicKey(
        Cipher::PublicKey targetPublicKey
    );



    // ------------------------------------------------------------------ Data modification

    static void generateRandomData(
        ::std::vector<::std::byte>&
    );

    static auto generateRandomData(
        ::std::size_t size
    ) -> ::std::vector<::std::byte>;

    static void scramble(
        ::std::vector<::std::byte>&
    );

    void encrypt(
        ::std::vector<::std::byte>&
    ) const;

    void decrypt(
        ::std::vector<::std::byte>&
    ) const;




private:

    // personal keys
    ::std::array<::std::byte, crypto_box_PUBLICKEYBYTES> m_publicKey; // must bo shared to communicate
    ::std::array<::std::byte, crypto_box_SECRETKEYBYTES> m_privateKey; // must remain secret

    // target key
    ::std::array<::std::byte, crypto_box_PUBLICKEYBYTES> m_targetPublicKey; // shared key of the target

};



} // namespace security



#endif // ENABLE_ENCRYPTION
