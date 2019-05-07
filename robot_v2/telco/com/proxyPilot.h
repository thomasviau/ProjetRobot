#ifndef ROBOT_V2_PROXYPILOT_H
#define ROBOT_V2_PROXYPILOT_H

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
typedef struct VelocityVectorT {
    Direction dir;
    int power;
} VelocityVector;

extern void pilotSetVelocity(VelocityVector);

extern void pilotCheck();

#endif //ROBOT_V2_PROXYPILOT_H
