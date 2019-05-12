#include "proxyUI.h"
#include "postman.h"
#include "../robot/logger.h"
#include "../util.h"

void proxyUIEncode(Param param){
    postmanCommandoSend(param);
}

void setLogs(int from, int to){
    TRACE(setLogs);
    Param param = {
            .idMethod = 2,
            .from = from,
            .to = to
    };
    proxyUIEncode(param);
}

void setLogsCount(int indice){
    TRACE(setLogsCount);
    Param param = {
            .idMethod = 3,
            .indice = indice
    };
    proxyUIEncode(param);
}