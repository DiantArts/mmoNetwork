#pragma once

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
