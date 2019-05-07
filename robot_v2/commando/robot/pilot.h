#ifndef ROBOT_V2_PILOT_H
#define ROBOT_V2_PILOT_H

#include "../watchdog.h"

/**
 * @brief Direction type
 */
typedef enum {
    LEFT=0,
    RIGHT,
    FORWARD,
    BACKWARD
} Direction;

/**
 * @brief Velocity vector type
 */
typedef struct VelocityVectorT
{
    Direction dir;
    int power;
} VelocityVector;

/**
 * @brief Pilot type
 */
typedef struct PilotT
{
    int speed;
    int bump;
    float luminosity;
    Watchdog* watchdog;
} Pilot;

/**
 * @brief Pilot class constructor
 *
 * Allocates a singleton Pilot object
 *
 * @retval no retval
 */
extern void pilotNew();


/**
 * @brief Pilot class starter
 *
 * Starts the Pilot object
 *
 * @retval no retval
 */
extern void pilotStart();


/**
 * @brief Pilot class stopper
 *
 * Stops the Pilot object
 *
 * @retval no retval
 */

extern void pilotStop();

/**
 * @brief Pilot class destructor
 *
 * Destructs a singleton Pilot object
 *
 * @retval no retval
 */

extern void pilotFree();

/**
* setVelocity
*
* @brief description
* @param vel
*/
extern void pilotSetVelocity(VelocityVector vel);

/**
 * getState
 *
 * @brief description
 * @return PilotState
 */
extern Pilot pilotGetState();

/**
 * check
 *
 * @brief description
 */
extern void pilotCheck();

/**
 * toggle emergency stop
 *
 * @brief description
 */
extern void pilotToggleEmergencyStop();

extern void pilotCheckSM();

#endif //ROBOT_V2_PILOT_H
