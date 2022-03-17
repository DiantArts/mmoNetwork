#pragma once

#include <Detail/Id.hpp>



namespace network {



struct SharableInformations {

    enum class Index {
        name = 1
    };

    ::std::string name;
};



} // namespace network



void push(
    ::network::Message<auto>& message,
    const ::network::SharableInformations& informations
)
{
    ::push(message, informations.name);
}

void pull(
    ::network::Message<auto>& message,
    ::network::SharableInformations& informations
)
{
    ::pull(message, informations.name);
}
