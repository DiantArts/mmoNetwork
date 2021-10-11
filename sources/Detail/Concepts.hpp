#pragma once

namespace detail {



template <
    typename Type
> concept isEnum = ::std::is_enum<Type>::value;

template <
    typename Type
> concept isStandardLayout = ::std::is_standard_layout<Type>::value;

template <
    typename Type1,
    typename Type2
> concept sameAs = ::std::same_as<::std::remove_cvref_t<Type1>, ::std::remove_cvref_t<Type2>>;

template <
    typename Type
> concept isPointer = ::std::is_pointer<Type>::value;




} // namespace detail
