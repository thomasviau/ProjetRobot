//
// Created by viauthom on 06/05/19.
//

#ifndef ROBOT_V2_DISPATCHER_H
#define ROBOT_V2_DISPATCHER_H

#include "postman.h"

typedef struct DispatcherCommandoT DispatcherCommando;

/**
 * @brief dispatcherCommando class constructor
 *
 * Allocates a singleton dispatcherCommando object
 *
 * @retval no retval
 */
extern void dispatcherCommandoNew(void);

/**
 * @brief dispatcherCommando class starter
 *
 * Starts the dispatcherCommando object
 *
 * @retval no retval
 */
extern void dispatcherCommandoStart(void);

/**
 * @brief dispatcherCommando class stopper
 *
 * Stops the dispatcherCommando object
 *
 * @retval no retval
 */
extern void dispatcherCommandoStop(void);

/**
 * @brief dispatcherCommando class destructor
 *
 * Destructs a singleton dispatcherCommando object
 *
 * @retval no retval
 */
extern void dispatcherCommandoFree(void);

/**
 * @brief dispatcherCommando class destructor
 *
 * Decode the message received by the postman
 *
 * @retval no retval
 */
extern void dispatcherCommandoDecode(Param);

#endif //ROBOT_V2_DISPATCHER_H
