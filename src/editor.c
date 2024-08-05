#include <assert.h>
#include <stddef.h>

#include <raylib.h>
#include <rcamera.h>

#include "config.h"
#include "editor.h"
#include "funkymacros.h"

/* Global variables */
Camera* cam;
CameraState cam_state;
/* ---------------- */

void CameraInit() {
    *cam = (Camera){0};
    cam->position = (Vector3){0.0f, 1.0f, 0.0f};
    cam->target   = (Vector3){1.0f, 1.0f, 0.0f};
    cam->up       = (Vector3){0.0f, 1.0f, 0.0f};
    cam->fovy = 45.0f;
    cam->projection = CAMERA_PERSPECTIVE;

    cam_state = Static;
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
    } else {
        cam_state = Static;
    }
    switch(cam_state) {
    case Static: {
        /* Handle shortcuts or whatever */

    } break;

    case WantsToMoveFreely: {
        /* Handle camera movement for free cam */

        char z = IsKeyDown(KEY_W) - IsKeyDown(KEY_S);
        char x = IsKeyDown(KEY_D) - IsKeyDown(KEY_A);

        CameraMoveForward(cam, CAMERA_MOVE_SPEED * z, false);
        CameraMoveRight(cam, CAMERA_MOVE_SPEED * x, false);

    } break;

    default: {

    } break;
    }
}

void Update() {
    CameraUpdate();
}

void CameraUpdate() {
    switch (cam_state) {
    case WantsToMoveFreely: {
        Vector2 mouseDelta = GetMouseDelta();

        CameraYaw(cam, -mouseDelta.x*CAMERA_MOUSE_SENSITIVITY, false);
        CameraPitch(cam, -mouseDelta.y*CAMERA_MOUSE_SENSITIVITY, true, false, false);

        /*have to disable cursor AFTER getting the mouse delta, otherwise it just
          doesn't work and the delta is always (Vector2){0.0f, 0.0f}*/
        DisableCursor();
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
