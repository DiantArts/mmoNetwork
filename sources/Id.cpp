#include <pch.hpp>
#include <Id.hpp>


// ------------------------------------------------------------------ Get

::network::Id::operator ::std::string() const
{
    return ::std::to_string(m_value);
}



// ------------------------------------------------------------------ Set

void ::network::Id::operator=(Id::Type value)
{
    m_value = value;
}



// ------------------------------------------------------------------ Incrementation

auto ::network::Id::operator++()
    -> Id::Type
{
    ++m_value;
    return m_value;
}

auto ::network::Id::operator++(int)
    -> Id::Type
{
    auto value { m_value };
    ++m_value;
    return value;
}

void ::network::Id::increment()
{
    ++m_value;
}



// ------------------------------------------------------------------ Others

auto ::network::Id::operator<=>(const Id& other)
    -> ::std::weak_ordering
{
    return m_value <=> other.m_value;
}
