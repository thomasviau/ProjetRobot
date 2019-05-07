#include <stdio.h>
#include <stdlib.h>

#include "robot.h"
#include "prose.h"

#define LEFT_MOTOR MD
#define RIGHT_MOTOR MA
#define LIGHT_SENSOR S1
#define FRONT_BUMPER S3
#define FLOOR_SENSOR S2

static Robot* robot;
static SensorState* sensorState;

void robotNew() {
    robot = (Robot *) malloc(sizeof(Robot));
    sensorState = (SensorState *) malloc(sizeof(SensorState));
}

void robotStart() {
    if (ProSE_Intox_init("127.0.0.1", 12345) == -1)
        PProseError("Problème d'initialisation du simulateur Intox");

    robot->mD = Motor_open(LEFT_MOTOR);
    if (robot->mD == NULL) PProseError("Problème d'ouverture du moteur droit (port MD)");

    robot->mG = Motor_open(RIGHT_MOTOR);
    if (robot->mG == NULL) PProseError("Problème d'ouverture du moteur gauche (port MA)");

    robot->lightSensor = LightSensor_open(LIGHT_SENSOR);
    robot->contactSensor = ContactSensor_open(FRONT_BUMPER);
    robot->floorSensor = ContactSensor_open(FLOOR_SENSOR);

    robot->sensorState = sensorState;
}

void robotStop() {
    robotSetWheelsVelocity(0,0);

    Motor_close(robot->mD);
    Motor_close(robot->mG);

    LightSensor_close(robot->lightSensor);
    ContactSensor_close(robot->contactSensor);
    ContactSensor_close(robot->floorSensor);
}

void robotFree() {
    free(robot);
    free(sensorState);
}

int robotGetSpeed() {
    return (( abs(Motor_getCmd(robot->mG))+ abs(Motor_getCmd(robot->mD)) )/2);;
}

SensorState robotGetSensorState() {
    if (ContactSensor_getStatus(robot->contactSensor) == ERROR)
        PProseError("Problème de commande du contact sensor");

    robot->sensorState->collision = (ContactSensor_getStatus(robot->contactSensor) == RELEASED) ? NO_BUMP : BUMPED;

    if (ContactSensor_getStatus(robot->floorSensor) == ERROR)
        PProseError("Problème de commande du contact sensor");

    robot->sensorState->collision = (ContactSensor_getStatus(robot->floorSensor) == RELEASED) ? NO_BUMP : BUMPED;

    if (LightSensor_getStatus(robot->lightSensor) == -1)
        PProseError("Problème de commande du light sensor");

    robot->sensorState->luminosity = (float) LightSensor_getStatus(robot->lightSensor);

    return *(robot->sensorState);
}

void robotSetWheelsVelocity(int mr,int ml) {
    if (Motor_setCmd(robot->mD, mr) == -1)
        PProseError("Problème de commande du moteur droit");

    if (Motor_setCmd(robot->mG, ml) == -1)
        PProseError("Problème de commande du moteur gauche");
}