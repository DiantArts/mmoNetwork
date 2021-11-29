#pragma once

#include <Network/Server/AServer.hpp>



namespace network {
    template <
        ::detail::constraint::isEnum UserMessageType
    > using AServer = ::network::server::AServer<UserMessageType>;
}
