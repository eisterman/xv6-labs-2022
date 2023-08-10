#include "kernel/types.h"
#include "user/user.h"

vector* vector_init() {
    vector* v = malloc(sizeof(vector));
    v->data = malloc(512);
    v->capacity = 512;
    v->length = 0;
    return v;
}

void __vector_resize(vector* v, uint bytes) {
    while (v->length+bytes > v->capacity) {
        // Resize
        uint newcapacity = v->capacity + (v->capacity > 4096 ? 4096 : v->capacity);
        void* newdata = malloc(newcapacity);
        memcpy(newdata, v->data, v->length);
        free(v->data);
        v->data = newdata;
        v->capacity = newcapacity;
    }
}

vector* vector_append(vector* v, void* m, uint bytes) {
    __vector_resize(v, bytes);
    memcpy(&v->data[v->length], m, bytes);
    v->length += bytes;
    return v;
}

vector* vector_byteappend(vector* v, char bt) {
    __vector_resize(v, 1);
    v->data[v->length] = bt;
    v->length += 1;
    return v;
}

void vector_free(vector* v) {
    free(v->data);
    free(v);
}
