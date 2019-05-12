#ifndef COMMANDO_LOGGER_H
#define COMMANDO_LOGGER_H

#include "robot.h";

extern void loggerStart();

extern void loggerStop();

extern int askEvents(int, int);

extern int askEventsCount();

extern void loggerClear();

extern int setRobotSpeed();

extern SensorState setSensorsState();

#endif //COMMANDO_LOGGER_H
