//
// Created by Rescyy on 10/22/2025.
//

#ifndef HTTPSERVERC_DESTRUCTORS_H
#define HTTPSERVERC_DESTRUCTORS_H

#include <array.h>

typedef struct {
    void (*func)(void *ptr);
    void *ptr;
} Destructor;

TYPEDEF_ARRAY(Destructor);
DECLARE_ARRAY_FUNCS(Destructor)

ARRAY_T(Destructor) *getDestructors();
void setDestructors(const ARRAY_T(Destructor) *destructors);
void invokeDestructors(ARRAY_T(Destructor) *destructors);
void initDestructors();
void deInitDestructors();

#endif //HTTPSERVERC_DESTRUCTORS_H