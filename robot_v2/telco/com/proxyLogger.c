#include "postman.h"
#include "proxyLogger.h"

void proxyLoggerEncode(Param param){
    postmanTelcoSend(param);
}

void askEvents(int from, int to){
    Param param = {
            .idMethod = 1,
            .from = from,
            .to = to
    };
    proxyLoggerEncode(param);
}

void askEventsCount(int indice){
    Param param = {
            .idMethod = 2,
            .indice = indice
    };
    proxyLoggerEncode(param);
}