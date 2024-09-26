#pragma once

#include <stdlib.h>

#define vector_header(Type)\
typedef struct {\
    Type *array;\
    size_t size;\
    size_t capacity;\
} Vector##Type;\
void init_vector##Type(Vector##Type *v, size_t initial_capacity);\
char insert_vector##Type(Vector##Type *v, Type t);\
void free_vector##Type(Vector##Type *v);

#define vector_body(Type)\
void init_vector##Type(Vector##Type *v, size_t initial_capacity) {\
    v->array = (Type*)malloc(initial_capacity * sizeof(Type));\
    v->size = 0;\
    v->capacity = initial_capacity;\
}\
char insert_vector##Type(Vector##Type *v, Type t) {\
    if(v->size >= v->capacity) {\
        v->capacity *= 2;\
        Type* arr = (Type*)realloc(v->array, v->capacity * sizeof(Type));\
        if(!arr)\
            exit(1);\
        v->array = arr;\
    }\
    v->array[v->size++] = t;\
    return 1;\
}\
void free_vector##Type(Vector##Type *v) {\
    free(v->array);\
    v->array = 0;\
    v->size = v->capacity = 0;\
}
