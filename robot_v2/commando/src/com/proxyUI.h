#ifndef ROBOT_V2_PROXYUI_H
#define ROBOT_V2_PROXYUI_H

#include "../robot/pilot.h"
#include "../robot/logger.h"

extern void setPilotState(PilotState);

extern void setEvents(Log[]);

extern void setEventsCount(int);

#endif //ROBOT_V2_PROXYUI_H
