//
// Akanya, Engine
// includes/pch
// g++ -x c++-header -Wpedantic -Wall -Wextra -Wno-volatile -Wno-address -Wno-nonnull-compare -std=c++20 -Ofast -pipe -o includes/pch.hpp.gch -Iincludes -Isources -Ilibraries -Iexterns includes/pch.hpp
//

#ifndef ___INCLUDE_GUARD_INCLUDES_PCH_HPP___
#define ___INCLUDE_GUARD_INCLUDES_PCH_HPP___



#include <stack>
#include <vector>
#include <list>
#include <deque>
#include <array>
#include <set>
#include <map>
#include <queue>
#include <unordered_set>
#include <unordered_map>

#include <algorithm>
#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <ranges>
#include <span>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>
#include <thread>
#include <utility>
#include <concepts>
#include <bitset>
#include <initializer_list>
#include <compare>
#include <cstring>

using ::std::string_literals::operator""s;
using ::std::literals::string_view_literals::operator""sv;
using ::std::chrono_literals::operator""ms;



#include <debugMacros.hpp>



#include <boost/array.hpp>
#include <boost/enable_shared_from_this.hpp>
#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#include <boost/bind.hpp>

#define ASIO_STANDALONE
#include <boost/asio.hpp>
#include <boost/asio/ts/buffer.hpp>
#include <boost/asio/ts/internet.hpp>



#define ENABLE_ENCRYPTION true



#endif // ___INCLUDE_GUARD_INCLUDES_PCH_HPP___
