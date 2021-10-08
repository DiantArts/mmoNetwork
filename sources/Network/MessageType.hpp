#pragma once



namespace network {



enum class MessageType : uint8_t
{
    error = 0,
    identificationAccepted, // contains udp port number
    identificationDenied,
    ping,
    messageAll,
    message
};



} // namespace network
