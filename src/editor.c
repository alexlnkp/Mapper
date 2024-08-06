#include <assert.h>
#include <stddef.h>

#include <raylib.h>
#include <rcamera.h>
#include <raymath.h>

#include "ui.h"
#include "config.h"
#include "editor.h"
#include "funkymacros.h"

/* Global variables */
Camera* cam;
CameraState cam_state;
float cam_speed;

BoundingBox ground;

Object* objects;
unsigned _n_objs;
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
    _n_objs = 0;
    MALLOC(objects, sizeof(Object) * OBJECTS_MEMORY_RESERVE);
    for (unsigned i = _n_objs; i < OBJECTS_MEMORY_RESERVE; ++i) {
        objects[i] = (Object){0};
    }
}

void InitGlobal(void) {
    MALLOC(cam, sizeof(Camera));
    CameraInit();

    ground.min = GROUND_MIN_BOUND;
    ground.max = GROUND_MAX_BOUND;

    ObjectsInit();
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

        if ((_n_objs + 1) % OBJECTS_MEMORY_RESERVE == 0) {
            REALLOC(objects, sizeof(Object) * (_n_objs + OBJECTS_MEMORY_RESERVE));
            TraceLog(LOG_DEBUG, "Realloc'd for %d more objects! Objects total: %d", OBJECTS_MEMORY_RESERVE, _n_objs + 1);

            for (unsigned i = _n_objs; i < (_n_objs + OBJECTS_MEMORY_RESERVE); ++i) {
                objects[i] = (Object){0};
            }
        }

        objects[_n_objs] = new_cube;
        _n_objs++;
    }
}

void DrawCubes(void) {
    for (unsigned i = 0; i < _n_objs; ++i) {
        Object cur_obj = objects[i];

        switch (cur_obj.type) {
        case CUBE: {
            Vector3 _dim = cur_obj.data.Cube.dim;
            DrawCube(cur_obj.pos, _dim.x, _dim.y, _dim.z, cur_obj.col);
        } break;

        default: {} break;
        }
    }
}

void DeInitGlobal(void) {
    FREE(cam);
    FREE(objects);
}

void HandleEvents(void) {
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        cam_state = WantsToMoveFreely;
    } else if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE) & IsKeyDown(KEY_LEFT_SHIFT)) {
        cam_state = WantsToPan;
    } else {
        cam_state = Static;
    }

    if (IsKeyPressed(KEY_SPACE)) {
        CreateCube();
    }

    cam_speed = (IsKeyDown(KEY_LEFT_SHIFT)) ? CAMERA_MAX_SPEED : CAMERA_MOVE_SPEED;

    switch(cam_state) {
    case Static: {
        /* Handle shortcuts or whatever */
        EnableGUI();
        UnlockGUI();

    } break;

    case WantsToMoveFreely: {
        /* Handle camera movement for free cam */

        DisableGUI();
        LockGUI();

        char z = IsKeyDown(KEY_W) - IsKeyDown(KEY_S);
        char x = IsKeyDown(KEY_D) - IsKeyDown(KEY_A);

        CameraMoveForward(cam, cam_speed * z, false);
        CameraMoveRight(cam, cam_speed * x, false);

    } break;

    default: {
        DisableGUI();
        LockGUI();
    } break;
    }
}

void Update(void) {
    CameraUpdate();
}

void CameraUpdate(void) {
    const Vector2 mouseDelta = GetMouseDelta();

    switch (cam_state) {
    case WantsToMoveFreely: {
        DisableCursor();

        CameraYaw(cam, -mouseDelta.x*CAMERA_MOUSE_SENSITIVITY, false);
        CameraPitch(cam, -mouseDelta.y*CAMERA_MOUSE_SENSITIVITY, true, false, false);
    } break;

    case WantsToPan: {
        DisableCursor();

        if (mouseDelta.x != 0.0f) {
            CameraMoveRight(cam, CAMERA_PAN_SPEED * mouseDelta.x, false);
        }
        if (mouseDelta.y != 0.0f) {
            CameraMoveUp(cam, CAMERA_PAN_SPEED * -mouseDelta.y);
        }

    } break;

    default: {
        EnableCursor();
    } break;
    }
}

void Draw(void) {
    BeginDrawing(); {
        ClearBackground(RAYWHITE);

        BeginMode3D(*cam); {
            DrawCubes();

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
