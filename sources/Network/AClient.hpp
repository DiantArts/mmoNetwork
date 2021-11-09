#pragma once

#include <Network/Client/AClient.hpp>



namespace network {
    template <
        ::detail::isEnum UserMessageType
    > using AClient = ::network::client::AClient<UserMessageType>;
}
