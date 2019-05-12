#include "proxyUI.h"
#include "postman.h"
#include "../util.h"

void proxyUIEncode(Param param){
    postmanCommandoSend(param);
}

void setPilotState(PilotState pilotState){
    TRACE("setPilotState");
    Param param = {
            .idMethod = 1,
            .pilotState = pilotState
    };
    proxyUIEncode(param);
}

void setLogs(Log log[]){
    TRACE("setEvents");
    Param param = {
            .idMethod = 2,
            .log = log
    };
    proxyUIEncode(param);
}

void setLogsCount(int indice){
    TRACE("setEventsCount");
    Param param = {
            .idMethod = 3,
            .indice = indice
    };
    proxyUIEncode(param);
}