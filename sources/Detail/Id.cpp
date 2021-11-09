#include <pch.hpp>
#include <Detail/Id.hpp>



// ------------------------------------------------------------------ *structors

::detail::Id::Id() = default;

::detail::Id::Id(
    Id::Type baseValue
)
    : m_value{ static_cast<detail::Id::Type>(baseValue) }
{}


::detail::Id::~Id() = default;



// ------------------------------------------------------------------ Get

::detail::Id::operator ::std::string() const
{
    return ::std::to_string(m_value);
}



// ------------------------------------------------------------------ Set

void ::detail::Id::operator=(Id::Type value)
{
    m_value = value;
}



// ------------------------------------------------------------------ Incrementation

auto ::detail::Id::operator++()
    -> Id::Type
{
    ++m_value;
    return m_value;
}

auto ::detail::Id::operator++(int)
    -> Id::Type
{
    auto value { m_value };
    ++m_value;
    return value;
}

void ::detail::Id::increment()
{
    ++m_value;
}
