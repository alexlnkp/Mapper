#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <raylib.h>

#include "editor.h"
#include "funkymacros.h"

#define LABEL_READ_MEMORY_RESERVE 10 /* how many characters we can read until we have to realloc */

char* ReadStringFromStream(FILE* file) {
    char* buffer = NULL;
    size_t buffer_size = 0;
    size_t bytes_read = 0;
    int c;

    /* read stream until 0x00 */
    while ((c = fgetc(file)) != EOF && c != '\0') {
        if (bytes_read % LABEL_READ_MEMORY_RESERVE == 0) {
            buffer_size += LABEL_READ_MEMORY_RESERVE;
            REALLOC(buffer, buffer_size);
        }

        buffer[bytes_read++] = (char)c;
    }

    /* trim buffer and add 0x00 at the end */
    if (bytes_read > 0) {
        buffer = realloc(buffer, bytes_read + 1);
        buffer[bytes_read] = '\0';
    }

    return buffer;
}

int main(int argc, char* argv[]) {
    int _err_code = 0;

    if (argc < 2) {
        printf("Provide the map file to inspect.\n");
        _err_code = -1; goto exit;
    }
    
    FILE* fp = fopen(argv[1], "rb");

    ObjectCounter _n_objs = 0;
    fread(&_n_objs, sizeof(ObjectCounter), 1, fp);
    assert(_n_objs && "Map is empty!");

    Object* objects;
    MALLOC(objects, sizeof(Object) * _n_objs);

/*
    Read the fields in order they're stored in...

    fwrite(&cur_obj.pos, sizeof(Vector3), 1, fp);
    fwrite(&cur_obj.type, sizeof(ObjectType), 1, fp);
    fwrite(&cur_obj.col, sizeof(Color), 1, fp);
    fwrite(&cur_obj.data, sizeof(cur_obj.data), 1, fp);
    fwrite(cur_obj.label, sizeof(char) * (strlen(cur_obj.label) + 1), 1, fp);
*/
    for (ObjectCounter i = 0; i < _n_objs; ++i) {
        fread(&objects[i].pos, sizeof(Vector3), 1, fp);
        fread(&objects[i].type, sizeof(ObjectType), 1, fp);
        fread(&objects[i].col, sizeof(Color), 1, fp);
        fread(&objects[i].data, sizeof(objects[i].data), 1, fp);
        objects[i].label = ReadStringFromStream(fp);
    }

    fclose(fp);

    /* checking out the data we've read from the map file */
    for (ObjectCounter i = 0; i < _n_objs; ++i) {
        Object cur_obj = objects[i];
        Vector3 cur_obj_dim = cur_obj.data.Cube.dim;
        printf("Object %d: %s; ", i, obj_types[cur_obj.type]);
        printf("Color: RGB(%d, %d, %d); ", cur_obj.col.r, cur_obj.col.g, cur_obj.col.b);
        printf("Dimensionality: {%f; %f; %f}; ", cur_obj_dim.x, cur_obj_dim.y, cur_obj_dim.z);
        printf("Location: {%f; %f; %f}; ", cur_obj.pos.x, cur_obj.pos.y, cur_obj.pos.z);
        printf("Label: \"%s\"\n", cur_obj.label);
    }

exit:
    return _err_code;
}