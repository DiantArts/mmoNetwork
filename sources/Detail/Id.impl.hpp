#pragma once

// ------------------------------------------------------------------ *structors

// default value is 0
constexpr ::detail::Id::Id()
    : m_value{ 0 }
{}

constexpr ::detail::Id::Id(
    ::std::integral auto baseValue
)
    : m_value{ static_cast<detail::Id::Type>(baseValue) }
{}

constexpr ::detail::Id::~Id() = default;



// ------------------------------------------------------------------ Get

constexpr auto ::detail::Id::get() const
    -> ::detail::Id::Type
{
    return m_value;
}

constexpr ::detail::Id::operator ::detail::Id::Type() const
{
    return m_value;
}
