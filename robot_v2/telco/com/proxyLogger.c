#include "postman.h"
#include "proxyLogger.h"

void proxyLoggerEncode(Param param){
    postmanTelcoSend(param);
}

void askLogs(int from, int to){
    Param param = {
            .idMethod = 1,
            .from = from,
            .to = to
    };
    proxyLoggerEncode(param);
}

void askLogsCount(){
    Param param = {
            .idMethod = 2
    };
    proxyLoggerEncode(param);
}