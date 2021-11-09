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
    ::network::Message<Enum> msg4{ ::Enum::nothing, int{ 3 }, ::std::string{ "hello" }, float{ 50 } };
    BOOST_TEST(msg3.extract<::std::string>() == ::std::string{ "hello" });
    BOOST_TEST(msg4.extract<float>() == float{ 50 });
    BOOST_TEST(msg4.extract<::std::string>() == ::std::string{ "hello" });
    BOOST_TEST(msg4.extract<int>() == int{ 3 });
}



BOOST_AUTO_TEST_SUITE(insertExtract)



BOOST_AUTO_TEST_CASE(SingleCpy)
{
    ::network::Message<Enum> msg;
    int a{ 5 };
    msg.insert(a);
    decltype(a) b;
    msg.extract(b);
    BOOST_TEST(a == b);
}

BOOST_AUTO_TEST_CASE(SingleMove)
{
    ::network::Message<Enum> msg;
    int a{ 5 };
    int aCpy{ a };
    msg.insert(::std::move(a));
    decltype(a) b;
    msg.extract(b);
    BOOST_TEST(aCpy == b);
}

BOOST_AUTO_TEST_CASE(SinglePointer)
{
    ::network::Message<Enum> msg;
    const char* a{ "hello" };
    msg.insert(a);
    ::std::string b;
    msg.extract(b);
    BOOST_TEST(a == b);
}

BOOST_AUTO_TEST_CASE(SingleString)
{
    ::network::Message<Enum> msg;
    ::std::string a{ "hello" };
    msg.insert(a);
    decltype(a) b;
    msg.extract(b);
    BOOST_TEST(a == b);
}

BOOST_AUTO_TEST_CASE(Strings)
{
    ::network::Message<Enum> msg;
    ::std::string a{ "hello" };
    ::std::string b{ "" };
    msg.insert(a);
    msg.insert(b);
    decltype(a) c;
    decltype(b) d;
    msg.extract(d);
    msg.extract(c);
    BOOST_TEST(a == c);
    BOOST_TEST(b == d);
}

BOOST_AUTO_TEST_CASE(SingleIntVector)
{
    ::network::Message<Enum> msg;
    ::std::vector<int> a{ 1, 2, 3, 4, 5, 9, 8, 7, 6 };
    msg.insert(a);
    decltype(a) b;
    msg.extract(b);
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
    msg.insert(a);
    msg.insert(b);
    msg.insert(c);
    msg.insert(d);
    msg.insert(e);
    msg.insert(f);
    msg.insert(g);
    msg.insert(h);
    int i;
    float j;
    char k;
    ::std::pair<int, float> l;
    ::detail::Id m;
    ::std::string n;
    ::std::vector<int> o;
    ::std::vector<::std::string> p;
    msg.extract(p);
    msg.extract(o);
    msg.extract(n);
    msg.extract(m);
    msg.extract(l);
    msg.extract(k);
    msg.extract(j);
    msg.extract(i);
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
