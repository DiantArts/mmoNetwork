#pragma once



namespace network {



enum class MessageType : uint32_t
{
    Error = 0,
    IdentificationDenied,
    Ping,
    MessageAll,
    Message
};



} // namespace network
