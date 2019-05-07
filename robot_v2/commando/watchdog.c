/**
 * @file compute_pi.c
 *
 * @author team FORMATO, ESEO
 *
 * @section License
 *
 * The MIT License
 *
 * Copyright (c) 2016, Jonathan ILIAS-PILLET (ESEO)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include "watchdog.h"
#include "util.h"
#include <malloc.h>
#include <time.h>
#include <signal.h>

struct WatchdogT {
    timer_t myTimer; /**< POSIX Timer*/
    uint32_t myDelay; /**< configured delay */
    WatchdogCallback myCallback; /**< function to be called at delay expiration */
};

/**
 * Calls the watchdog callback when the delay of the timer expires
 *
 * @param handlerParam must be the watchdog reference
 */
static void mainHandler (union sigval handlerParam) {
    // Utilisation de l'union sigval qui stocke le watchdog
    Watchdog *theWatchdog = handlerParam.sival_ptr;
    // Appel de la fonction stockée dans le paramètre callback
    theWatchdog->myCallback(theWatchdog);
}

Watchdog *watchdogConstruct (uint32_t thisDelay, WatchdogCallback callback) {
    Watchdog *result;
    int error_code;

    // allocates and initializes the watchdog's attributes
    result = (Watchdog *)malloc (sizeof(Watchdog));
    STOP_ON_ERROR(result == NULL);
    result->myDelay = thisDelay;
    result->myCallback = callback;

    // initialisation des paramètres du sigevent
    struct sigevent sev = {
            .sigev_notify = SIGEV_THREAD, // pour pouvoir appeler une fonction à la fin du timer
            .sigev_value.sival_ptr = result, // donnée passée lors de la notification (ici le pointeur de watchdog)
            .sigev_notify_function = &mainHandler // fonction à appeler lors de la notification
    };

    // Création du timer avec les paramètres du sigevent
    error_code = timer_create(CLOCK_REALTIME, &sev, &(result->myTimer));
    STOP_ON_ERROR(error_code == -1);

    return result;
}

void watchdogStart(Watchdog *this) {
    int error_code;

    struct itimerspec its;

    // Définition de la valeur d'expiration en fonction du delay du watchdog (en microsecondes)
    its.it_value.tv_sec = this->myDelay/(1000*1000); // partie en secondes
    its.it_value.tv_nsec = (this->myDelay%(1000*1000))*1000; // partie en nanosecondes (x1000 car on passe de micro à nano)

    // Pas d'intervalle
    its.it_interval.tv_nsec = 0;
    its.it_interval.tv_sec = 0;

    // Démarrage du timer
    error_code = timer_settime(this->myTimer, 0, &its, NULL);
    STOP_ON_ERROR(error_code == -1);
}

void watchdogCancel(Watchdog *this) {
    int error_code;

    // Désarmement du timer
    struct itimerspec its;

    its.it_value.tv_sec = 0;
    its.it_value.tv_nsec = 0;

    its.it_interval.tv_nsec = 0;
    its.it_interval.tv_sec = 0;

    error_code = timer_settime(this->myTimer, 0, &its, NULL);
    STOP_ON_ERROR(error_code == -1);
}

void watchdogDestroy(Watchdog *this) {
    int error_code;

    // Désarmement du timer
    watchdogCancel(this);
    // Supression du timer
    error_code = timer_delete(this->myTimer);
    STOP_ON_ERROR(error_code == -1);

    // then we can free memory
    free (this);
}
