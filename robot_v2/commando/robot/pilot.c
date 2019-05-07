#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <mqueue.h>
#include <stdbool.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>

#include "../watchdog.h"
#include "pilot.h"
#include "robot.h"

#define MQ_MSG_COUNT 10
#define MQ_MSG_SIZE 256

#define TIMEOUT_BUMP (1*1000*1000)

/**
 * @brief Instance of the Pilot class
 */
static Pilot *pilot;

static const VelocityVector velocityZero = {FORWARD, 0};

static void pilotSendMvt(VelocityVector);
static void pilotEvaluateVelocity();
static void pilotEvaluateBump();
static int pilotHasBumped();
static void pilotSetTimeout();
static void pilotResetTimeout();
static void pilotWdExpires(Watchdog*);

/* ----------------------- STATE GENERATION -----------------------*/
#define STATE_GENERATION S(S_FORGET) S(S_IDLE) S(S_DETECTION) S(S_RUNNING) S(S_EMERGENCY) S(S_DEATH)
#define S(x) x,

/**
 * @brief State enumeration
 */
typedef enum {
    STATE_GENERATION
    STATE_NB
} State;

#undef S

#define S(x) #x,

/**
 * @brief List of states
 */
static const char * const kStateName[] = { STATE_GENERATION };
#undef STATE_GENERATION
#undef S

/**
 * @brief State name generator
 *
 * @param Index of the state
 *
 * @retval Name of the state
 */
static const char * stateGetName(int i) {
    return kStateName[i];
}

/* ----------------------- ACTION GENERATION -----------------------*/
#define ACTION_GENERATION   S(A_NONE) S(A_IDLE_DETECTION) S(A_IDLE_EMERGENCY) S(A_DETECTION_IDLE) S(A_DETECTION_RUNNING) \
                            S(A_RUNNING_IDLE) S(A_RUNNING_DETECTION) S(A_RUNNING_RUNNING) S(A_RUNNING_EMERGENCY) \
                            S(A_EMERGENCY_IDLE)// actions of your state machine
#define S(x) x,

/**
 * @brief Action enumeration
 */
typedef enum {
    ACTION_GENERATION
    ACTION_NB
} Action;

#undef S

#define S(x) #x,

/**
 * @brief List of actions
 */
static const char * const kActionName[] = { ACTION_GENERATION };
#undef ACTION_GENERATION
#undef S

/**
 * @brief Action name generator
 *
 * @param Index of the action
 *
 * @retval Name of the action
 */
static const char * actionGetName(int i) {
    return kActionName[i];
}

/**
 * @brief Function to call for a transition without any actions
 *
 * @retval no retval
 */
static void actionNone() {

}

/**
 * @brief Actions to do in the S_IDLE / S_DETECTION transition
 *
 * @retval no retval
 */
static void actionIdleDetection(VelocityVector vel) {
    pilot->bump = pilotHasBumped();
    pilotEvaluateBump();
}

/**
 * @brief Actions to do in the S_IDLE / S_EMERGENCY transition
 *
 * @retval no retval
 */
static void actionIdleEmergency(VelocityVector vel) {
    pilotSendMvt(vel);
}

/**
 * @brief Actions to do in the S_IDLE / S_THREE transition
 *
 * @retval no retval
 */
static void actionDetectionIdle(VelocityVector vel) {
    pilotSendMvt(vel);
}

/**
 * @brief Actions to do in the S_DETECTION / S_RUNNING transition
 *
 * @retval no retval
 */
static void actionDetectionRunning(VelocityVector vel) {
    pilotSetTimeout();
}

/**
 * @brief Actions to do in the S_RUNNING / S_IDLE transition
 *
 * @retval no retval
 */
static void actionRunningIdle(VelocityVector vel) {
    pilotSendMvt(vel);
    pilotResetTimeout();
}

/**
 * @brief Actions to do in the S_RUNNING / S_DETECTION  transition
 *
 * @retval no retval
 */
static void actionRunningDetection(VelocityVector vel) {
    pilot->bump = pilotHasBumped();
    pilotEvaluateBump();
}

/**
 * @brief Actions to do in the S_RUNNING / S_RUNNING transition
 *
 * @retval no retval
 */
static void actionRunningRunning(VelocityVector vel) {
    pilotSendMvt(vel);
    pilotEvaluateVelocity();
}

/**
 * @brief Actions to do in the S_RUNNING / S_EMERGENCY transition
 *
 * @retval no retval
 */
static void actionRunningEmergency(VelocityVector vel) {
    pilotSendMvt(vel);
}

/**
 * @brief Actions to do in the S_EMERGENCY / S_IDLE transition
 *
 * @retval no retval
 */
static void actionEmergencyIdle(VelocityVector vel) {
    pilotSendMvt(vel);
}

/**
 * @brief Action pointer
 */
typedef void (*ActionPtr)(VelocityVector);

/**
 * @brief List of functions to be pointed, one for each to transition [TO COMPLETE]
 */
static const ActionPtr ActionsTab[ACTION_NB] = {
        &actionNone, &actionIdleDetection, &actionIdleEmergency, &actionDetectionIdle,
        &actionDetectionRunning, &actionRunningIdle, &actionRunningDetection,
        &actionRunningRunning, &actionRunningEmergency, &actionEmergencyIdle
};

/* ----------------------- EVENT GENERATION -----------------------*/
#define EVENT_GENERATION S(E_VELOCITY) S(E_CHECK) S(E_BUMP) S(E_NOBUMP) S(E_TOGGLESTOP) S(E_STOP)
#define S(x) x,
/**
 * @brief Event enumeration
 */
typedef enum {
    EVENT_GENERATION
    EVENT_NB
} Event;

#undef S

#define S(x) #x,

/**
 * @brief List of events
 */
static const char * const kEventName[] = { EVENT_GENERATION };
#undef EVENT_GENERATION
#undef S

/**
 * @brief Event name generator
 *
 * @param Index of the event
 *
 * @retval Name of the event
 */
static const char * eventGetName(int i) {
    return kEventName[i];
}

/* ----------------------- STATE MACHINE -----------------------*/

/**
 * @brief Transition type
 */
typedef struct {
    State destinationState;
    Action action;
} Transition;

/**
 * @brief Current state of the state machine
 */
static State currentState = S_IDLE;

/**
 * @brief State machine that defines all transitions [TO COMPLETE]
 */
static Transition stateMachine[STATE_NB][EVENT_NB] = {
        [S_IDLE][E_VELOCITY] = {S_DETECTION, A_IDLE_DETECTION},
        [S_IDLE][E_TOGGLESTOP] = {S_EMERGENCY, A_IDLE_EMERGENCY},
        [S_DETECTION][E_BUMP] = {S_IDLE, A_DETECTION_IDLE},
        [S_DETECTION][E_NOBUMP] = {S_RUNNING, A_DETECTION_RUNNING},
        [S_RUNNING][E_STOP] = {S_IDLE, A_RUNNING_IDLE},
        [S_RUNNING][E_CHECK] = {S_DETECTION, A_RUNNING_DETECTION},
        [S_RUNNING][E_VELOCITY] = {S_RUNNING, A_RUNNING_RUNNING},
        [S_RUNNING][E_TOGGLESTOP] = {S_EMERGENCY, A_RUNNING_EMERGENCY},
        [S_EMERGENCY][E_TOGGLESTOP] = {S_IDLE, A_EMERGENCY_IDLE}
};

/**
 * @brief State machine checker
 *
 * Generates a .puml file to check if the correct state machine is running
 *
 * @retval no retval
 */

void pilotCheckSM() {
    int iState, iEvent;
    FILE *stdPlant;

    stdPlant = fopen("SMPilot.puml", "w+");

    fprintf(stdPlant, "%s\n", "@startuml");
    fprintf(stdPlant,"[*] -->%s\n", stateGetName(1));

    for(iState = 0; iState < STATE_NB; iState++) {
        for(iEvent = 0; iEvent < EVENT_NB; iEvent++) {
            if(stateMachine[iState][iEvent].destinationState != S_FORGET){
                fprintf(stdPlant,"%s --> %s: %s / %s \n",
                        stateGetName(iState),
                        stateGetName(stateMachine[iState][iEvent].destinationState),
                        eventGetName(iEvent),
                        actionGetName(stateMachine[iState][iEvent].action ) ) ;
            }
        }
    }

    fprintf(stdPlant, "%s\n", "@enduml");
    fclose(stdPlant);
}

/* ----------------------- MESSAGE QUEUE -----------------------*/
/**
 * @brief Message queue to handle events
 */
static mqd_t queue;

/**
 * @brief Name of the message queue
 */
static const char kQueueName[] = "/mqPilot";

/**
 * @brief Shape of the message to send/receive
 */
typedef struct {
    Event event;
    VelocityVector vel;
} MqMsg;

/**
 * @brief Wrapper of the message and a string
 */
typedef union {
    char toString[MQ_MSG_SIZE];
    MqMsg data;
} MqMsgWrapper;

/**
 * @brief Message queue's message receiver
 *
 * @retval message received
 */
static MqMsg pilotMqReceive() {
    MqMsgWrapper msg;

    mq_receive(queue, msg.toString, MQ_MSG_SIZE, 0);

    return msg.data;
}

/**
 * @brief Message queue's message sender
 *
 * @param Message to send
 *
 * @retval no retval
 */
static void pilotMqSend(MqMsg aMsg) {
    MqMsgWrapper msg;
    msg.data = aMsg;

    mq_send(queue, msg.toString, sizeof(msg.toString), 0);
}

/* ----------------------- RUN FUNCTION -----------------------*/
/**
 * @brief Pilot class runner
 *
 * Runs the state machine
 *
 * @retval no retval
 */
static void* pilotRun() {
    MqMsg msg;
    Action act;

    while (currentState != S_DEATH) {
        msg = pilotMqReceive();
        if (stateMachine[currentState][msg.event].destinationState != S_FORGET) {
            act = stateMachine[currentState][msg.event].action;
            ActionsTab[act](msg.vel);
            currentState = stateMachine[currentState][msg.event].destinationState;
        }
    }
    return(0);
}

/* ----------------------- NEW STOP START FREE -----------------------*/
/**
 * @brief Thread that will run the state machine
 */
static pthread_t runThread;

void pilotNew() {
    pilot = (Pilot *) malloc(sizeof(pilot));

    struct mq_attr mqa = {
            MQ_MSG_COUNT,
            MQ_MSG_SIZE,
            0
    };
    //mq_unlink(kQueueName);
    mq_open(kQueueName, O_CREAT | O_RDWR, 0777, &mqa);

    currentState = S_IDLE;

    pilot->watchdog = watchdogConstruct(TIMEOUT_BUMP, &pilotWdExpires);
    robotNew();
}

void pilotStart() {
    pthread_create(&runThread, NULL, pilotRun, NULL);
    robotStart();
}

void pilotStop() {
    pthread_join(runThread, NULL);

    MqMsg msg = {
            .event = E_STOP,
            .vel = velocityZero
    };
    pilotMqSend(msg);
    robotStop();
}

void pilotFree() {
    robotFree();
    mq_close(queue);
    mq_unlink(kQueueName);

    watchdogDestroy(pilot->watchdog);

    free(pilot);
}

Pilot pilotGetState() {
    pilot->speed = robotGetSpeed();
    pilot->luminosity = robotGetSensorState().luminosity;
    pilot->bump = (robotGetSensorState().collision == BUMPED) ? 1 : 0;
    return *pilot;
}

void pilotSetVelocity(VelocityVector vel) {
    MqMsg data = {
            .event = E_VELOCITY,
            .vel = vel
    };

    pilotMqSend(data);
}

void pilotCheck() {
    printf("Speed : %d\n", pilotGetState().speed);
    printf("Luminosity : %f\n", pilotGetState().luminosity);
    printf("Collision : %s\n", (pilotGetState().bump ? "yes" : "no"));
};

void pilotToggleEmergencyStop() {
    MqMsg data = {
            .event = E_TOGGLESTOP,
            .vel = velocityZero
    };

    pilotMqSend(data);
}

static void pilotSendMvt(VelocityVector vel) {
    switch(vel.dir)
    {
        case LEFT:
            robotSetWheelsVelocity(vel.power, -(vel.power));
            break;

        case RIGHT:
            robotSetWheelsVelocity(-(vel.power), vel.power);
            break;

        case FORWARD:
            robotSetWheelsVelocity(vel.power, vel.power);
            break;

        case BACKWARD:
            robotSetWheelsVelocity(-(vel.power), -(vel.power));
            break;
    }

    pilot->speed = robotGetSpeed();
}

static void pilotEvaluateVelocity() {
    pilot->speed = robotGetSpeed();

    if (pilot->speed == 0) {
        MqMsg data = {
                .event = E_STOP,
                .vel = velocityZero
        };

        pilotMqSend(data);
    }
}

static void pilotEvaluateBump() {
    if (pilot->bump) {
        MqMsg data = {
                .event = E_BUMP,
                .vel = velocityZero
        };

        pilotMqSend(data);
    } else {
        MqMsg data = {
                .event = E_NOBUMP,
                .vel = pilot->speed
        };

        pilotMqSend(data);
    }
}

static int pilotHasBumped() {
    return ((robotGetSensorState().collision == BUMPED) ? 1 : 0);
}

static void pilotSetTimeout() {
    watchdogStart(pilot->watchdog);
}

static void pilotResetTimeout() {
    watchdogCancel(pilot->watchdog);
}

static void pilotWdExpires(Watchdog *this) {
    MqMsg data = {
            .event = E_CHECK,
            .vel = pilot->speed
    };
    data.event = E_CHECK;
    pilotMqSend(data);
}