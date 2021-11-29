#pragma once

#include <Detail/Id.hpp>



namespace network {



class Informations {

public:

    enum class Index
        : ::std::uint8_t
    {
        id = 1,
        name,
    };

    struct Sharable {

        ::std::string name;

    };



public:

    // ------------------------------------------------------------------ *structors

    Informations()
    {}

    Informations(
        ::detail::Id id
    )
        : m_id{ id }
    {}

    ~Informations() = default;



    // ------------------------------------------------------------------ getters/setters

    [[ nodiscard ]] auto getSharableOnes() const
        -> Informations::Sharable
    {
        return m_sharable;
    }



    void setId(
        ::detail::Id id
    )
    {
        m_id = id;
    }

    [[ nodiscard ]] auto getId() const
        -> ::detail::Id
    {
        return m_id;
    }

    [[ nodiscard ]] static inline constexpr auto getIdIndex()
        -> Informations::Index
    {
        return Informations::Index::id;
    }



    void setName(
        const ::std::string& name
    )
    {
        m_sharable.name = name;
    }

    [[ nodiscard ]] auto getName() const
        -> ::std::string
    {
        return m_sharable.name;
    }

    [[ nodiscard ]] static inline constexpr auto getNameIndex()
        -> Informations::Index
    {
        return Informations::Index::name;
    }



    template <
        ::network::Informations::Index indexValue
    > void set(
        auto&&... args
    )
    {
        if constexpr (indexValue == ::network::Informations::Index::name) {
            this->setName(::std::forward<decltype(args)>(args)...);
        }
    }





private:

    ::detail::Id m_id{ 0 };

    Informations::Sharable m_sharable;

};



} // namespace network



void push(
    ::network::Message<auto>& message,
    const ::network::Informations::Sharable& informations
)
{
    ::push(message, informations.name);
}

void pull(
    ::network::Message<auto>& message,
    ::network::Informations::Sharable& informations
)
{
    ::pull(message, informations.name);
}
