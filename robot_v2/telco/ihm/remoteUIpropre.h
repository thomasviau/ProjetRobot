#ifndef COMMANDO_REMOTEUIPROPRE_H
#define COMMANDO_REMOTEUIPROPRE_H

#include "remoteUIpropre.h"
#include "../com/proxyPilot.h"
#include "../com/proxyLogger.h"

typedef enum {
    MAIN_SCREEN,
    LOG_SCREEN,
    NB_SCREEN
} IdScreen;

extern void remoteUIStart();

extern void remoteUIStop();

extern void remoteUIAsk4Log();

extern void remoteUIAskClearLog();

extern void remoteUIDisplay(IdScreen);

extern void setDir(Direction);

extern void setLogsCount(int);

extern void setLogs(Log[]);

extern void displayLogs();


#endif //COMMANDO_REMOTEUIPROPRE_H
