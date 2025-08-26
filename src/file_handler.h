//
// Created by Crucerescu Vladislav on 16.08.2025.
//

#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

extern const char *defaultMimeType;
char *getExtension(const char *filename);
const char *getMimeType(const char *extension);

#endif //FILE_HANDLER_H
