#ifndef ROBOT_V2_POSTMAN_H
#define ROBOT_V2_POSTMAN_H

#include "../robot/pilot.h"
#include "../robot/logger.h"

/**
 * @brief postmanCommando type
 */
typedef struct PostmanCommandoT PostmanCommando;

typedef union {
    PilotState pilotState;
    VelocityVector vel;
    Log *log;
    int from;
    int to;
    int indice;
    int idMethod;
} Param;

/**
 * @brief postmanCommando class constructor
 *
 * Allocates a singleton postmanCommando object
 *
 * @retval no retval
 */
extern void postmanCommandoNew(void);


/**
 * @brief postmanCommando class starter
 *
 * Starts the postmanCommando object
 *
 * @retval no retval
 */
extern void postmanCommandoStart(void);


/**
 * @brief postmanCommando class stopper
 *
 * Stops the postmanCommando object
 *
 * @retval no retval
 */

extern void postmanCommandoStop(void);

/**
 * @brief postmanCommando class destructor
 *
 * Destructs a singleton postmanCommando object
 *
 * @retval no retval
 */

extern void postmanCommandoFree(void);

extern void postmanCommandoSend(Param);


#endif //ROBOT_V2_POSTMAN_H
