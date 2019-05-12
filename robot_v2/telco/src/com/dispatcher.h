#ifndef ROBOT_V2_DISPATCHER_H
#define ROBOT_V2_DISPATCHER_H

typedef struct DispatcherTelcoT DispatcherTelco;

/**
 * @brief dispatcherTelco class constructor
 *
 * Allocates a singleton dispatcherTelco object
 *
 * @retval no retval
 */
extern void dispatcherTelcoNew(void);

/**
 * @brief dispatcherTelco class starter
 *
 * Starts the dispatcherTelco object
 *
 * @retval no retval
 */
extern void dispatcherTelcoStart(void);

/**
 * @brief dispatcherTelco class stopper
 *
 * Stops the dispatcherTelco object
 *
 * @retval no retval
 */
extern void dispatcherTelcoStop(void);

/**
 * @brief dispatcherTelco class destructor
 *
 * Destructs a singleton dispatcherTelco object
 *
 * @retval no retval
 */
extern void dispatcherTelcoFree(void);

/**
 * @brief dispatcherTelco class destructor
 *
 * Decode the message received by the postman
 *
 * @retval no retval
 */
extern void dispatcherTelcoDecode(int);

#endif //ROBOT_V2_DISPATCHER_H
