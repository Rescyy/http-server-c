//
// Created by Rescyy on 9/28/2025.
//

#include "../includes/errors.h"

const char* errToStr(int error) {
    switch (error) {
        case TCP_STREAM_ERROR:
            return "TCP stream error";
        case ENTITY_TOO_LARGE_ERROR:
            return "Entity too large";
        case UNKNOWN_METHOD:
            return "Unknown method";
        case UNKNOWN_VERSION:
            return "Unknown version";
        case BAD_REQUEST_ERROR:
            return "Bad request (bad format or protocol)";
        case URI_TOO_LARGE_ERROR:
            return "URI too large";
        case TCP_STREAM_CLOSED:
            return "TCP stream closed";
        case TCP_STREAM_TIMEOUT:
            return "TCP stream timeout";
        default:
            return "Unknown error";
    }
}
