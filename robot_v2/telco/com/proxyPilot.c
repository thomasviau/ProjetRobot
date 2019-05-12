#include "postman.h"
#include "proxyPilot.h"

void proxyPilotEncode(Param param){
    postmanTelcoSend(param);
}

void setVelocity(VelocityVector vel){
    Param param = {
            .idMethod = 3,
            .vel = vel
    };
    proxyPilotEncode(param);
    TRACE(setVelocity);
}

void toggleEmergencyStop(void){
    Param param = {
            .idMethod = 4
    };
    proxyPilotEncode(param);
    TRACE(toggleEmergencyStop);
}

void askPilotState(void){
    Param param = {
            .idMethod = 5
    };
    proxyPilotEncode(param);
    TRACE(askPilotState);
}