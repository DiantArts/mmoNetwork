#pragma once



enum class MessageType : ::std::uint16_t
{
    message, // TODO imlement
    messageAll,
    startCall,
    incommingCall,
    acceptCall,
    invalidTarget,
    refuseCall,
    setName,
    last,
};
