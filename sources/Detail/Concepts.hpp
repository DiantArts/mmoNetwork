#pragma once

namespace detail { class Id; }



namespace detail::constraint {


template <
    typename Type
> concept hasEnumValueLast = requires(Type){ Type::last; };

template <
    typename Type
> concept isEnum = ::std::is_enum<Type>::value && ::detail::constraint::hasEnumValueLast<Type>;

template <
    typename Type1,
    typename Type2
> concept sameAs = ::std::same_as<::std::remove_cvref_t<Type1>, ::std::remove_cvref_t<Type2>>;

template <
    typename Type
> concept isSendableData =
    !::detail::constraint::sameAs<Type, char*> &&
    (::std::is_trivial<Type>::value || ::detail::constraint::sameAs<Type, ::detail::Id>);

template <
    typename Type
> concept isNotSendableData = !isSendableData<Type>;

template <
    typename Type
> concept isPointer = ::std::is_pointer<Type>::value;




} // namespace detail::constraint
