//
// Created by Crucerescu Vladislav on 13.08.2025.
//

#ifndef APP_STATE_H
#define APP_STATE_H
#include "connection.h"
#include "alloc.h"

typedef struct {
    TcpSocket clientSocket;
    unsigned long connectionIndex;
    unsigned long requestIndex;
} SessionState;

void initSessionStateFactory();
SessionState *newSessionState();
void setSessionState(SessionState *state);
SessionState *getSessionState();

#endif //APP_STATE_H
