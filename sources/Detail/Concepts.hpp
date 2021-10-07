#pragma once

namespace detail {



template <
    typename Type
> concept IsEnum = ::std::is_enum<Type>::value;

template <
    typename Type
> concept IsStandardLayout = ::std::is_standard_layout<Type>::value;

template <
    typename Type1,
    typename Type2
> concept same_as = ::std::same_as<::std::remove_cvref_t<Type1>, ::std::remove_cvref_t<Type2>>;




} // namespace detail
