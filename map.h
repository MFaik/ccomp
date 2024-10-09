#pragma once

#include <string.h>
#include <stdbool.h>

#include "string_view.h"
#include "vector.h"

extern unsigned ID_NOT_FOUND;
typedef struct {
    StringView sv;
    unsigned id;
} SVNumPair;

vector_header(SVNumPair);
vector_header(VectorSVNumPair);

typedef struct {
    VectorVectorSVNumPair sv_arr;
    unsigned size;
} SVMap;

void init_map(SVMap* map);

//the string view len must be longer than 1
unsigned insert_map(SVMap* map, StringView sv, unsigned id);

unsigned get_map(SVMap* map, StringView sv);

void free_map(SVMap* map);
