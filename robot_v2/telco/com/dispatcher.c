#include <stdbool.h>
#include <stdlib.h>

#include "../util.h"
#include "dispatcher.h"

struct DispatcherTelcoT{};

DispatcherTelco *dispatcherTelco;

void dispatcherTelcoNew(void){
    dispatcherTelco = (DispatcherTelco *)malloc (sizeof(DispatcherTelco));
    STOP_ON_ERROR(dispatcherTelco == NULL);
}

void dispatcherTelcoStart(void){}

void dispatcherTelcoStop(void){}

void dispatcherTelcoFree(void){
    free(dispatcherTelco);
}

void dispatcherTelcoDecode(int msg){
    printf("Message a décoder :%d\n", msg);
}