//
// Created by Crucerescu Vladislav on 13.08.2025.
//

#ifndef APP_STATE_H
#define APP_STATE_H
#include "connection.h"

typedef struct {
    TcpSocket clientSocket;
    unsigned long connectionIndex;
    unsigned long requestIndex;
} SessionState;

SessionState *newSessionState();
void setCurrentThreadSessionState(SessionState *state);
SessionState *getCurrentThreadSessionState();

#endif //APP_STATE_H
