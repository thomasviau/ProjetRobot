#ifndef ROBOT_V2_ROBOT_H
#define ROBOT_V2_ROBOT_H

typedef enum {
    NO_BUMP=0,
    BUMPED
} Collision;

typedef struct SensorStateT {
    Collision collision;
    float luminosity;
} SensorState;

typedef struct RobotT Robot;

/**
 * Start the Robot (initialize communication and open port)

 */
extern void robotStart();

/**
 * Stop Robot (stop communication and close port)
 */
extern void robotStop();

/**
 * @briel initialize in memory the object Robot
 *
 */
extern void robotNew();

/**
 *  @brief destruct the object Robot from memory
 */
extern void robotFree();

/**
 * getRobotSpeed
 *
 * @brief return the speed of the robot (positive average of the right's and left's current wheel power)
 * @return speed of the robot (beetween 0 and 100)
 */
extern int robotGetSpeed();

/**
 * getSensorState
 *
 * @brief return the captor's states of the bumper and the luminosity
 * @return SensorState
 */
extern SensorState robotGetSensorState();

/**
 * setWheelsVelocity
 *
 * @brief set the power on the wheels of the robot
 * @param int mr : right's wheel power, value between -10O and 100
 * @param int ml : left's wheel power, value between -100 and 100
 */
extern void robotSetWheelsVelocity(int mr,int ml);

#endif //ROBOT_V2_ROBOT_H