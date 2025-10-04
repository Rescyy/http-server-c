//
// Created by Crucerescu Vladislav on 13.08.2025.
//

#include <pthread.h>

#include "app_state.h"
#include "alloc.h"


SessionState *newSessionState() {
    SessionState *state = allocate(sizeof(SessionState));
    state->requestIndex = 1;
}

static pthread_key_t sessionStateKey;
static pthread_once_t sessionStateKeyOnce = PTHREAD_ONCE_INIT;

static void freeSessionState(void *state) {
    deallocate(state);
}

static void createSessionStateKey() {
    pthread_key_create(&sessionStateKey, freeSessionState);
}

void setCurrentThreadSessionState(SessionState *state) {
    pthread_once(&sessionStateKeyOnce, createSessionStateKey);
    pthread_setspecific(sessionStateKey, state);
}

SessionState *getCurrentThreadSessionState() {
    return pthread_getspecific(sessionStateKey);
}


