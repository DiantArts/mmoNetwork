#pragma once



namespace network {



enum class MessageType : uint8_t
{
    error = 0,
    invalidTarget,
    identificationAccepted,
    identificationDenied,
    ping, // TODO imlement
    message, // TODO imlement
    messageAll,
    startCall,
    incommingCall,
    acceptCall,
    refuseCall,
};



} // namespace network
