//
// Created by Crucerescu Vladislav on 29.08.2025.
//

#ifndef RESULT_TYPE_H
#define RESULT_TYPE_H

#define RESULT_T(type) type##_result_t
#define TYPEDEF_RESULT(type) \
typedef struct {\
    int ok;\
    type var;\
} type##_result_t

#define RESULT_ERROR(type) ((RESULT_T(type)) {.ok = 0})
#define RESULT_FROM_VAR(type, _var) ((RESULT_T(type)) {.var = _var, .ok = 1})
#define IF_ERROR_RETURN(to_t, result) if (!result.ok) return RESULT_ERROR(to_t)

#endif //RESULT_TYPE_H
