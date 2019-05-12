#include "proxyUI.h"
#include "postman.h"

void proxyUIEncode(Param param){
    postmanCommandoSend(param);
}

void setPilotState(PilotState pilotState){
    TRACE(setPilotState);
    Param param = {
            .idMethod = 1,
            .pilotState = pilotState
    };
    proxyUIEncode(param);
}

void setEvents(int from, int to){
    TRACE(setEvents);
    Param param = {
            .idMethod = 2,
            .from = from,
            .to = to
    };
    proxyUIEncode(param);
}

void setEventsCount(int indice){
    TRACE(setEventsCount);
    Param param = {
            .idMethod = 3,
            .indice = indice
    };
    proxyUIEncode(param);
}