#include <stdio.h>
#include <assert.h>
#include <stddef.h>
#include <string.h>

#include <raylib.h>
#include <rcamera.h>
#include <raymath.h>

#include "ui.h"
#include "config.h"
#include "editor.h"
#include "funkymacros.h"

/* Global variables */
bool cursor_disabled;

Camera* cam;
CameraState cam_state;
float cam_speed;

BoundingBox ground;

Map map;

Object* selected_obj;
/* ---------------- */

inline float vFov_from_hFov(float hFov, float aspect) {
    return (2.0f * atan(tan((hFov * DEG2RAD) / 2.0f) / aspect)) * RAD2DEG;
}

void CameraInit(void) {
    *cam = (Camera){0};
    cam->position = (Vector3){0.0f, 1.0f, 0.0f};
    cam->target   = (Vector3){1.0f, 1.0f, 0.0f};
    cam->up       = (Vector3){0.0f, 1.0f, 0.0f};
    cam->fovy = vFov_from_hFov(CAMERA_FOV, ((float)WINDOW_WIDTH / (float)WINDOW_HEIGHT));
    cam->projection = CAMERA_PERSPECTIVE;

    cam_state = Static;
    cam_speed = CAMERA_MOVE_SPEED;
}

void ObjectsInit(void) {
    map.num_objects = 0;
    MALLOC(map.objects, sizeof(Object) * OBJECTS_MEMORY_RESERVE);
    for (ObjectCounter i = map.num_objects; i < OBJECTS_MEMORY_RESERVE; ++i) {
        map.objects[i] = (Object){0};
    }
    selected_obj = NULL;
}

void MapInit(void) {
    map.dim = (Vector3){
        .x=GROUND_TOTAL_LENGTH,
        .y=GROUND_TOTAL_LENGTH, /* Filler value, will be replaced later */
        .z=GROUND_TOTAL_LENGTH
    };
    map.meta = (MapMetadata){
        .author="",
        .name=""
    };
    
    ObjectsInit();
}

void InitGlobal(void) {
    cursor_disabled = false;

    MALLOC(cam, sizeof(Camera));
    CameraInit();

    ground.min = GROUND_MIN_BOUND;
    ground.max = GROUND_MAX_BOUND;

    MapInit();
}

void SetMapMetadata(MapMetadata new_metadata) {
    map.meta = new_metadata;
}

void ExportMap(void) {
    map.meta.author = (strcmp(map.meta.author, "") == 0) ? DEFAULT_MAP_AUTHOR : map.meta.author;
    map.meta.name = (strcmp(map.meta.name, "") == 0) ? DEFAULT_MAP_NAME : map.meta.name;
    
    FILE *fp = fopen(map.meta.name, "wb");
    assert(fp != NULL);

    fwrite(&map.num_objects, sizeof(ObjectCounter), 1, fp);

    for (ObjectCounter i = 0; i < map.num_objects; ++i) {
        fwrite(&map.objects[i], sizeof(Object), 1, fp);
    }

    /* Meta is the last to be stored */
    fwrite(map.meta.name, sizeof(char) * (strlen(map.meta.name) + 1), 1, fp);
    fwrite(map.meta.author, sizeof(char) * (strlen(map.meta.author) + 1), 1, fp);

    fclose(fp);
}

void CreateCube(void) {
    Ray ray;
    ray.position = cam->position;
    ray.direction = GetCameraForward(cam);

    RayCollision ground_collision = GetRayCollisionBox(ray, ground);

    if (ground_collision.hit) {
        Vector3 col_point = ground_collision.point;
        Object new_cube = {
            .pos = (Vector3){col_point.x, col_point.y + 0.5f, col_point.z},
            .type = CUBE,
            .data.Cube = {
                .dim = (Vector3){1.0f, 1.0f, 1.0f}
            },
            .col = RED
        };

        if ((map.num_objects + 1) % OBJECTS_MEMORY_RESERVE == 0) {
            REALLOC(map.objects, sizeof(Object) * (map.num_objects + OBJECTS_MEMORY_RESERVE));
            TraceLog(LOG_DEBUG, "Realloc'd for %d more objects! Objects total: %d", OBJECTS_MEMORY_RESERVE, map.num_objects + 1);

            for (ObjectCounter i = map.num_objects; i < (map.num_objects + OBJECTS_MEMORY_RESERVE); ++i) {
                map.objects[i] = (Object){0};
            }
        }

        map.objects[map.num_objects] = new_cube;
        map.num_objects++;
    }
}

ObjectCounter GetObjectIndexUnderMouse(Vector2 mousePosition) {
    Ray ray = GetMouseRay(mousePosition, *cam);

    for (ObjectCounter i = 0; i < map.num_objects; ++i) {
        Object cur_obj = map.objects[i];

        RayCollision obj_collision;

        switch(cur_obj.type) {
        case CUBE: {
            BoundingBox cube_bb = {
                .min = Vector3Subtract(cur_obj.pos, Vector3Scale(cur_obj.data.Cube.dim, 0.5f)),
                .max = Vector3Add(cur_obj.pos, Vector3Scale(cur_obj.data.Cube.dim, 0.5f))
            };
            obj_collision = GetRayCollisionBox(ray, cube_bb);
        } break;
        case SPHERE: {
            obj_collision = GetRayCollisionSphere(ray, cur_obj.pos, cur_obj.data.Sphere.radius);
        } break;

        default: {} break;
        }

        if (obj_collision.hit) {
            TraceLog(LOG_DEBUG, "Clicked an object with index %d at {%f; %f; %f}",
                i, cur_obj.pos.x, cur_obj.pos.y, cur_obj.pos.z);
            return i;
        }
    }

    return -1; /* No object was clicked */
}

void DrawObjects(void) {
    for (ObjectCounter i = 0; i < map.num_objects; ++i) {
        Object cur_obj = map.objects[i];

        switch (cur_obj.type) {
        case CUBE: {
            Vector3 _dim = cur_obj.data.Cube.dim;
            if (selected_obj != NULL && selected_obj == &map.objects[i]) {
                /* A VERY hacky way to do an object outline. Won't work for more complex shapes. */
                DrawCube(cur_obj.pos, _dim.x*-1.04, _dim.y*-1.04, _dim.z*-1.04, (Color){255, 202, 76, 255});
            }
            DrawCube(cur_obj.pos, _dim.x, _dim.y, _dim.z, cur_obj.col);
        } break;

        default: {} break;
        }
    }
}

void DeInitGlobal(void) {
    FREE(cam);
    FREE(map.objects);
}

void LockCursor(void) {
    /* Lock cursor (ONLY if it's unlocked) */
    if (!cursor_disabled) {
        DisableCursor();
        DisableGUI();
        LockGUI();
        cursor_disabled = true;
    }
}

void UnlockCursor(void) {
    /* Unlock cursor (ONLY if it's locked) */
    if (cursor_disabled) {
        EnableCursor();
        EnableGUI();
        UnlockGUI();
        cursor_disabled = false;
    }
}

void HandleEvents(void) {
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        cam_state = WantsToMoveFreely;
        LockCursor();
    } else if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE) & IsKeyDown(KEY_LEFT_SHIFT)) {
        cam_state = WantsToPan;
        LockCursor();
    } else {
        cam_state = Static;
        UnlockCursor();
    }

    cam_speed = (IsKeyDown(KEY_LEFT_SHIFT)) ? CAMERA_MAX_SPEED : CAMERA_MOVE_SPEED;

    switch(cam_state) {
    case Static: {
        /* Handle shortcuts or whatever */
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            ObjectCounter obj_index = GetObjectIndexUnderMouse(GetMousePosition());
            if (obj_index != (ObjectCounter)-1) {
                /* User clicked on an object */
                selected_obj = &map.objects[obj_index];
            } else {
                /* User clicked on nothing */
                selected_obj = NULL;
            }
        }

        if (IsKeyDown(KEY_LEFT_CONTROL) & IsKeyPressed(KEY_S)) ExportMap();

    } break;

    case WantsToMoveFreely: {
        /* Handle camera movement for free cam */

        char z = IsKeyDown(KEY_W) - IsKeyDown(KEY_S);
        char x = IsKeyDown(KEY_D) - IsKeyDown(KEY_A);

        CameraMoveForward(cam, cam_speed * z, false);
        CameraMoveRight(cam, cam_speed * x, false);

    } break;

    default: {
    } break;
    }
}

void Update(void) {
    CameraUpdate();
}

void CameraUpdate(void) {
    if (cam_state) {
        const Vector2 mouseDelta = GetMouseDelta();

        switch (cam_state) {
        case WantsToMoveFreely: {
            CameraYaw(cam, -mouseDelta.x*CAMERA_MOUSE_SENSITIVITY, false);
            CameraPitch(cam, -mouseDelta.y*CAMERA_MOUSE_SENSITIVITY, true, false, false);
        } break;

        case WantsToPan: {
            if (mouseDelta.x != 0.0f) {
                CameraMoveRight(cam, CAMERA_PAN_SPEED * mouseDelta.x, false);
            }
            if (mouseDelta.y != 0.0f) {
                CameraMoveUp(cam, CAMERA_PAN_SPEED * -mouseDelta.y);
            }

        } break;

        default: {} break;
        }
    }
}

void Draw(void) {
    BeginDrawing(); {
        ClearBackground(RAYWHITE);

        BeginMode3D(*cam); {
            DrawObjects();

            DrawGrid((int)GROUND_TOTAL_LENGTH, 1.0f);

        } EndMode3D();
        DrawGUI();
    } EndDrawing();
}

int main(void) {
    SetConfigFlags(FLAG_WINDOW_UNDECORATED | FLAG_MSAA_4X_HINT);
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);
    SetTargetFPS(TARGET_FPS);

    InitGlobal();

    while (!WindowShouldClose()) {
        HandleEvents();

        Update();

        Draw();
    }

    DeInitGlobal();
    CloseWindow();

    return 0;
}
