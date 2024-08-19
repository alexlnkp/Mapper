#include <stdio.h>
#include <assert.h>
#include <stddef.h>
#include <string.h>

#include <raylib.h>
#include <rlgl.h>
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

Object** selected_objects; /* Holds the addresses to the selected objects in the map.objects array */
ObjectCounter num_selected_objects; /* (ObjectCounter)-1 means no object selected, 0 means one is selected */

ObjectMoveState move_state;

float default_line_width;

RenderTexture renderTexture;
Shader outline_shader;
int sizeLoc;

int screen_w; /* current screen width */
int screen_h; /* current screen height */
/* ---------------- */

inline float vFov_from_hFov(float hFov, float aspect) {
    return (2.0f * atan(tan((hFov * DEG2RAD) / 2.0f) / aspect)) * RAD2DEG;
}

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
        /* problem here was found due to cppcheck, thank god for that */
        char* new_buf = MemRealloc(buffer, bytes_read + 1);
        if (new_buf == NULL) {
            /* new realloc'd memory is NULL, but original buffer is still valid!! */
            FREE(buffer); /* freeing to prevent OOB read segfault */
            assert(false && "Not enough memory to realloc");
        } else {
            buffer = new_buf;
        }

        buffer[bytes_read] = '\0';
    }

    return buffer;
}

void CameraInit(void) {
    *cam = (Camera){0};
    cam->position = (Vector3){0.0f, 1.0f, 0.0f};
    cam->target   = (Vector3){1.0f, 1.0f, 0.0f};
    cam->up       = (Vector3){0.0f, 1.0f, 0.0f};
    cam->fovy = vFov_from_hFov(CAMERA_FOV, ((float)WINDOW_WIDTH / (float)WINDOW_HEIGHT));
    cam->projection = CAMERA_PERSPECTIVE;

    cam_state = CS_Static;
    cam_speed = CAMERA_MOVE_SPEED;
}

void ObjectsInit(void) {
    map.num_objects = 0;
    MALLOC(map.objects, sizeof(Object) * OBJECTS_MEMORY_RESERVE);
    for (ObjectCounter i = map.num_objects; i < OBJECTS_MEMORY_RESERVE; ++i) {
        map.objects[i] = (Object){0};
    }
    MALLOC(map.meta.name, sizeof(char) * (strlen(DEFAULT_MAP_NAME) + 1 + MAP_META_FIELD_MEMORY_RESERVE));
    strcpy(map.meta.name, DEFAULT_MAP_NAME);

    MALLOC(map.meta.author, sizeof(char) * (strlen(DEFAULT_MAP_AUTHOR) + 1 + MAP_META_FIELD_MEMORY_RESERVE));
    strcpy(map.meta.author, DEFAULT_MAP_AUTHOR);

    MALLOC(selected_objects, sizeof(Object*) * SELECTED_OBJECTS_MEMORY_RESERVE);
    num_selected_objects = (ObjectCounter)-1;
}

void DeInitObjects(void) {
    num_selected_objects = (ObjectCounter)-1;
    FREE(selected_objects);
    for (ObjectCounter i = 0; i < map.num_objects; ++i) {
        map.objects[i] = (Object){0};
        FREE(map.objects[i].label);
    }
    FREE(map.objects);
    map.num_objects = 0;

    FREE(map.meta.author);
    FREE(map.meta.name);
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

void ShadersInit(void) {
    screen_w = GetRenderWidth();
    screen_h = GetRenderHeight();

    int outline_size = OUTLINE_WIDTH;
    const float outlineColor[4] = OUTLINE_COLOR;

    outline_shader = LoadShader(0, OUTLINE_SHADER_LOCATION);

    SetShaderValue(outline_shader, GetShaderLocation(outline_shader, "width"), &screen_w, SHADER_UNIFORM_INT);
    SetShaderValue(outline_shader, GetShaderLocation(outline_shader, "height"), &screen_h, SHADER_UNIFORM_INT);
    SetShaderValue(outline_shader, GetShaderLocation(outline_shader, "color"), outlineColor, SHADER_UNIFORM_VEC4);

    sizeLoc = GetShaderLocation(outline_shader, "size");
    SetShaderValue(outline_shader, sizeLoc, &outline_size, SHADER_UNIFORM_INT);

    renderTexture = LoadRenderTexture(screen_w, screen_h);
}

void ShadersDeInit(void) {
    UnloadShader(outline_shader);
    UnloadRenderTexture(renderTexture);
}

void InitGlobal(void) {
    cursor_disabled = false;

    MALLOC(cam, sizeof(Camera));
    CameraInit();

    ground.min = GROUND_MIN_BOUND;
    ground.max = GROUND_MAX_BOUND;

    move_state = OMS_Stationary;

    MapInit();

    default_line_width = rlGetLineWidth();
    ShadersInit();

    InitGUI();
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
        Object cur_obj = map.objects[i];
        fwrite(&cur_obj.pos, sizeof(Vector3), 1, fp);
        fwrite(&cur_obj.type, sizeof(ObjectType), 1, fp);
        fwrite(&cur_obj.col, sizeof(Color), 1, fp);
        fwrite(&cur_obj.data, sizeof(cur_obj.data), 1, fp);
        fwrite(cur_obj.label, sizeof(char) * (strlen(cur_obj.label) + 1), 1, fp);
    }

    /* Meta is the last to be stored */
    fwrite(map.meta.name, sizeof(char) * (strlen(map.meta.name) + 1), 1, fp);
    fwrite(map.meta.author, sizeof(char) * (strlen(map.meta.author) + 1), 1, fp);

    fclose(fp);
}

void ImportMap(char* file) {
    FILE *fp = fopen(file, "rb");
    assert(fp != NULL);

    TraceLog(LOG_DEBUG, "Reading file in ImportMap() function. File: %s", file);

    fread(&map.num_objects, sizeof(ObjectCounter), 1, fp);

    for (ObjectCounter i = 0; i < map.num_objects; ++i) {
        fread(&map.objects[i].pos, sizeof(Vector3), 1, fp);
        fread(&map.objects[i].type, sizeof(ObjectType), 1, fp);
        fread(&map.objects[i].col, sizeof(Color), 1, fp);
        fread(&map.objects[i].data, sizeof(map.objects[i].data), 1, fp);
        map.objects[i].label = ReadStringFromStream(fp);
    }

    fclose(fp);
}

void CreatePrimitive(Object new_obj) {
    Ray ray;
    ray.position = cam->position;
    ray.direction = GetCameraForward(cam);

    RayCollision ground_collision = GetRayCollisionBox(ray, ground);

    if (ground_collision.hit) {
        Vector3 col_point = ground_collision.point;
        new_obj.pos = (Vector3){col_point.x, col_point.y + 0.5f, col_point.z};

        if ((map.num_objects + 1) % OBJECTS_MEMORY_RESERVE == 0) {
            REALLOC(map.objects, sizeof(Object) * (map.num_objects + OBJECTS_MEMORY_RESERVE));
            TraceLog(LOG_DEBUG, "Realloc'd for %d more objects! Objects total: %d", OBJECTS_MEMORY_RESERVE, map.num_objects + 1);

            for (ObjectCounter i = map.num_objects; i < (map.num_objects + OBJECTS_MEMORY_RESERVE); ++i) {
                map.objects[i] = (Object){0};
            }
        }

        MALLOC(new_obj.label, sizeof(char) * LABEL_DATA_MEMORY_RESERVE);
        strncpy(new_obj.label, DEFAULT_OBJECT_LABEL, LABEL_DATA_MEMORY_RESERVE);

        map.objects[map.num_objects] = new_obj;
        map.num_objects++;
    }
}

void CreateSphere(void) {
    CreatePrimitive((Object){
        .pos  = Vector3Zero(),
        .type = OT_Sphere,
        .data.Sphere.radius = 0.5f,
        .col  = RED
    });
}

void CreateCube(void) {
    CreatePrimitive((Object) {
        .pos  = Vector3Zero(),
        .type = OT_Cube,
        .data.Cube.dim = (Vector3){1.0f, 1.0f, 1.0f},
        .col  = RED
    });
}

ObjectCounter GetObjectIndexUnderMouse(Vector2 mousePosition) {
    Ray ray = GetMouseRay(mousePosition, *cam);

    for (ObjectCounter i = 0; i < map.num_objects; ++i) {
        Object cur_obj = map.objects[i];

        RayCollision obj_collision;

        switch(cur_obj.type) {
        case OT_Cube: {
            BoundingBox cube_bb = {
                .min = Vector3Subtract(cur_obj.pos, Vector3Scale(cur_obj.data.Cube.dim, 0.5f)),
                .max = Vector3Add(cur_obj.pos, Vector3Scale(cur_obj.data.Cube.dim, 0.5f))
            };
            obj_collision = GetRayCollisionBox(ray, cube_bb);
        } break;
        case OT_Sphere: {
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
        case OT_Cube: {
            DrawCubeV(cur_obj.pos, cur_obj.data.Cube.dim, cur_obj.col);
        } break;

        case OT_Sphere: {
            DrawSphere(cur_obj.pos, cur_obj.data.Sphere.radius, cur_obj.col);
        } break;

        default: {} break;
        }
    }
}

void DrawSelectedObjects(void) {
    /* Draw silhouettes of our selected objects so that the shader can properly draw an outline for them */
    if (num_selected_objects != (ObjectCounter)-1) {
        for (ObjectCounter i = 0; i <= num_selected_objects; ++i) {
            Object* cur_obj = selected_objects[i];

            switch (cur_obj->type) {
            case OT_Cube: {
                DrawCubeV(cur_obj->pos, cur_obj->data.Cube.dim, BLACK);
            } break;

            case OT_Sphere: {
                DrawSphere(cur_obj->pos, cur_obj->data.Sphere.radius, BLACK);
            } break;

            default: {} break;
            }
        }
    }
}

void DeInitGlobal(void) {
    FREE(cam);
    DeInitObjects();
    ShadersDeInit();
    DeInitGUI();
}

void LockCursor(void) {
    /* Lock cursor (ONLY if it's unlocked) */
    if (!cursor_disabled) {
        DisableCursor();
        LockGUI();
        cursor_disabled = true;
    }
}

void UnlockCursor(void) {
    /* Unlock cursor (ONLY if it's locked) */
    if (cursor_disabled) {
        EnableCursor();
        UnlockGUI();
        cursor_disabled = false;
    }
}

void EmptyObjectSelection(void) {
    if (num_selected_objects != (ObjectCounter)-1) {
        for (ObjectCounter i = 0; i < num_selected_objects; ++i) {
            selected_objects[i] = NULL;
        }
        if (num_selected_objects > SELECTED_OBJECTS_MEMORY_RESERVE) {
            REALLOC(selected_objects, sizeof(Object*) * SELECTED_OBJECTS_MEMORY_RESERVE);
        }
        num_selected_objects = (ObjectCounter)-1;
    }
}

void ResizeObjectSelection(void) {
    /* Resize selected_objects array if needed */
    if ((num_selected_objects + 1) % SELECTED_OBJECTS_MEMORY_RESERVE == 0) {
        REALLOC(selected_objects, sizeof(Object*) * (num_selected_objects + 1 + SELECTED_OBJECTS_MEMORY_RESERVE));
    }
}

void HandleResolutionChange(void) {
    int cur_render_w = GetRenderWidth();
    int cur_render_h = GetRenderHeight();
    if (screen_w != cur_render_w || screen_h != cur_render_h) {
        screen_w = cur_render_w;
        screen_h = cur_render_h;

        UnloadRenderTexture(renderTexture);
        renderTexture = LoadRenderTexture(screen_w, screen_h);
    }
}

void CameraUpdate(void) {
    if (cam_state) {
        const Vector2 mouseDelta = GetMouseDelta();

        switch (cam_state) {
        case CS_WantsToMoveFreely: {
            CameraYaw(cam, -mouseDelta.x*CAMERA_MOUSE_SENSITIVITY, false);
            CameraPitch(cam, -mouseDelta.y*CAMERA_MOUSE_SENSITIVITY, true, false, false);
        } break;

        case CS_WantsToPan: {
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

void DrawMoveHelpers(void) {
    rlSetLineWidth(XYZLINES_THICKNESS);
    if (move_state & OMS_WantsToMoveOnX) {
        rlBegin(RL_LINES);
            rlColor4ub(XLINE_COLOR.r, XLINE_COLOR.g, XLINE_COLOR.b, XLINE_COLOR.a);
            rlVertex3f(-GROUND_TOTAL_LENGTH, 0.002f, 0.0f);
            rlVertex3f(GROUND_TOTAL_LENGTH, 0.002f, 0.0f);
        rlEnd();
    } if (move_state & OMS_WantsToMoveOnZ) {
        rlBegin(RL_LINES);
            rlColor4ub(ZLINE_COLOR.r, ZLINE_COLOR.g, ZLINE_COLOR.b, ZLINE_COLOR.a);
            rlVertex3f(0.0f, 0.002f, -GROUND_TOTAL_LENGTH);
            rlVertex3f(0.0f, 0.002f, GROUND_TOTAL_LENGTH);
        rlEnd();
    } if (move_state & OMS_WantsToMoveOnY) {
        rlBegin(RL_LINES);
            rlColor4ub(YLINE_COLOR.r, YLINE_COLOR.g, YLINE_COLOR.b, YLINE_COLOR.a);
            rlVertex3f(0.0f, -GROUND_TOTAL_LENGTH, 0.0f);
            rlVertex3f(0.0f, GROUND_TOTAL_LENGTH, 0.0f);
        rlEnd();
    }

    rlSetLineWidth(default_line_width);
}

/* add address of an object from the object array to the selected_objects array */
void SelectObjectAtIndex(ObjectCounter idx) {
    if (idx == (ObjectCounter)-1) return;
    bool active = false;

    for (ObjectCounter j = 0; j <= num_selected_objects && num_selected_objects != (ObjectCounter)-1; ++j) {
        active = (selected_objects[j] == &map.objects[idx]);
        if (active) break;
    }
    
    if (!active) {
        ResizeObjectSelection();
        num_selected_objects++;
        selected_objects[num_selected_objects] = &map.objects[idx];
    }
}

void DeSelectObjectAtIndex(ObjectCounter idx) {
    if (idx == (ObjectCounter)-1 || idx > num_selected_objects || num_selected_objects == (ObjectCounter)-1) return;

    selected_objects[idx] = NULL;

    for (ObjectCounter i = idx; i < num_selected_objects; ++i) {
        selected_objects[i] = selected_objects[i + 1];
    }
    num_selected_objects--;

    ResizeObjectSelection();
}

/*                                  Dispatchers                                  */
void HandleEvents(void) {
    HandleResolutionChange();

    if (!IsInteractingWithGUI()) {
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            cam_state = CS_WantsToMoveFreely;
            move_state = OMS_Stationary;
            LockCursor();
        } else if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE) & IsKeyDown(KEY_LEFT_SHIFT)) {
            cam_state = CS_WantsToPan;
            move_state = OMS_Stationary;
            LockCursor();
        } else {
            cam_state = CS_Static;
            UnlockCursor();
        }

        cam_speed = (IsKeyDown(KEY_LEFT_SHIFT)) ? CAMERA_MAX_SPEED : CAMERA_MOVE_SPEED;

    }

    switch(cam_state) {
    case CS_Static: {
        /* Handle shortcuts or whatever */

        if (!IsInteractingWithGUI()) {
            Vector2 mouse_position = GetMousePosition();

            /*                Object selection                */
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                ObjectCounter obj_index = GetObjectIndexUnderMouse(mouse_position);

                if (IsKeyDown(KEY_LEFT_CONTROL)) {
                    /*      Multiple object selection      */
                    if (obj_index != (ObjectCounter)-1) {
                        /* User clicked on an object */
                        SelectObjectAtIndex(obj_index);
                    }
                    /* ----------------------------------- */
                } else {
                    /*       Single object selection       */
                    EmptyObjectSelection();
                    if (obj_index != (ObjectCounter)-1) {
                        /* User clicked on an object */
                        SelectObjectAtIndex(obj_index);
                    }
                    /* ----------------------------------- */
                }

            }
            /* ---------------------------------------------- */

            float mouse_wheel_delta = GetMouseWheelMove();
            /*Having an if (mouse_wheel_delta) before this should be technically more correct,
            but i prefer this look. If issues arise then no doubt i'll change it*/
            CameraMoveForward(cam, CAMERA_ZOOM_SPEED*mouse_wheel_delta, false);
            
            if (IsKeyPressed(KEY_G) && num_selected_objects != (ObjectCounter)-1) {
                move_state = OMS_WantsToMoveOnX | OMS_WantsToMoveOnZ;
            }
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) move_state = OMS_Stationary;

        if (move_state != OMS_Stationary) {
            Vector2 mouse_delta = GetMouseDelta();

            if (IsKeyDown(KEY_X)) {
                move_state = OMS_WantsToMoveOnX;
            } else if (IsKeyDown(KEY_Y)) {
                move_state = OMS_WantsToMoveOnY;
            } else if (IsKeyDown(KEY_Z)) {
                move_state = OMS_WantsToMoveOnZ;
            }

            for (ObjectCounter i = 0; i <= num_selected_objects; ++i) {
                if (move_state & OMS_WantsToMoveOnX) {
                    selected_objects[i]->pos.x += (mouse_delta.x) * OBJECT_MOVE_SPEED;
                } if (move_state & OMS_WantsToMoveOnZ) {
                    selected_objects[i]->pos.z += (mouse_delta.y) * OBJECT_MOVE_SPEED;
                } if (move_state & OMS_WantsToMoveOnY) {
                    selected_objects[i]->pos.y += (mouse_delta.x + -mouse_delta.y) * OBJECT_MOVE_SPEED;
                }
            }
        }

        if (IsKeyDown(KEY_LEFT_CONTROL) & IsKeyPressed(KEY_S)) ExportMap();

    } break;

    case CS_WantsToMoveFreely: {
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

void Draw(void) {
    BeginDrawing(); {
        ClearBackground(RAYWHITE);

        BeginMode3D(*cam); {
            DrawGrid((int)GROUND_TOTAL_LENGTH, 1.0f);
            DrawMoveHelpers();
            DrawObjects();

        } EndMode3D();
        
        /*Next section is HEAVILY based on https://github.com/denysmaistruk/raylib-model-outline
          Huge thanks to Denys Maistruk for this wonderful code!*/
        BeginTextureMode(renderTexture); {
            ClearBackground(WHITE);

            BeginMode3D(*cam); {
                DrawSelectedObjects();
            } EndMode3D();
        } EndTextureMode(); 

        BeginShaderMode(outline_shader); {
            DrawTextureRec(renderTexture.texture, 
                (Rectangle){ 0.0f, 0.0f, (float)renderTexture.texture.width, (float)-renderTexture.texture.height }, (Vector2){ 0.0f, 0.0f }, WHITE);
        } EndShaderMode();
        /*--------------------------------------------------------------------------------------*/

        DrawGUI(selected_objects, &num_selected_objects, &map);
    } EndDrawing();
}

/* ----------------------------------------------------------------------------- */

void AskToLeave(void) {
    DeInitGlobal();
    CloseWindow();
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

    AskToLeave();

    return 0;
}
