#include <stdlib.h>
#include <stdio.h>
#include "ihm/remoteUI.h"

int main (int argc, char *argv[]){
    remoteUINew();
    remoteUIStart();
    remoteUIStop();
    remoteUIFree();
    return 0;
}