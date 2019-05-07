//
// Created by viauthom on 06/05/19.
//

#ifndef ROBOT_V2_POSTMAN_H
#define ROBOT_V2_POSTMAN_H

/**
 * @brief postmanCommando type
 */
typedef struct PostmanCommandoT PostmanCommando;

typedef union MsgAdapterT MsgAdapter;

typedef union ParamT Param;

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

#endif //ROBOT_V2_POSTMAN_H
