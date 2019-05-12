#ifndef COMMANDO_LOGGER_H
#define COMMANDO_LOGGER_H

#include "robot.h"

typedef struct Log_t Log;

extern void loggerStart();

extern void loggerStop();

extern void askLogs(int, int);

extern void askLogsCount();

extern void loggerClear();

#endif //COMMANDO_LOGGER_H
