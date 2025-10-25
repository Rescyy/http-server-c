//
// Created by Crucerescu Vladislav on 13.08.2025.
//

#include <alloc.h>
#include <app_state.h>
#include <logging.h>
#include <pthread.h>

static int keyIsCreated = 0;
static pthread_key_t sessionStateKey;

SessionState *newSessionState() {
    SessionState *state = allocate(sizeof(SessionState));
    state->requestIndex = 1;
    return state;
}

static void freeSessionState(void *ptr) {
    SessionState *state = ptr;
    info("Closing connection %d with %s", state->connectionIndex, state->clientSocket.ip);
    closeSocket(&state->clientSocket);
    deallocate(ptr);
}

void initSessionStateFactory() {
    pthread_key_create(&sessionStateKey, freeSessionState);
    keyIsCreated = 1;
}

void setSessionState(SessionState *state) {
    pthread_setspecific(sessionStateKey, state);
}

SessionState *getSessionState() {
    return keyIsCreated ? pthread_getspecific(sessionStateKey) : NULL;
}


