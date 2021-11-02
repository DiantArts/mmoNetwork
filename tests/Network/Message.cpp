#include <pch.hpp>
#include <Network/Message.hpp>
#include <Detail/Id.hpp>

enum class Enum{ nothing };



#include <boost/test/unit_test.hpp>
BOOST_AUTO_TEST_SUITE(Network)

BOOST_AUTO_TEST_CASE(Constructors)
{
    ::network::Message<Enum> msg1;
    ::network::Message<Enum> msg2{ ::Enum::nothing };
    ::network::Message<Enum> msg3{ ::Enum::nothing, ::std::string{ "hello" } };
    ::network::Message<Enum> msg4{ ::Enum::nothing, ::std::string{ "hello" }, int{ 3 }, float{ 50 } };
}



BOOST_AUTO_TEST_SUITE(insertExtract)



BOOST_AUTO_TEST_CASE(SingleCpy)
{
    ::network::Message<Enum> msg;
    int a{ 5 };
    msg << a;
    decltype(a) b;
    msg >> b;
    BOOST_TEST(a == b);
}

BOOST_AUTO_TEST_CASE(SingleMove)
{
    ::network::Message<Enum> msg;
    int a{ 5 };
    int aCpy{ a };
    msg << ::std::move(a);
    decltype(a) b;
    msg >> b;
    BOOST_TEST(aCpy == b);
}

BOOST_AUTO_TEST_CASE(SinglePointer)
{
    ::network::Message<Enum> msg;
    const char* a{ "hello" };
    msg << a;
    ::std::string b;
    msg >> b;
    BOOST_TEST(a == b);
}

BOOST_AUTO_TEST_CASE(MultipleTypes)
{
    ::network::Message<Enum> msg;
    int a{ 1 };
    float b{ 3.5 };
    char c{ 'c' };
    ::std::pair<int, float> d{ 1, 5.4 };
    ::detail::Id e{ 3 };
    ::std::string f{ "hello" };
    ::std::vector<int> g{ 1, 2, 3, 4, 5, 6 };
    ::std::vector<::std::string> h{ "1", "2", "3", "4", "5", "6" };
    msg << a << b << c << d << e << f << g << h;
    int i;
    float j;
    char k;
    ::std::pair<int, float> l;
    ::detail::Id m;
    ::std::string n;
    ::std::vector<int> o;
    ::std::vector<::std::string> p;
    msg >> p >> o >> n >> m >> l >> k >> j >> i;
    BOOST_TEST(a == i);
    BOOST_TEST(b == j);
    BOOST_TEST(c == k);
    BOOST_TEST((d.first == l.first && d.second == l.second));
    BOOST_TEST(e == m);
    BOOST_TEST(f == n);
    BOOST_TEST(g == o);
    BOOST_TEST(h == p);
}


BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
