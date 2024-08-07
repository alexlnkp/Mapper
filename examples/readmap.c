#include <assert.h>
#include <stdio.h>
#include <raylib.h>

#include "editor.h"
#include "funkymacros.h"

const char* GetObjectType(ObjectType type) {
    switch (type) {
    case CUBE: {
        return "Cube";
    }

    case SPHERE: {
        return "Sphere";
    }

    default: {
        return "IDK what that is";
    }
    }

    assert(false);
    return "";
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

    for (ObjectCounter i = 0; i < _n_objs; ++i) {
        fread(&objects[i], sizeof(Object), 1, fp);
    }

    fclose(fp);

    for (ObjectCounter i = 0; i < _n_objs; ++i) {
        Object cur_obj = objects[i];
        Vector3 cur_obj_dim = cur_obj.data.Cube.dim;
        printf("Object %d: %s; ", i, GetObjectType(cur_obj.type));
        printf("Color: RGB(%d, %d, %d); ", cur_obj.col.r, cur_obj.col.g, cur_obj.col.b);
        printf("Dimensionality: {%f; %f; %f}; ", cur_obj_dim.x, cur_obj_dim.y, cur_obj_dim.z);
        printf("Location: {%f; %f; %f}\n", cur_obj.pos.x, cur_obj.pos.y, cur_obj.pos.z);
    }

exit:
    return _err_code;
}