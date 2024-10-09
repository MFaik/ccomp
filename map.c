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

void init_map(SVMap* map) {
    const unsigned min_bucket = 61;
    map->size = 0;
    init_vectorVectorSVNumPair(&map->sv_arr, min_bucket);
    for(int i = 0;i < min_bucket;i++) {
        VectorSVNumPair vec;
        init_vectorSVNumPair(&vec, 2);
        push_vectorVectorSVNumPair(&map->sv_arr, vec);
    }
}

unsigned insert_map(SVMap* map, StringView sv, unsigned id) {
    //load factor is 1
    if(map->size >= map->sv_arr.size) {
        VectorVectorSVNumPair new_vec;
        init_vectorVectorSVNumPair(&new_vec, map->sv_arr.size*11);
        new_vec.size = map->sv_arr.size*7;
        for(int i = 0;i < new_vec.size;i++) {
            init_vectorSVNumPair(&new_vec.array[i], 2);
        }

        for(int i = 0;i < map->sv_arr.size;i++) {
            for(int j = 0;j < map->sv_arr.array[i].size;j++) {
                unsigned new_h = hash(map->sv_arr.array[i].array[j].sv)%new_vec.size;
                push_vectorSVNumPair(&new_vec.array[new_h], map->sv_arr.array[i].array[j]); 
            }
            free_vectorSVNumPair(&map->sv_arr.array[i]);
        }
        free_vectorVectorSVNumPair(&map->sv_arr);
        map->sv_arr = new_vec;
    }

    unsigned long h = hash(sv)%map->sv_arr.size;
    VectorSVNumPair *m = &map->sv_arr.array[h];
    for(int i = 0;i < m->size;i++) {
        if(sv.len == m->array[i].sv.len && 
                !strncmp(sv.start, m->array[i].sv.start, sv.len)) {
            return m->array[i].id;
        }
    }
    push_vectorSVNumPair(m, (SVNumPair){sv, id});
    map->size++;
    return id;
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

void free_map(SVMap* map) {
    for(int i = 0;i < map->sv_arr.size;i++) {
        free_vectorSVNumPair(&map->sv_arr.array[i]);
    }
    free_vectorVectorSVNumPair(&map->sv_arr);
}

#ifdef MAP_TEST 
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

const unsigned text_len = 10000;

int test(unsigned sv_cnt) {
    char *text;text = malloc(text_len * sizeof(char));
    for(unsigned i = 0;i < text_len;i++) {
        text[i] = rand()%200+55;
    }

    unsigned id = 0;
    SVMap map;
    init_map(&map);
    VectorSVNumPair vec;
    init_vectorSVNumPair(&vec, sv_cnt);
    
    for(unsigned i = 0;i < sv_cnt;i++) {
        StringView sv;
        sv.len = rand()%(text_len-1)+1;
        sv.start = text+rand()%(text_len-sv.len);
        push_vectorSVNumPair(&vec, (SVNumPair){sv, 0});
    }

    clock_t start = clock(), diff;
    for(unsigned i = 0;i < sv_cnt;i++) {
        unsigned id = insert_map(&map, vec.array[i].sv, id++);
        vec.array[i].id = id;
    }

    diff = clock() - start;
    int msec = diff * 1000 / CLOCKS_PER_SEC;
    printf("Map took %d milliseconds with %d elements\n", msec, sv_cnt);

    for(int i = 0;i < vec.size;i++) {
        if(vec.array[i].id != get_map(&map, vec.array[i].sv)) {
            printf("error %.*s id changed", vec.array[i].sv.len, vec.array[i].sv.start);
        }
        for(int j = i+1;j < vec.size;j++) {
            if(vec.array[i].sv.len == vec.array[j].sv.len && 
               !strncmp(vec.array[i].sv.start, vec.array[j].sv.start, vec.array[i].sv.len)) {
                if(vec.array[i].id != vec.array[j].id) {
                    printf("error %.*s != %.*s", vec.array[i].sv.len, vec.array[i].sv.start,
                                                 vec.array[j].sv.len, vec.array[j].sv.start);
                    return 1;
                }
            } else {
                if(vec.array[i].id == vec.array[j].id) {
                    printf("error %.*s == %.*s", vec.array[i].sv.len, vec.array[i].sv.start,
                                                 vec.array[j].sv.len, vec.array[j].sv.start);
                    return 2;
                }
            }
        }
    }
    return 0;
}

int main() {
    srand(time(NULL));
    for(int i = 0;i < 100;i++) {
        int ret = test(rand()%10000);
        if(ret)
            return ret;
    }
    return 0;
}

#endif
