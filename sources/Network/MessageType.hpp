#pragma once



namespace network {



enum class MessageType : uint8_t
{
    error = 0,
    udpPort,
    identificationDenied,
    ping,
    messageAll,
    message
};



} // namespace network
