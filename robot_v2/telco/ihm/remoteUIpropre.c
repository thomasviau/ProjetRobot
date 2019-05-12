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
#include <termio.h>
#include <memory.h>
#include <assert.h>
#include <ctype.h>

#include "remoteUIpropre.h"
#include "../com/proxyPilot.h"
#include "../com/proxyLogger.h"
#include "../../commando/watchdog.h"

#define TIMEOUT_LOG (1*1000*1000)

#define MQ_MSG_COUNT 10
#define MQ_MSG_SIZE 256

static VelocityVector myVel;
static Watchdog *watchdog;

static int currentLogsCount;
static int previousLogsCount;
static Log myLogs[];

/* ----------------------- STATE GENERATION -----------------------*/
#define STATE_GENERATION S(S_FORGET) S(S_MAIN) S(S_LOG) S(S_WAITING) S(S_DEATH) // states of your state machine
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
#define ACTION_GENERATION S(A_NOP) S(A_MAIN) S(A_ASKLOG) S(A_CLEARLOG) S(A_TOLOG) S(A_STOP)// actions of your state machine
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
 * @param[in] Structure of parameters
 *
 * @retval no retval
 */
static void actionNone() {

}

/**
 * @brief Actions to do in the S_IDLE / S_TWO transition
 *
 * @param[in] Structure of parameters
 *
 * @retval no retval
 */
static void actionMain() {
    watchdogCancel(watchdog);
    remoteUIDisplay(MAIN_SCREEN);
}

/**
 * @brief Actions to do in the S_IDLE / S_THREE transition
 *
 * @param[in] Structure of parameters
 *
 * @retval no retval
 */
static void actionAskLog() {
    remoteUIDisplay(LOG_SCREEN);
    watchdogStart(watchdog);
}

/**
 * @brief Actions to do in the S_TWO / S_THREE transition
 *
 * @param[in] Structure of parameters
 *
 * @retval no retval
 */
static void actionClearLog() {

}

static void actionTOLog() {
    askLogsCount();
    askLogs(previousLogsCount, currentLogsCount);
    previousLogsCount = currentLogsCount;
    displayLogs(myLogs);
}

/**
 * @brief Action pointer
 */
typedef void (*ActionPtr)();

/**
 * @brief List of functions to be pointed, one for each to transition [TO COMPLETE]
 */
static const ActionPtr ActionsTab[ACTION_NB] = {&actionNone, &actionMain, &actionAskLog, &actionClearLog, &actionTOLog};

/* ----------------------- EVENT GENERATION -----------------------*/
#define EVENT_GENERATION S(E_ASKLOG) S(E_CLEARLOG) S(E_BACKMAIN) S(E_TOLOG) S(E_STOP) // events of your state machine
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
static State currentState;

/**
 * @brief State machine that defines all transitions [TO COMPLETE]
 */
static Transition stateMachine[STATE_NB][EVENT_NB] = {
        [S_MAIN][E_ASKLOG]= {S_LOG, A_ASKLOG},
        [S_MAIN][E_STOP]={S_DEATH, A_STOP},
        [S_LOG][E_ASKLOG]= {S_LOG, A_ASKLOG},
        [S_LOG][E_BACKMAIN]= {S_MAIN, A_MAIN},
        [S_LOG][E_CLEARLOG]= {S_LOG, A_CLEARLOG},
        [S_LOG][E_TOLOG]= {S_WAITING, A_TOLOG},
        [S_LOG][E_STOP]= {S_DEATH, A_STOP},
        [S_WAITING][E_ASKLOG]= {S_LOG, A_ASKLOG}
};

/**
 * @brief State machine checker
 *
 * Generates a .puml file to check if the correct state machine is running
 *
 * @retval no retval
 */

static void remoteUICheckSM() {
    int iState, iEvent;
    FILE *stdPlant;

    stdPlant = fopen("SMRemoteUI.puml", "w+");

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
static const char kQueueName[] = "/mqActive";

/**
 * @brief Shape of the message to send/receive
 */
typedef struct {
    Event event;
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
static MqMsg remoteUIMqReceive() {
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
static void remoteUIMqSend(MqMsg aMsg) {
    MqMsgWrapper msg;
    msg.data = aMsg;

    mq_send(queue, msg.toString, sizeof(msg.toString), 0);
}

/* ----------------------- RUN FUNCTION -----------------------*/
/**
 * @brief Class runner
 *
 * Runs the state machine
 *
 * @retval no retval
 */
static void* remoteUIRun() {
    MqMsg msg;
    Action act;

    while (currentState != S_DEATH) {
        msg = remoteUIMqReceive();
        if (stateMachine[currentState][msg.event].destinationState == S_FORGET) {

        }
        else {
            act = stateMachine[currentState][msg.event].action;
            ActionsTab[act]();
            currentState = stateMachine[currentState][msg.event].destinationState;
        }
    }
    return(0);
}

/* ----------------------- READ BUFFER -----------------------*/

static VelocityVector translate(Direction dir) {
    myVel.dir = dir;
    myVel.power = 50;

    return myVel;
}

/**
 * @brief Reads buffer
 *
 * @author Jim McNamara
 *
 * @see https://www.linuxquestions.org/questions/programming-9/getchar-question-128140/
 *
 * @retval no retval
 */
static int getch(void) {
    int c = 0;

    struct termios org_opts, new_opts;
    int res = 0;

    //-----  store old settings -----------
    res = tcgetattr(STDIN_FILENO, &org_opts);
    assert(res == 0);

    //---- set new terminal parms --------
    memcpy(&new_opts, &org_opts, sizeof(new_opts));
    new_opts.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL | ECHOPRT | ECHOKE | ICRNL);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_opts);
    c = getchar();

    //------  restore old settings ---------
    res = tcsetattr(STDIN_FILENO, TCSANOW, &org_opts);
    assert(res == 0);
    return(c);
}

/**
 * @brief Class reader
 *
 * Reads the buffer
 *
 * @retval no retval
 */
static void* remoteUIRead() {
    while (currentState != S_DEATH) {
        switch( toupper(getch()) ) {
            case 'Z':
                setVelocity(translate(FORWARD));
                break;

            case 'S':
                setVelocity(translate(BACKWARD));
                break;

            case 'Q':
                setVelocity(translate(LEFT));
                break;

            case 'D':
                setVelocity(translate(RIGHT));
                break;

            case 'E':
                remoteUIAskClearLog();
                break;

            case 'R':
                remoteUIAsk4Log();
                break;

            case 'A':
                remoteUIStop();
                break;

            default:
                myVel.power = 0;
                setVelocity(myVel);
        }
    }
    return(0);
}

/* ----------------------- NEW STOP START FREE -----------------------*/
/**
 * @brief Thread that will run the state machine
 */
static pthread_t runThread;

/**
 * @brief Thread that will read the characters
 */
static pthread_t readThread;

static void remoteUIWdExpires(Watchdog *this) {
    MqMsg msg = {
            .event = E_TOLOG
    };
    remoteUIMqSend(msg);
}

void remoteUINew() {
    previousLogsCount = 0;
    
    struct mq_attr mqa = {
            .mq_maxmsg = MQ_MSG_COUNT,
            .mq_msgsize = MQ_MSG_SIZE
    };
    mq_unlink(kQueueName);
    mq_open(kQueueName, O_CREAT | O_RDWR, 0777, &mqa);

    watchdog = watchdogConstruct(TIMEOUT_LOG, &remoteUIWdExpires);

    currentState = S_MAIN;
}

void remoteUIStart() {
    pthread_create(&readThread, NULL, remoteUIRead, NULL);
    pthread_create(&runThread, NULL, remoteUIRun, NULL);
}

void remoteUIStop() {
    MqMsg msg = {
            .event = E_STOP
    };
    remoteUIMqSend(msg);

    pthread_join(runThread, NULL);
    pthread_join(readThread, NULL);
}

void remoteUIFree() {
    mq_close(queue);
    mq_unlink(kQueueName);
}

void remoteUIAsk4Log() {
    MqMsg msg = {
            .event = E_ASKLOG
    };
    remoteUIMqSend(msg);
}

void remoteUIAskClearLog() {
    MqMsg msg = {
            .event = E_CLEARLOG
    };
    remoteUIMqSend(msg);
}

void remoteUIDisplay(IdScreen id) {
    switch(id) {
        case MAIN_SCREEN:
            printf("Projet robot 2.1 : Quentin Ageneau & Thomas Viau\n");
            break;

        case LOG_SCREEN:
            printf("Voici les logs\n");
            break;
    }
}

void setLogsCount(int count) {
    currentLogsCount = count;
    MqMsg msg = {
            .event = E_ASKLOG
    };
    remoteUIMqSend(msg);
}

void setLogs(Log logs[]) {
    for (int i = 0; i < currentLogsCount - previousLogsCount; i++) {
        myLogs[i] = logs[i];
    }

    MqMsg msg = {
            .event = E_ASKLOG
    };
    remoteUIMqSend(msg);
}

void displayLogs() {
    for (int i = 0; i < currentLogsCount - previousLogsCount; i++) {
        printf("Logs\n"); // TODO
    }
}