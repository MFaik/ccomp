#include "map.h"
vector_body(SVNumPair);

void init_map(SVMap* map) {
    for(int i = 0;i < 26;i++) {
        init_vectorSVNumPair(&map->str[i], 4);
    }
    map->cnt = 0;
}

unsigned insert_map(SVMap* map, StringView sv) {
    VectorSVNumPair *vsv = &map->str[sv.start[0]-'a'];
    for(int i = 0;i < vsv->size;i++) {
        if(vsv->array[i].sv.len == sv.len && 
                !strncmp(vsv->array[i].sv.start, sv.start, sv.len)) {
            return vsv->array[i].num;
        }
    }
    insert_vectorSVNumPair(map->str, (SVNumPair){sv, map->cnt});
    return map->cnt++;
}
