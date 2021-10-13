#pragma once

namespace detail { class Id; }



namespace detail {



template <
    typename Type
> concept isEnum = ::std::is_enum<Type>::value;

template <
    typename Type1,
    typename Type2
> concept sameAs = ::std::same_as<::std::remove_cvref_t<Type1>, ::std::remove_cvref_t<Type2>>;

template <
    typename Type
> concept isSendableData =
    !::detail::sameAs<Type, char*> &&
    (::std::is_trivial<Type>::value || ::detail::sameAs<Type, ::detail::Id>);

template <
    typename Type
> concept isPointer = ::std::is_pointer<Type>::value;




} // namespace detail
