#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "logger.h"
#include "../watchdog.h"
#include "../com/proxyUI.h"

#define TIMEOUT_LOGGER (0.25*1000*1000)
#define MAX_LOGS_COUNT 1000

static int logsCount;

static Log robotLogs[MAX_LOGS_COUNT];

static const Log emptyLog = {
        .state.collision = 0,
        .state.luminosity = 0.0,
        .speed = 0
};

static Watchdog* watchdog;

static void appendLog(SensorState state, int speed) {
    if (logsCount == MAX_LOGS_COUNT)
        loggerClear();

    Log log = {
            .state = state,
            .speed = speed
    };
    robotLogs[logsCount] = log;

    logsCount++;
}

static void loggerWdExpires(Watchdog *this) {
    appendLog(robotGetSensorState(), robotGetSpeed());
}

void loggerStart() {
    logsCount = 0;

    watchdog = watchdogConstruct((uint32_t) TIMEOUT_LOGGER, &loggerWdExpires);
    watchdogStart(watchdog);
}

void loggerStop() {
    watchdogCancel(watchdog);
    watchdogDestroy(watchdog);
    free(watchdog);
}

void loggerClear() {
    for(int i = 0; i < logsCount; i++) {
        robotLogs[i] = emptyLog;
    }

    logsCount = 0;
}

void askLogsCount() {
    setLogsCount(logsCount);
}

void askLogs(int from, int to) {
    Log logsToSend[to-from];

    for (int i = 0; i < to-from; i++) {
        logsToSend[i] = robotLogs[from+i];
    };

    setLogs(logsToSend);
}