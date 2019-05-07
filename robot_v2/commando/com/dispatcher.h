//
// Created by viauthom on 06/05/19.
//

#ifndef ROBOT_V2_DISPATCHER_H
#define ROBOT_V2_DISPATCHER_H

typedef struct DispatcherCommando DispatcherCommando;

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
extern void dispatcherCommandoDecode(int);

#endif //ROBOT_V2_DISPATCHER_H
