#pragma once



namespace network {



enum class MessageType : uint8_t
{
    error = 0,
    identificationAccepted, // contains udp port number
    identificationDenied,
    ping,
    message,
    messageAll,
    startCall
};



} // namespace network
