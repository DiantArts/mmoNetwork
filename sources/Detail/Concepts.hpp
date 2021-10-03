#pragma once

namespace network::detail {



template <
    class Type
> concept IsEnum = ::std::is_enum<Type>::value;

template <
    class Type
> concept IsStandardLayout = ::std::is_standard_layout<Type>::value;




} // namespace network::detail