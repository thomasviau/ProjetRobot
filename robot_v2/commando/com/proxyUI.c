#include "proxyUI.h"
#include "postman.h"

void proxyUIEncode(Param param){
    MqMsg mqMsg={
            .event = E_SEND_MSG;
            .param = param;
    };
    postmanCommandoMqSend(mqMsg);
}

void setPilotState(PilotState pilotState){
    Param param = {
            .idMethod = 1;
            .pilotState = pilotState;
    };
    proxyUIEncode(param);
}

void setEvents(int from, int to){
    Param param = {
            .idMethod = 2;
            .from = from;
            .to = to;
    };
    proxyUIEncode(param);
}

void setEventsCount(int indice){
    Param param = {
            .idMethod = 3;
            .indice = indice;
    };
    proxyUIEncode(param);
}