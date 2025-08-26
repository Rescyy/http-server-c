//
// Created by Crucerescu Vladislav on 16.08.2025.
//

#include "file_handler.h"

#include <stdio.h>
#include <string.h>

typedef struct {
    const char *ext;
    const char *mime;
} MimeMapping;

static const MimeMapping mime_mappings[] = {
    // Documents
    { "txt",  "text/plain" },
    { "html", "text/html" },
    { "htm",  "text/html" },
    { "css",  "text/css" },
    { "csv",  "text/csv" },
    { "xml",  "application/xml" },
    { "json", "application/json" },
    { "pdf",  "application/pdf" },
    { "doc",  "application/msword" },
    { "docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document" },
    { "xls",  "application/vnd.ms-excel" },
    { "xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet" },
    { "ppt",  "application/vnd.ms-powerpoint" },
    { "pptx", "application/vnd.openxmlformats-officedocument.presentationml.presentation" },

    // Images
    { "jpg",  "image/jpeg" },
    { "jpeg", "image/jpeg" },
    { "png",  "image/png" },
    { "gif",  "image/gif" },
    { "bmp",  "image/bmp" },
    { "webp", "image/webp" },
    { "tif",  "image/tiff" },
    { "tiff", "image/tiff" },
    { "svg",  "image/svg+xml" },
    { "ico",  "image/x-icon" },

    // Audio
    { "mp3",  "audio/mpeg" },
    { "wav",  "audio/wav" },
    { "ogg",  "audio/ogg" },
    { "flac", "audio/flac" },
    { "aac",  "audio/aac" },
    { "m4a",  "audio/mp4" },

    // Video
    { "mp4",  "video/mp4" },
    { "webm", "video/webm" },
    { "ogv",  "video/ogg" },
    { "mov",  "video/quicktime" },
    { "avi",  "video/x-msvideo" },
    { "mkv",  "video/x-matroska" },
    { "flv",  "video/x-flv" },
    { "wmv",  "video/x-ms-wmv" },

    // Archives / Binary
    { "zip",  "application/zip" },
    { "tar",  "application/x-tar" },
    { "gz",   "application/gzip" },
    { "rar",  "application/vnd.rar" },
    { "7z",   "application/x-7z-compressed" },
    { "exe",  "application/vnd.microsoft.portable-executable" },
    { "bin",  "application/octet-stream" },
    { "iso",  "application/x-iso9660-image" },

    // Web / Scripts
    { "js",   "application/javascript" },
    { "mjs",  "text/javascript" },
    { "wasm", "application/wasm" }
};

static const size_t mime_mappings_count = sizeof(mime_mappings) / sizeof(mime_mappings[0]);

char *getExtension(const char *filename) {
    if (filename == NULL) {
        return NULL;
    }
    char *last = strrchr(filename, '.');
    if (last == NULL) {
        return NULL;
    }
    return last + 1;
}

const char *defaultMimeType = "application/octet-stream";

// Example lookup function
const char* getMimeType(const char *extension) {
    if (extension == NULL) {
        return defaultMimeType;
    }

    for (size_t i = 0; i < mime_mappings_count; i++) {
        if (strcmp(extension, mime_mappings[i].ext) == 0) {
            return mime_mappings[i].mime;
        }
    }

    return defaultMimeType;
}
