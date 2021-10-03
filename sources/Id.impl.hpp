#pragma once

// ------------------------------------------------------------------ *structors

// default value is 0
constexpr ::network::Id::Id()
    : m_value{ 0 }
{}

constexpr ::network::Id::Id(
    ::std::integral auto baseValue
)
    : m_value{ static_cast<Id::Type>(baseValue) }
{}

constexpr ::network::Id::~Id() = default;



// ------------------------------------------------------------------ Get

constexpr auto ::network::Id::get() const
    -> ::network::Id::Type
{
    return m_value;
}

constexpr ::network::Id::operator ::network::Id::Type() const
{
    return m_value;
}
