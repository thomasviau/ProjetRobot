#include "proxyPilot.h"

static void encode();

void pilotSetVelocity(VelocityVector vel) {

}

void pilotCheck() {
    printf("Speed : %d\n", pilotGetState().speed);
    printf("Luminosity : %f\n", pilotGetState().luminosity);
    printf("Collision : %s\n", (pilotGetState().bump ? "yes" : "no"));
};

static void encode() {

}