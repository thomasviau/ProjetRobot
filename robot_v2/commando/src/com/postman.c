#include "postman.h"
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
#include <sys/socket.h>
#include <netinet/in.h>

#include "../util.h"
#include "proxyUI.h"
#include "dispatcher.h"
#include "postman.h"

#define NAME_MQ  "/postmanCommandoBAL"
#define MQ_MSG_COUNT 10
#define MQ_MSG_SIZE 256
#define MSG_SIZE 256

#define SERVER_PORT (50100)
#define MAX_PENDING_CONNECTIONS 5

/* ----------------------- DEFINITION OF STRUCTURES -----------------------*/

typedef union {
    char buffer[MSG_SIZE];
    Param param;
} MsgAdapter;

struct PostmanCommandoT {};

int socketListen;
int socketData;
struct sockaddr_in myAddress;
static pthread_t thread;

/* ----------------------- STATE GENERATION -----------------------*/
#define STATE_GENERATION S(S_FORGET) S(S_IDLE) S(S_INIT) S(S_CONNECT) S(S_RUNNING) S(S_DEATH) // states of your state machine
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
#define ACTION_GENERATION S(A_NOP) S(A_INIT) S(A_CONNECT) S(A_CREATE_READ_THREAD) S(A_SEND_MSG) S(A_STOP) S(A_ERROR) // actions of your state machine
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
static void actionNop(Param param) {
    //noting to do
}

/**
 * @brief Actions to do in the S_IDLE / S_INIT transition
 *
 * @retval no retval
 */
static void actionInit(Param param) {
    socketListen = socket(AF_INET, SOCK_STREAM, 0);
    STOP_ON_ERROR(socketListen < 0);
    myAddress.sin_family = AF_INET;
    myAddress.sin_port = htons(SERVER_PORT);
    myAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    TRACE("\n");
}

/**
 * @brief Actions to do in the S_INIT / S_CONNECT transition
 *
 * @retval no retval
 */
static void actionConnect(Param param) {
    int err = bind(socketListen, (struct sockaddr *)&myAddress, sizeof(myAddress));
    STOP_ON_ERROR(err < 0);

    listen(socketListen, MAX_PENDING_CONNECTIONS);
    printf("Socket server started\n");

    printf( "Waiting for new client...\n" );
    socketData = accept(socketListen, NULL, 0);
    STOP_ON_ERROR(socketData < 0);
    printf( "Opened new communication with client\n" );
}

/**
 * @brief Thread that read
 *
 * @retval no retval
 */
static void* actionReceiveMsg(void){
    MsgAdapter msgAdapter;
    while (1){
        int err = read(socketData, &msgAdapter.buffer, sizeof(msgAdapter.buffer));
        STOP_ON_ERROR(err < 0);
        dispatcherCommandoDecode(msgAdapter.param);
        if (msgAdapter.param.idMethod == -1){
            break;
        }
    }
    return 0;
}

/**
 * @brief Actions to do in the S_CONNECT / S_RUNNING transition
 *
 * @retval no retval
 */
static void actionCreateReadThread(Param param) {
    int err = pthread_create(&thread, NULL, actionReceiveMsg, NULL);
    STOP_ON_ERROR(err == -1);
}

/**
 * @brief Actions to do in the S_RUNNING / S_RUNNING transition
 *
 * @retval no retval
 */
static void actionSendMsg(Param param){
    MsgAdapter msgAdapter;
    msgAdapter.param = param;
    int err = write(socketData, &msgAdapter.buffer, sizeof(msgAdapter.buffer));
    STOP_ON_ERROR(err < 0);
}

/**
 * @brief Actions to do in the * / S_DEATH transition
 *
 * @retval no retval
 */
static void actionStop(Param param) {
    if (socketListen)
        close(socketListen);
    if (socketData)
        close(socketData);
}

/**
 * @brief Actions to do in the * / S_DEATH transition
 *
 * @retval no retval
 */
static void actionError(Param param) {
    actionStop(param);
}

/**
 * @brief Action pointer
 */
typedef void (*ActionPtr)(Param);

/**
 * @brief List of functions to be pointed, one for each to transition [TO COMPLETE]
 */
static const ActionPtr ActionsTab[ACTION_NB] = {&actionNop, &actionInit, &actionConnect, &actionCreateReadThread, &actionSendMsg, &actionStop, &actionError};

/* ----------------------- EVENT GENERATION -----------------------*/
#define EVENT_GENERATION S(E_NOP) S(E_INIT) S(E_CONNECT) S(E_CREATE_READ_THREAD) S(E_SEND_MSG) S(E_STOP) S(E_ERROR) // events of your state machine
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
 * @brief State machine that defines all transitions
 */
static Transition stateMachine[STATE_NB][EVENT_NB] = {
        [S_IDLE][E_INIT]= {S_INIT, A_INIT},
        [S_INIT][E_CONNECT]={S_CONNECT, A_CONNECT},
        [S_CONNECT][E_CREATE_READ_THREAD]= {S_RUNNING, A_CREATE_READ_THREAD},
        [S_RUNNING][E_SEND_MSG]= {S_RUNNING, A_SEND_MSG},
        [S_IDLE][E_ERROR]= {S_DEATH, A_ERROR},
        [S_INIT][E_ERROR]= {S_DEATH, A_ERROR},
        [S_CONNECT][E_ERROR]= {S_DEATH, A_ERROR},
        [S_RUNNING][E_ERROR]= {S_DEATH, A_ERROR},
        [S_IDLE][E_STOP]= {S_DEATH, A_STOP},
        [S_INIT][E_STOP]= {S_DEATH, A_STOP},
        [S_CONNECT][E_STOP]= {S_DEATH, A_STOP},
        [S_RUNNING][E_STOP]= {S_DEATH, A_STOP}
};

/**
 * @brief State machine checker
 *
 * Generates a .puml file to check if the correct state machine is running
 *
 * @retval no retval
 */

static void postmanCommandoCheckSM(void) {
    int iState, iEvent;
    FILE *stdPlant;

    stdPlant = fopen("MAE_PostmanCommando.puml", "w+");

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
 * @brief Shape of the message to send/receive
 */
typedef struct{
    Event event;
    Param param;
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
static MqMsg postmanCommandoMqReceive(void) {
    MqMsgWrapper msg;

    //int err = mq_open(NAME_MQ, O_RDONLY);
    //STOP_ON_ERROR(err == -1);

    int err = mq_receive(queue, msg.toString, MQ_MSG_SIZE, 0);
    STOP_ON_ERROR(err != sizeof (msg.toString));

    //mq_close(queue);
    return msg.data;
}

/**
 * @brief Message queue's message sender
 *
 * @param Message to send
 *
 * @retval no retval
 */
static void postmanCommandoMqSend(MqMsg aMsg) {
    MqMsgWrapper msg = {.data = aMsg};

    //int err = mq_open(NAME_MQ, O_WRONLY);
    //STOP_ON_ERROR(err == -1);

    int err = mq_send(queue, msg.toString, sizeof(msg.toString), 0);
    STOP_ON_ERROR(err);

    //mq_close(queue);
}

/* ----------------------- RUN FUNCTION -----------------------*/
/**
 * @brief postmanCommando class runner
 *
 * Runs the state machine
 *
 * @retval no retval
 */
static void* postmanCommandoRun(void) {
    MqMsg msg;
    Action act;

    while (currentState != S_DEATH) {
        msg = postmanCommandoMqReceive();
        if (stateMachine[currentState][msg.event].destinationState == S_FORGET) {
        }
        else {
            act = stateMachine[currentState][msg.event].action;
            ActionsTab[act](msg.param);
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


/**
 * @brief Instance of the postmanCommando class
 */
static PostmanCommando *this;

void postmanCommandoNew(void) {
    dispatcherCommandoNew();

    this = (PostmanCommando *) malloc(sizeof(this));

    struct mq_attr mqa = {
            .mq_maxmsg = MQ_MSG_COUNT,
            .mq_msgsize = MQ_MSG_SIZE,
            .mq_flags = 0
    };
    //mq_unlink(NAME_MQ);
    queue = mq_open (NAME_MQ, O_CREAT | O_RDWR, 0666, &mqa);

    STOP_ON_ERROR(queue == -1);

    //mq_close(queue);

    currentState = S_IDLE;
    TRACE("New instance\n");
}

void postmanCommandoStart(void) {
    dispatcherCommandoStart();

    pthread_create(&runThread, NULL, postmanCommandoRun, NULL);
    TRACE("Start instance\n");
}

void postmanCommandoStop(void) {
    dispatcherCommandoStop();

    pthread_join(runThread, NULL);

    MqMsg msg = {
            .event = E_STOP
    };
    postmanCommandoMqSend(msg);
    TRACE("Stop instance\n");
}

void postmanCommandoFree(void) {
    dispatcherCommandoFree();

    mq_close (queue);
    mq_unlink(NAME_MQ);

    free(this);
    TRACE("Free instance\n");
}

void postmanCommandoSend(Param param){
    MqMsg mqMsg={
            .event = E_SEND_MSG,
            .param = param
    };
    postmanCommandoMqSend(mqMsg);
}
