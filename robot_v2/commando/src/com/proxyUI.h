#ifndef ROBOT_V2_PROXYUI_H
#define ROBOT_V2_PROXYUI_H

#include "../robot/pilot.h"
#include "../robot/logger.h"

extern void setPilotState(PilotState);

extern void setLogs(Log[]);

extern void setLogsCount(int);

#endif //ROBOT_V2_PROXYUI_H
