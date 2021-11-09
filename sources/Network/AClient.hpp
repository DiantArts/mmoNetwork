#pragma once

#include <Network/Client/AClient.hpp>



namespace network {
    template <
        typename UserMessageType
    > using AClient = ::network::client::AClient<UserMessageType>;
}
