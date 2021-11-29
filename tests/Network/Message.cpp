#include <pch.hpp>
#include <Network/Message.hpp>
#include <Detail/Id.hpp>

enum class Enum{ last };



#include <boost/test/unit_test.hpp>
BOOST_AUTO_TEST_SUITE(Network)

BOOST_AUTO_TEST_CASE(Constructors)
{
    ::network::Message<Enum> msg1;
    ::network::Message<Enum> msg2{ ::Enum::last };
    ::network::Message<Enum> msg3{ ::Enum::last, ::std::string{ "hello" } };
    ::network::Message<Enum> msg4{ ::Enum::last, int{ 3 }, ::std::string{ "hello" }, float{ 50 } };
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

    msg.push(a);
    msg.push(b);
    msg.push(c);
    msg.push(d);
    msg.push(e);
    msg.push(f);

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

}

BOOST_AUTO_TEST_CASE(StdContainers)
{
    ::network::Message<Enum> msg;
    ::std::vector<int> a{ 1, 2, 3, 4, 5, 6 }, b;
    ::std::vector<::std::string> c{ "1", "2", "3", "4", "5", "6" }, d;
    ::std::array<::std::string, 4> e{ "1", "2", "3", "4" }, f;
    ::std::map<int, ::std::string> g{ {1, "1"}, {2, "2"}, {3, "3"} }, h;
    ::std::unordered_map<int, ::std::string> i{ {1, "1"}, {2, "2"}, {3, "3"} }, j;

    msg.push(a);
    msg.push(c);
    msg.push(e);
    msg.push(g);
    msg.push(i);

    msg.pull(j);
    msg.pull(h);
    msg.pull(f);
    msg.pull(d);
    msg.pull(b);

    BOOST_TEST(a == b);
    BOOST_TEST(c == d);
    BOOST_TEST(e == f);
    BOOST_TEST(g == h);
    BOOST_TEST(i == j);
}

BOOST_AUTO_TEST_CASE(ModifiedStdContainers)
{
    ::network::Message<Enum> msg;
    ::std::vector<int> a{ 1, 2, 3, 4, 5, 6 }, b;
    ::std::vector<::std::string> c{ "1", "2", "3", "4", "5", "6" }, d;
    ::std::map<int, ::std::string> g{ {1, "1"}, {2, "2"}, {3, "3"} }, h;
    ::std::unordered_map<int, ::std::string> i{ {1, "1"}, {2, "2"}, {3, "3"} }, j;

    msg.push(a);
    msg.push(c);
    msg.push(g);
    msg.push(i);

    a.insert(a.begin(), 1, 0);
    b.push_back(0);
    c.insert(c.begin(), 1, "0");
    d.push_back("0");

    msg.pull(j);
    msg.pull(h);
    msg.pull(d);
    msg.pull(b);

    BOOST_TEST(a == b);
    BOOST_TEST(c == d);
    BOOST_TEST(g == h);
    BOOST_TEST(i == j);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
