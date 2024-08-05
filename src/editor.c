#include <assert.h>
#include <stddef.h>

#include <raylib.h>
#include <rcamera.h>
#include <raymath.h>

#include "config.h"
#include "editor.h"
#include "funkymacros.h"

/* Global variables */
Camera* cam;
CameraState cam_state;
float cam_speed;
/* ---------------- */

inline float vFov_from_hFov(float hFov, float aspect) {
    return (2.0f * atan(tan((hFov * DEG2RAD) / 2.0f) / aspect)) * RAD2DEG;
}

void CameraInit() {
    *cam = (Camera){0};
    cam->position = (Vector3){0.0f, 1.0f, 0.0f};
    cam->target   = (Vector3){1.0f, 1.0f, 0.0f};
    cam->up       = (Vector3){0.0f, 1.0f, 0.0f};
    cam->fovy = vFov_from_hFov(CAMERA_FOV, ((float)WINDOW_WIDTH / (float)WINDOW_HEIGHT));
    cam->projection = CAMERA_PERSPECTIVE;

    cam_state = Static;
    cam_speed = CAMERA_MOVE_SPEED;
}

void InitGlobal() {
    MALLOC(cam, sizeof(Camera));
    CameraInit();
}

void DeInitGlobal() {
    FREE(cam);
}

void HandleEvents() {
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        cam_state = WantsToMoveFreely;
    } else if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE) & IsKeyDown(KEY_LEFT_SHIFT)) {
        cam_state = WantsToPan;
    } else {
        cam_state = Static;
    }

    cam_speed = (IsKeyDown(KEY_LEFT_SHIFT)) ? CAMERA_MAX_SPEED : CAMERA_MOVE_SPEED;

    switch(cam_state) {
    case Static: {
        /* Handle shortcuts or whatever */

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

void Update() {
    CameraUpdate();
}

void CameraUpdate() {
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

void Draw() {
    BeginDrawing(); {
        ClearBackground(RAYWHITE);

        BeginMode3D(*cam); {

            DrawGrid(10, 1.0f);

        } EndMode3D();
    } EndDrawing();
}

int main(void) {
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
