//
// Created by Rescyy on 10/30/2025.
//

#include "signal_helper.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

static void segfaultHandler(int sig, siginfo_t *info, void *ucontext) {
    (void)ucontext;
    printf("Segmentation fault at address: %p\n\n", info->si_addr);
    fflush(stdout);
    printf("Exiting...\n\n");
    fflush(stdout);
    _exit(1);
}

static void setupSegfaultHandler() {
    struct sigaction sa;
    sa.sa_sigaction = segfaultHandler;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGSEGV, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }
}

void setupSignalHandlers() {
    setupSegfaultHandler();
    signal(SIGPIPE, SIG_IGN);
}
