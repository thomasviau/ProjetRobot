#include <stdlib.h>
#include <stdio.h>
#include "robot/pilot.h"

int main (int argc, char *argv[]){
    pilotNew();
    pilotStart();
    pilotStop();
    pilotFree();
    return 0;
}