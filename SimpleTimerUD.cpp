/*
 * SimpleTimerUD.cpp
 *
 * SimpleTimerUD - A timer library for Arduino. (now with
 * userData in timer callback)
 * Author: ajlennon@dynamicdevices.co.uk
 * Author: mromani@ottotecnica.com
 * Copyright (c) 2010 OTTOTECNICA Italy
 *
 * This library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser
 * General Public License as published by the Free Software
 * Foundation; either version 2.1 of the License, or (at
 * your option) any later version.
 *
 * This library is distributed in the hope that it will
 * be useful, but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser
 * General Public License along with this library; if not,
 * write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */


#include "SimpleTimerUD.h"


// Select time function:
//static inline unsigned long elapsed() { return micros(); }
static inline unsigned long elapsed() { return millis(); }


SimpleTimerUD::SimpleTimerUD() {
    unsigned long current_millis = elapsed();

    for (int i = 0; i < MAX_TIMERS; i++) {
        enabled[i] = false;
        callbacks[i] = 0;                   // if the callback pointer is zero, the slot is free, i.e. doesn't "contain" any timer
        prev_millis[i] = current_millis;
        numRuns[i] = 0;
    }

    numTimers = 0;
}


void SimpleTimerUD::run() {
    int i;
    unsigned long current_millis;

    // get current time
    current_millis = elapsed();

    for (i = 0; i < MAX_TIMERS; i++) {

        toBeCalled[i] = DEFCALL_DONTRUN;

        // no callback == no timer, i.e. jump over empty slots
        if (callbacks[i]) {

            // is it time to process this timer ?
            // see http://arduino.cc/forum/index.php/topic,124048.msg932592.html#msg932592

            if (current_millis - prev_millis[i] >= delays[i]) {

                // update time
                //prev_millis[i] = current_millis;
                prev_millis[i] += delays[i];

                // check if the timer callback has to be executed
                if (enabled[i]) {

                    // "run forever" timers must always be executed
                    if (maxNumRuns[i] == RUN_FOREVER) {
                        toBeCalled[i] = DEFCALL_RUNONLY;
                    }
                    // other timers get executed the specified number of times
                    else if (numRuns[i] < maxNumRuns[i]) {
                        toBeCalled[i] = DEFCALL_RUNONLY;
                        numRuns[i]++;

                        // after the last run, delete the timer
                        if (numRuns[i] >= maxNumRuns[i]) {
                            toBeCalled[i] = DEFCALL_RUNANDDEL;
                        }
                    }
                }
            }
        }
    }

    for (i = 0; i < MAX_TIMERS; i++) {
        switch(toBeCalled[i]) {
            case DEFCALL_DONTRUN:
                break;

            case DEFCALL_RUNONLY:
                (*callbacks[i])(userData[i]);
                break;

            case DEFCALL_RUNANDDEL:
                (*callbacks[i])(userData[i]);
                deleteTimer(i);
                break;
        }
    }
}


// find the first available slot
// return -1 if none found
int SimpleTimerUD::findFirstFreeSlot() {
    int i;

    // all slots are used
    if (numTimers >= MAX_TIMERS) {
        return -1;
    }

    // return the first slot with no callback (i.e. free)
    for (i = 0; i < MAX_TIMERS; i++) {
        if (callbacks[i] == 0) {
            return i;
        }
    }

    // no free slots found
    return -1;
}


int SimpleTimerUD::setTimer(long d, timer_callback f, int n) {
    return SimpleTimerUD::setTimer(d, f, n, NULL);
}

int SimpleTimerUD::setTimer(long d, timer_callback f, int n, void *userData) {
    int freeTimer;

    freeTimer = findFirstFreeSlot();
    if (freeTimer < 0) {
        return -1;
    }

    if (f == NULL) {
        return -1;
    }

    delays[freeTimer] = d;
    callbacks[freeTimer] = f;
    this->userData[freeTimer] = userData;
    maxNumRuns[freeTimer] = n;
    enabled[freeTimer] = true;
    prev_millis[freeTimer] = elapsed();

    numTimers++;

    return freeTimer;
}


int SimpleTimerUD::setInterval(long d, timer_callback f) {
    return setInterval(d, f, NULL);
}

int SimpleTimerUD::setInterval(long d, timer_callback f, void *userData) {
    return setTimer(d, f, RUN_FOREVER, userData);
}

int SimpleTimerUD::setTimeout(long d, timer_callback f) {
    return setTimeout(d, f, NULL);
}

int SimpleTimerUD::setTimeout(long d, timer_callback f, void *userData) {
    return setTimer(d, f, RUN_ONCE, userData);
}


void SimpleTimerUD::deleteTimer(int timerId) {
    if (timerId >= MAX_TIMERS) {
        return;
    }

    // nothing to delete if no timers are in use
    if (numTimers == 0) {
        return;
    }

    // don't decrease the number of timers if the
    // specified slot is already empty
    if (callbacks[timerId] != NULL) {
        callbacks[timerId] = 0;
        enabled[timerId] = false;
        toBeCalled[timerId] = DEFCALL_DONTRUN;
        delays[timerId] = 0;
        numRuns[timerId] = 0;

        // update number of timers
        numTimers--;
    }
}


// function contributed by code@rowansimms.com
void SimpleTimerUD::restartTimer(int numTimer) {
    if (numTimer >= MAX_TIMERS) {
        return;
    }

    prev_millis[numTimer] = elapsed();
}


boolean SimpleTimerUD::isEnabled(int numTimer) {
    if (numTimer >= MAX_TIMERS) {
        return false;
    }

    return enabled[numTimer];
}


void SimpleTimerUD::enable(int numTimer) {
    if (numTimer >= MAX_TIMERS) {
        return;
    }

    enabled[numTimer] = true;
}


void SimpleTimerUD::disable(int numTimer) {
    if (numTimer >= MAX_TIMERS) {
        return;
    }

    enabled[numTimer] = false;
}


void SimpleTimerUD::toggle(int numTimer) {
    if (numTimer >= MAX_TIMERS) {
        return;
    }

    enabled[numTimer] = !enabled[numTimer];
}


int SimpleTimerUD::getNumTimers() {
    return numTimers;
}
