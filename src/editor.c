#include <assert.h>
#include <stddef.h>
#include <stdio.h>

#include <raylib.h>
#include <rcamera.h>
#include <raymath.h>
#include "raygui.h"

#include "config.h"
#include "editor.h"
#include "funkymacros.h"

/* Global variables */
Camera* cam;
CameraState cam_state;
float cam_speed;

BoundingBox ground;

Cube cubes[1024];
unsigned _n_cubes;
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

void InitGlobal(void) {
    MALLOC(cam, sizeof(Camera));
    CameraInit();

    ground.min = GROUND_MIN_BOUND;
    ground.max = GROUND_MAX_BOUND;

    _n_cubes = 0;

    for (unsigned i = 0; i < sizeof(cubes) / sizeof(cubes[0]); ++i) {
        cubes[i] = (Cube){0};
    }
}

void CreateCube(void) {
    Ray ray;
    ray.position = cam->position;
    ray.direction = GetCameraForward(cam);

    RayCollision ground_collision = GetRayCollisionBox(ray, ground);

    if (ground_collision.hit) {
        Vector3 col_point = ground_collision.point;
        Cube new_cube = {
            .pos = (Vector3){col_point.x, col_point.y + 0.5f, col_point.z},
            .dim = (Vector3){1.0f, 1.0f, 1.0f},
            .col = RED
        };

        cubes[_n_cubes] = new_cube;
        _n_cubes++;
    }
}

void DrawCubes(void) {
    for (unsigned i = 0; i < _n_cubes; ++i) {
        Cube cur_cube = cubes[i];
        DrawCube(cur_cube.pos, cur_cube.dim.x, cur_cube.dim.y, cur_cube.dim.z, cur_cube.col);
    }
}

void DeInitGlobal(void) {
    FREE(cam);
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
        GuiEnable();
        GuiUnlock();

    } break;

    case WantsToMoveFreely: {
        /* Handle camera movement for free cam */

        GuiDisable();
        GuiLock();

        char z = IsKeyDown(KEY_W) - IsKeyDown(KEY_S);
        char x = IsKeyDown(KEY_D) - IsKeyDown(KEY_A);

        CameraMoveForward(cam, cam_speed * z, false);
        CameraMoveRight(cam, cam_speed * x, false);

    } break;

    default: {
        GuiDisable();
        GuiLock();
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

void DrawGUI(void) {
    if (GuiButton((Rectangle){.height=30.0f, .width=100.0f, .x=0.0f, .y=0.0f}, "#80#Create cube")) {
        CreateCube();
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

    CloseWindow();

    return 0;
}
