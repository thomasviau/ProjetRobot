#include <stdbool.h>
#include <stdlib.h>

#include "../util.h"
#include "dispatcher.h"

#define NAME_MQ  "/dispatcheCommandoBAL"
#define MQ_MSG_COUNT 10
#define MQ_MSG_SIZE 256

/* ----------------------- MESSAGE QUEUE -----------------------*/
/**
 * @brief Message queue to handle events
 */
static mqd_t queue;

/**
 * @brief Shape of the message to send/receive
 */
struct MqMsgT{
    Event event;
    Param param;
};

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
static MqMsg dispatcheCommandoMqReceive(void) {
    MqMsgWrapper msg;

    int err = mq_receive(queue, msg.toString, MQ_MSG_SIZE, 0);
    STOP_ON_ERROR(err != sizeof (msg.toString));

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
static void dispatcheCommandoMqSend(MqMsg aMsg) {
    MqMsgWrapper msg = {.data = aMsg};

    int err = mq_send(queue, msg.toString, sizeof(msg.toString), 0);
    STOP_ON_ERROR(err);

    printf("queue message sent : %d\n",msg.data.param.value);
}

/* ----------------------- RUN -----------------------*/

void *dispatcherRun(void){
    MqMsg msg;

    while (1) {
        msg = dispatcheCommandoMqReceive();
        dispatcherCommandoDecode(msg);
    }
}

/* ----------------------- NEW START STOP FREE -----------------------*/

struct DispatcherCommandoT{};

pthread_t runThread;

DispatcherCommando *dispatcherCommando;

void dispatcherCommandoNew(void) {
    dispatcherCommando = (DispatcherCommando*)malloc (sizeof(DispatcherCommando));
    STOP_ON_ERROR(dispatcherCommando == NULL);
}

void dispatcherCommandoStart(void) {
    pthread_create(&runThread, NULL, dispatcherRun, NULL);
    TRACE("Start instance\n");
}

void dispatcherCommandoStop(void) {

}

void dispatcherCommandoFree(void) {
    free(dispatcherCommando);
}

void dispatcherCommandoDecode(Param param) {
    switch(param.idMethod){
        case 1:
            askEvents(param.from,param.to);
            break;
        case 2:
            askEventsCount(param.indice);
            break;
        case 3:
            setVelocity(param.vel);
            break;
        case 4:
            toggleEmergencyStop();
            break;
        case 5:
            askPilotState();
            break;
    }
}

void askEvents(int from, int to){
    //TODO
}

void askEventsCount(int indice){
    //TODO
}

void setVelocity(VelocityVector vel){
    pilotSetVelocity(vel);
}

void toggleEmergencyStop(void){
    pilotToggleEmergencyStop();
}

void askPilotState(void){
    pilotAskPilotState();
}