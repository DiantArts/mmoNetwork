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
    BOOST_TEST(msg3.pull<::std::string>() == ::std::string{ "hello" });
    BOOST_TEST(msg4.pull<float>() == float{ 50 });
    BOOST_TEST(msg4.pull<::std::string>() == ::std::string{ "hello" });
    BOOST_TEST(msg4.pull<int>() == int{ 3 });
}



BOOST_AUTO_TEST_SUITE(pushExtract)



BOOST_AUTO_TEST_CASE(SingleCpy)
{
    ::network::Message<Enum> msg;
    int a{ 5 };
    msg.push(a);
    decltype(a) b;
    msg.pull(b);
    BOOST_TEST(a == b);
}

BOOST_AUTO_TEST_CASE(SingleMove)
{
    ::network::Message<Enum> msg;
    int a{ 5 };
    int aCpy{ a };
    msg.push(::std::move(a));
    decltype(a) b;
    msg.pull(b);
    BOOST_TEST(aCpy == b);
}

BOOST_AUTO_TEST_CASE(SinglePointer)
{
    ::network::Message<Enum> msg;
    const char* a{ "hello" };
    msg.push(a);
    ::std::string b;
    msg.pull(b);
    BOOST_TEST(a == b);
}

BOOST_AUTO_TEST_CASE(SingleString)
{
    ::network::Message<Enum> msg;
    ::std::string a{ "hello" };
    msg.push(a);
    decltype(a) b;
    msg.pull(b);
    BOOST_TEST(a == b);
}

BOOST_AUTO_TEST_CASE(Strings)
{
    ::network::Message<Enum> msg;
    ::std::string a{ "hello" };
    ::std::string b{ "" };
    msg.push(a);
    msg.push(b);
    decltype(a) c;
    decltype(b) d;
    msg.pull(d);
    msg.pull(c);
    BOOST_TEST(a == c);
    BOOST_TEST(b == d);
}

BOOST_AUTO_TEST_CASE(SingleIntVector)
{
    ::network::Message<Enum> msg;
    ::std::vector<int> a{ 1, 2, 3, 4, 5, 9, 8, 7, 6, 2, 3, 4, 5 };
    msg.push(a);
    decltype(a) b;
    msg.pull(b);
    BOOST_TEST(a == b);
}

BOOST_AUTO_TEST_CASE(MultipleTypes)
{
    ::network::Message<Enum> msg;
    int a{ 1 }, i;
    float b{ 3.5 }, j;
    char c{ 'c' }, k;
    ::std::pair<int, float> d{ 1, 5.4 }, l;
    ::detail::Id e{ 3 }, m;
    ::std::string f{ "hello" }, n;
    ::std::vector<int> g{ 1, 2, 3, 4, 5, 6 }, o;
    ::std::vector<::std::string> h{ "1", "2", "3", "4", "5", "6" }, p;
    ::std::array<::std::string, 4> q{ "1", "hello", "blb" }, r;
    msg.push(a);
    msg.push(b);
    msg.push(c);
    msg.push(d);
    msg.push(e);
    msg.push(f);
    msg.push(g);
    msg.push(h);
    msg.push(q);
    msg.pull(r);
    msg.pull(p);
    msg.pull(o);
    msg.pull(n);
    msg.pull(m);
    msg.pull(l);
    msg.pull(k);
    msg.pull(j);
    msg.pull(i);
    BOOST_TEST(a == i);
    BOOST_TEST(b == j);
    BOOST_TEST(c == k);
    BOOST_TEST((d.first == l.first && d.second == l.second));
    BOOST_TEST(e == m);
    BOOST_TEST(f == n);
    BOOST_TEST(g == o);
    BOOST_TEST(h == p);
    BOOST_TEST(q == r);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
