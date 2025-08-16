//
// Created by Crucerescu Vladislav on 13.08.2025.
//

#ifndef APP_STATE_H
#define APP_STATE_H
#include "connection.h"

// typedef struct {
// } GlobalState;

typedef struct {
    unsigned long connectionIndex;
    TcpSocket clientSocket;
} SessionState;

SessionState *newSessionAppState();
// GlobalState *getAppState();
void releaseAppState();

#endif //APP_STATE_H
