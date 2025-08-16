//
// Created by Crucerescu Vladislav on 13.08.2025.
//

#include <pthread.h>

#include "app_state.h"
#include "alloc.h"

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
// static GlobalState appState = {
// };

SessionState *newSessionAppState() {
    return allocate(sizeof(SessionState));
}

// GlobalState *getAppState() {
//     pthread_mutex_lock(&mutex);
//     return &appState;
// }

void releaseAppState() {
    pthread_mutex_unlock(&mutex);
}



