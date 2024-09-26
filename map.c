#include "map.h"
vector_body(SVNumPair);
vector_body(VectorSVNumPair);

unsigned ID_NOT_FOUND = ~0;

unsigned long hash(StringView sv) {
    unsigned long ret = 5381;
    for(int i = 0;i < sv.len;i++)
        ret = (ret << 5) + ret + sv.start[i];
    return ret;
}

void init_map(SVMap* map, unsigned id_start) {
    const unsigned min_bucket = 1;
    map->id = id_start;
    map->size = 0;
    init_vectorVectorSVNumPair(&map->sv_arr, min_bucket);
    for(int i = 0;i < min_bucket;i++) {
        VectorSVNumPair vec;
        init_vectorSVNumPair(&vec, 2);
        insert_vectorVectorSVNumPair(&map->sv_arr, vec);
    }
}

unsigned insert_map(SVMap* map, StringView sv) {
    unsigned long h = hash(sv)%map->sv_arr.size;
    VectorSVNumPair *m = &map->sv_arr.array[h];
    for(int i = 0;i < m->size;i++) {
        if(sv.len == m->array[i].sv.len && 
                !strncmp(sv.start, m->array[i].sv.start, sv.len)) {
            return m->array[i].id;
        }
    }
    insert_vectorSVNumPair(m, (SVNumPair){sv, map->id});
    map->size++;
    return map->id++;
    //add rehashing
}

unsigned get_map(SVMap* map, StringView sv) {
    unsigned long h = hash(sv)%map->sv_arr.size;
    VectorSVNumPair *m = &map->sv_arr.array[h];
    for(int i = 0;i < m->size;i++) {
        if(sv.len == m->array[i].sv.len && 
                !strncmp(sv.start, m->array[i].sv.start, sv.len)) {
            return m->array[i].id;
        }
    }
    return ID_NOT_FOUND;
}
