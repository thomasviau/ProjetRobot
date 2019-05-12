//
// Created by viauthom on 06/05/19.
//

#ifndef ROBOT_V2_POSTMAN_H
#define ROBOT_V2_POSTMAN_H

/**
 * @brief postmanTelco type
 */
typedef struct PostmanTelcoT PostmanTelco;

typedef union {
    PilotState pilotState;
    VelocityVector vel;
    int from;
    int to;
    int indice;
    int idMethod;
} Param;

/**
 * @brief postmanTelco class constructor
 *
 * Allocates a singleton postmanTelco object
 *
 * @retval no retval
 */
extern void postmanTelcoNew(void);


/**
 * @brief postmanTelco class starter
 *
 * Starts the postmanTelco object
 *
 * @retval no retval
 */
extern void postmanTelcoStart(void);


/**
 * @brief postmanTelco class stopper
 *
 * Stops the postmanTelco object
 *
 * @retval no retval
 */

extern void postmanTelcoStop(void);

/**
 * @brief postmanTelco class destructor
 *
 * Destructs a singleton postmanTelco object
 *
 * @retval no retval
 */

extern void postmanTelcoFree(void);

extern void postmanTelcoSend(Param);


#endif //ROBOT_V2_POSTMAN_H
