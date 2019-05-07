#include "remoteUI.h"
#include "../com/proxyPilot.h";

#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>

#define POWER 50

/* Variable de sortie du programme. */
static int keepGoing = 1;
/* Booléen pour l'affichage des logs */
int logs = 0;


static int getch(void) {
    int c = 0;

    struct termios org_opts, new_opts;
    int res = 0;

    //-----  store old settings -----------
    res = tcgetattr(STDIN_FILENO, &org_opts);
    assert(res == 0);

    //---- set new terminal parms --------
    memcpy(&new_opts, &org_opts, sizeof(new_opts));
    new_opts.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL | ECHOPRT | ECHOKE | ICRNL);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_opts);
    c = getchar();

    //------  restore old settings ---------
    res = tcsetattr(STDIN_FILENO, TCSANOW, &org_opts);
    assert(res == 0);
    return(c);
}

void remoteUINew() {

}

void remoteUIStart() {
    remoteUIRun();
}

void remoteUIStop() {

}

void remoteUIFree() {

}

void remoteUIRun()
{
    remoteUIDisplay();
    while(keepGoing)
    {
        remoteUICaptureChoice();
    }
}

void remoteUIDisplay()
{
    printf("Projet Robot - Thomas VIAU \n");
}

void remoteUICaptureChoice()
{
    VelocityVector vel = {LEFT, 0};

    switch( toupper(getch()) )
    {
        case 'Z':
            vel.dir = FORWARD;
            vel.power = POWER;
            pilotSetVelocity(vel);
            break;

        case 'S':
            vel.dir = BACKWARD;
            vel.power = POWER;
            pilotSetVelocity(vel);
            break;

        case 'Q':
            vel.dir = LEFT;
            vel.power = POWER;
            pilotSetVelocity(vel);
            break;

        case 'D':
            vel.dir = RIGHT;
            vel.power = POWER;
            pilotSetVelocity(vel);
            break;

        case 'E':
            remoteUIAskClearLog();
            break;

        case 'R':
            remoteUIAsk4Log();
            break;

        case 'A':
            keepGoing = 0;
            break;

        default:
            pilotSetVelocity(vel);
    }
    if (logs)
        pilotCheck();
}

void remoteUIAsk4Log() {
    if (!logs)
    {
        logs = 1;
        printf("Voici les logs \n");
    }
}

void remoteUIAskClearLog() {
    if (logs)
    {
        logs = 0;
        printf("Fin des logs\n");
    }
}