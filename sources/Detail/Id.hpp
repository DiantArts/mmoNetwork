#pragma once

namespace detail {



class Id {

public:

    using Type = ::std::uint64_t;



public:

    // ------------------------------------------------------------------ *structors

    // default value is 0
    Id();

    Id(
        Id::Type baseValue
    );

    ~Id();



    // ------------------------------------------------------------------ Get

    [[ nodiscard ]] constexpr auto get() const
        -> Id::Type;

    [[ nodiscard ]] constexpr operator Id::Type() const;

    [[ nodiscard ]] operator ::std::string() const;



    // ------------------------------------------------------------------ Set

    void operator=(Id::Type value);



    // ------------------------------------------------------------------ Incrementation

    auto operator++()
        -> Id::Type;

    [[ nodiscard ]] auto operator++(int)
        -> Id::Type;

    void increment();



private:

    Id::Type m_value{ 0 };

};



} // namespace detail

#include <Detail/Id.impl.hpp>
