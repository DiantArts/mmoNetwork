#pragma once

#include <Network/Server/AServer.hpp>



namespace network {
    template <
        typename UserMessageType
    > using AServer = ::network::server::AServer<UserMessageType>;
}
