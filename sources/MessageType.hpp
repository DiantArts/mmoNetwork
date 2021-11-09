#pragma once



enum class MessageType : ::std::uint16_t
{
    message, // TODO implement
    messageAll,
    startCall,
    incommingCall,
    acceptCall,
    invalidTarget,
    refuseCall,
    setName,
    last,
};
