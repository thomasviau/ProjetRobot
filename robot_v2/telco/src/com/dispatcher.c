#include <stdbool.h>
#include <stdlib.h>

#include "../util.h"
#include "dispatcher.h"
#include "postman.h"
#include "../ihm/remoteUI.h"

/* ----------------------- NEW START STOP FREE -----------------------*/

struct DispatcherTelcoT{};

DispatcherTelco *dispatcherTelco;

void dispatcherTelcoNew(void) {
    dispatcherTelco = (DispatcherTelco*)malloc (sizeof(DispatcherTelco));
    STOP_ON_ERROR(dispatcherTelco == NULL);
}

void dispatcherTelcoStart(void) {}

void dispatcherTelcoStop(void) {}

void dispatcherTelcoFree(void) {
    free(dispatcherTelco);
}

/* ----------------------- DECODE AND ACTIONS METHODS -----------------------*/

void setPilotState(PilotState pilotState){
    //TODO
    TRACE(setPilotState);
}

void setEvents(int from, int to) {
    //TODO
    TRACE(setEvents);
}

void setEventsCount(int indice){
    //TODO
    TRACE(setEventsCount);
}

void dispatcherTelcoDecode(Param param) {
    switch(param.idMethod){
        case 1:
            setPilotState(param.pilotState);
            break;
        case 2:
            setLogs(param.from,param.to);
            break;
        case 3:
            setLogsCount(param.indice);
            break;
    }
}
