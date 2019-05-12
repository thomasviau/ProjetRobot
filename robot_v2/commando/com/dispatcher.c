#include <stdbool.h>
#include <stdlib.h>

#include "../util.h"
#include "dispatcher.h"


/* ----------------------- NEW START STOP FREE -----------------------*/

struct DispatcherCommandoT{};

DispatcherCommando *dispatcherCommando;

void dispatcherCommandoNew(void) {
    dispatcherCommando = (DispatcherCommando*)malloc (sizeof(DispatcherCommando));
    STOP_ON_ERROR(dispatcherCommando == NULL);
}

void dispatcherCommandoStart(void) {}

void dispatcherCommandoStop(void) {}

void dispatcherCommandoFree(void) {
    free(dispatcherCommando);
}

/* ----------------------- DECODE AND ACTIONS METHODS -----------------------*/

void askEvents(int from, int to){
    TRACE(askEvents);
    //TODO
}

void askEventsCount(int indice){
    TRACE(askEventsCount);
    //TODO
}

void setVelocity(VelocityVector vel){
    TRACE(setVelocity);
    pilotSetVelocity(vel);
}

void toggleEmergencyStop(void){
    TRACE(toggleEmergencyStop);
    pilotToggleEmergencyStop();
}

void askPilotState(void){
    TRACE(askPilotState);
    pilotAskPilotState();
}

void dispatcherCommandoDecode(Param param) {
    switch(param.idMethod){
        case 1:
            askEvents(param.from,param.to);
            break;
        case 2:
            askEventsCount(param.indice);
            break;
        case 3:
            setVelocity(param.vel);
            break;
        case 4:
            toggleEmergencyStop();
            break;
        case 5:
            askPilotState();
            break;
    }
}