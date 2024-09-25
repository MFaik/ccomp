#pragma once

#include "string.h"

#include "string_view.h"
#include "vector.h"

typedef struct {
    StringView sv;
    unsigned num;
} SVNumPair;

vector_header(SVNumPair);

typedef struct {
    VectorSVNumPair str[26];
    unsigned cnt;
} SVMap;

void init_map(SVMap* map);

//the string view len must be longer than 1
unsigned insert_map(SVMap* map, StringView sv);

