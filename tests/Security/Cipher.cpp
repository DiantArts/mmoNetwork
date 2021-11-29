#include <pch.hpp>
#include <Security/Cipher.hpp>
#include <Network/Message.hpp>

enum class Enum{ last };



#include <boost/test/unit_test.hpp>
BOOST_AUTO_TEST_SUITE(Security)

BOOST_AUTO_TEST_CASE(simpleEncryptionDecryption)
{
    ::security::Cipher serverCipher;
    ::security::Cipher clientCipher;
    serverCipher.setTargetPublicKey(clientCipher.getPublicKey());
    clientCipher.setTargetPublicKey(serverCipher.getPublicKey());

    auto baseValue{ serverCipher.generateRandomData(1024) };
    auto baseValueCpy{ baseValue };

    BOOST_TEST(baseValueCpy == baseValue);
    serverCipher.encrypt(baseValue);
    BOOST_TEST(baseValueCpy != baseValue);
    clientCipher.decrypt(baseValue);

    BOOST_TEST(baseValueCpy == baseValue);
    clientCipher.encrypt(baseValue);
    BOOST_TEST(baseValueCpy != baseValue);
    serverCipher.decrypt(baseValue);
    BOOST_TEST(baseValueCpy == baseValue);
}

BOOST_AUTO_TEST_CASE(scrambleEncryptionDecryption)
{
    ::security::Cipher serverCipher;
    ::security::Cipher clientCipher;
    serverCipher.setTargetPublicKey(clientCipher.getPublicKey());
    clientCipher.setTargetPublicKey(serverCipher.getPublicKey());

    auto baseValue{ serverCipher.generateRandomData(1024) };
    auto baseValueCpy{ baseValue };
    BOOST_TEST(baseValueCpy == baseValue);

    serverCipher.encrypt(baseValue);
    clientCipher.decrypt(baseValue);
    clientCipher.scramble(baseValue);
    clientCipher.encrypt(baseValue);
    serverCipher.decrypt(baseValue);

    BOOST_TEST(baseValueCpy != baseValue, "scramble gives the same value"); // TODO: needs scramble to work
    serverCipher.scramble(baseValueCpy);
    BOOST_TEST(baseValueCpy == baseValue, "scramble doesn give twice the same value");
}

BOOST_AUTO_TEST_CASE(sendEncryptionDecryption)
{
    ::network::Message<Enum> msg;
    ::security::Cipher serverCipher;
    ::security::Cipher clientCipher;
    serverCipher.setTargetPublicKey(clientCipher.getPublicKey());
    clientCipher.setTargetPublicKey(serverCipher.getPublicKey());

    // server
    auto baseValue{ serverCipher.generateRandomData(1024) };
    auto baseValueCpy{ baseValue };
    BOOST_TEST(baseValueCpy == baseValue);
    serverCipher.encrypt(baseValue);
    msg.push(::std::vector{ baseValue });

    // client
    auto receivedValue{ msg.pull<::std::vector<::std::byte>>() };
    BOOST_TEST(baseValue == receivedValue);
    clientCipher.decrypt(receivedValue);
    BOOST_TEST(baseValueCpy == receivedValue);
    clientCipher.scramble(receivedValue);
    clientCipher.encrypt(receivedValue);
    msg.push(receivedValue);

    // server
    auto receivedBackValue{ msg.pull<::std::vector<::std::byte>>() };
    BOOST_TEST(receivedValue == receivedBackValue);
    serverCipher.decrypt(receivedBackValue);
    BOOST_TEST(baseValueCpy == receivedBackValue);
}

BOOST_AUTO_TEST_SUITE_END()
