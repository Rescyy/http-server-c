//
// Created by Rescyy on 10/22/2025.
//

#include "destructors.h"

#include <logging.h>
#include <pthread.h>

DEFINE_ARRAY_FUNCS(Destructor, allocate, reallocate)

static pthread_key_t destructorsThreadKey;

void attachDestructor(destructor_t func, void *ptr) {
    Destructor destructor = {
        .func = func,
        .ptr = ptr,
    };
    ARRAY_T(Destructor) *destructors = getDestructors();
    ARRAY_PUSH(Destructor, destructors, destructor);
}

void setDestructors(const ARRAY_T(Destructor) *destructors) {
    pthread_setspecific(destructorsThreadKey, destructors);
}

ARRAY_T(Destructor) *getDestructors() {
    ARRAY_T(Destructor) *destructors = pthread_getspecific(destructorsThreadKey);
    return destructors;
}

void invokeDestructors(ARRAY_T(Destructor) *destructors) {
    debug("Invoking destructors %u", destructors->length);
    for (int i = destructors->length - 1; i >= 0; i--) {
        destructors->data[i].func(destructors->data[i].ptr);
    }
    deallocate(destructors->data);
    deallocate(destructors);
}

static void invokeDestructorsWrapper(void *ptr) {
    invokeDestructors(ptr);
}

void initDestructors() {
    pthread_key_create(&destructorsThreadKey, invokeDestructorsWrapper);
}

void deInitDestructors() {
    invokeDestructors(getDestructors());
    pthread_key_delete(destructorsThreadKey);
}

