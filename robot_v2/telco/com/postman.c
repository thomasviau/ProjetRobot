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
#include "proxyPilot.h"
#include "dispatcher.h"
#include "postman.h"

#define NAME_MQ  "/postmanTelcoBAL"
#define MQ_MSG_COUNT 10
#define MQ_MSG_SIZE 256
#define MSG_SIZE 256

#define SERVER_PORT (50100)
#define MAX_PENDING_CONNECTIONS 5

/* ----------------------- DEFINITION OF STRUCTURES -----------------------*/

union MsgAdapterT{
    char buffer[MSG_SIZE];
    int msg;
};

struct PostmanTelcoT {};

union ParamT {
    int value;
};

int socketListen;
int socketData;
struct sockaddr_in myAddress;
static pthread_t thread;

/* ----------------------- ACTION GENERATION -----------------------*/
/**
 * @brief Actions to do in the S_IDLE / S_INIT transition
 *
 * @retval no retval
 */
static void actionInit(void) {
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
static void actionConnect(void) {
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
 * @brief Actions to do in the S_CONNECT / S_RUNNING transition
 *
 * @retval no retval
 */
static void actionCreateReadThread(void) {
    int err = pthread_create(&thread, NULL, actionReceiveMsg, NULL);
    STOP_ON_ERROR(err == -1);
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
        printf("Received: %d\n", msgAdapter.msg);
        //dispatcherTelcoDecode(msgAdapter.msg);

        if (msgAdapter.msg == 0){
            break;
        }
    }
}

/**
 * @brief Actions to do in the S_RUNNING / S_RUNNING transition
 *
 * @retval no retval
 */
static void actionSendMsg(int msg){
    MsgAdapter msgAdapter;
    msgAdapter.msg = msg;
    int err = write(socketData, &msgAdapter.buffer, sizeof(msgAdapter.buffer));
    STOP_ON_ERROR(err < 0);
    printf("Sent: %d\n", msgAdapter.msg);
}

/**
 * @brief Actions to do in the * / S_DEATH transition
 *
 * @retval no retval
 */
static void actionError(void) {
    actionStop();
}

/**
 * @brief Actions to do in the * / S_DEATH transition
 *
 * @retval no retval
 */
static void actionStop(void) {
    if (socketListen)
        close(socketListen);
    if (socketData)
        close(socketData);
}

/**
 * @brief Action pointer
 */
typedef void (*ActionPtr)();

/**
 * @brief List of functions to be pointed, one for each to transition [TO COMPLETE]
 */
static const ActionPtr ActionsTab[ACTION_NB] = {&actionNop, &actionInit, &actionConnect, &actionCreateReadThread, &actionSendMsg, &actionStop, &actionError};


/* ----------------------- MESSAGE QUEUE -----------------------*/
/**
 * @brief Message queue to handle events
 */
static mqd_t queue;

/**
 * @brief Shape of the message to send/receive
 */
typedef struct {
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
static MqMsg postmanTelcoMqReceive(void) {
    MqMsgWrapper msg;

    //int err = mq_open(NAME_MQ, O_RDONLY);
    //STOP_ON_ERROR(err == -1);

    int err = mq_receive(queue, msg.toString, MQ_MSG_SIZE, 0);
    STOP_ON_ERROR(err != sizeof (msg.toString));

    //mq_close(queue);

    printf("queue message sent : %d\n",msg.data.param.value);

    return msg.data;
}

/**
 * @brief Message queue's message sender
 *
 * @param Message to send
 *
 * @retval no retval
 */
static void postmanTelcoMqSend(MqMsg aMsg) {
    MqMsgWrapper msg = {.data = aMsg};

    //int err = mq_open(NAME_MQ, O_WRONLY);
    //STOP_ON_ERROR(err == -1);

    int err = mq_send(queue, msg.toString, sizeof(msg.toString), 0);
    STOP_ON_ERROR(err);

    //mq_close(queue);

    printf("queue message sent : %d\n",msg.data.param.value);
}

/* ----------------------- RUN FUNCTION -----------------------*/
/**
 * @brief postmanTelco class runner
 *
 * Runs the state machine
 *
 * @retval no retval
 */
static void* postmanTelcoRun(void) {
    MqMsg msg;
    Action act;

    while (currentState != S_DEATH) {
        msg = postmanTelcoMqReceive();
        if (stateMachine[currentState][msg.event].destinationState == S_FORGET) {
        }
        else {
            act = stateMachine[currentState][msg.event].action;
            ActionsTab[act](msg.param.value);
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
 * @brief Instance of the postmanTelco class
 */
static PostmanTelco *this;

void postmanTelcoNew(void) {
    dispatcherTelcoNew();

    this = (PostmanTelco *) malloc(sizeof(this));

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

void postmanTelcoStart(void) {
    dispatcherTelcoStart();

    pthread_create(&runThread, NULL, postmanTelcoRun, NULL);
    TRACE("Start instance\n");
}

void postmanTelcoStop(void) {
    dispatcherTelcoStop();

    pthread_join(runThread, NULL);

    MqMsg msg = {
            .event = E_STOP
    };
    postmanTelcoMqSend(msg);
    TRACE("Stop instance\n");
}

void postmanTelcoFree(void) {
    dispatcherTelcoFree();

    mq_close (queue);
    mq_unlink(NAME_MQ);

    free(this);
    TRACE("Free instance\n");
}