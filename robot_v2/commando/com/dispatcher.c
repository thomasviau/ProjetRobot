#include <stdbool.h>
#include <stdlib.h>

#include "../util.h"
#include "dispatcher.h"

struct DispatcherCommandoT{};

DispatcherCommando *dispatcherCommando;

void dispatcherCommandoNew(void) {
    dispatcherCommando = (DispatcherCommando*)malloc (sizeof(DispatcherCommando));
    STOP_ON_ERROR(dispatcherCommando == NULL);
}

void dispatcherCommandoStart(void) {

}

void dispatcherCommandoStop(void) {

}

void dispatcherCommandoFree(void) {
    free(dispatcherCommando);
}

void dispatcherCommandoDecode(int msg) {
    printf("Message a décoder :%d\n", msg);
}