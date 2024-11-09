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

CameraContext* CameraInit(void) {
    CameraContext* cctx = MALLOCFUNC(sizeof(CameraContext));
    MALLOC(cctx->cam, sizeof(Camera));

    cctx->cam->position = (Vector3){0.0f, 1.0f, 0.0f};
    cctx->cam->target   = (Vector3){1.0f, 1.0f, 0.0f};
    cctx->cam->up       = (Vector3){0.0f, 1.0f, 0.0f};
    cctx->cam->fovy = vFov_from_hFov(CAMERA_FOV, ((float)WINDOW_WIDTH / (float)WINDOW_HEIGHT));
    cctx->cam->projection = CAMERA_PERSPECTIVE;

    cctx->cam_state = CS_Static;
    cctx->cam_speed = CAMERA_MOVE_SPEED;

    return cctx;
}

void ObjectsInit(MapContext* map_ctx) {
    map_ctx->map.num_objects = 0;
    MALLOC(map_ctx->map.objects, sizeof(Object) * OBJECTS_MEMORY_RESERVE);
    for (ObjectCounter i = map_ctx->map.num_objects; i < OBJECTS_MEMORY_RESERVE; ++i) {
        map_ctx->map.objects[i] = (Object){0};
    }
    MALLOC(map_ctx->map.meta.name, sizeof(char) * (strlen(DEFAULT_MAP_NAME) + 1 + MAP_META_FIELD_MEMORY_RESERVE));
    strcpy(map_ctx->map.meta.name, DEFAULT_MAP_NAME);

    MALLOC(map_ctx->map.meta.author, sizeof(char) * (strlen(DEFAULT_MAP_AUTHOR) + 1 + MAP_META_FIELD_MEMORY_RESERVE));
    strcpy(map_ctx->map.meta.author, DEFAULT_MAP_AUTHOR);

    MALLOC(map_ctx->selected_objects, sizeof(Object*) * SELECTED_OBJECTS_MEMORY_RESERVE);
    map_ctx->num_selected_objects = (ObjectCounter)-1;
}

void DeInitObjects(MapContext** map_ctx) {
    (*map_ctx)->num_selected_objects = (ObjectCounter)-1;
    FREE((*map_ctx)->selected_objects);
    for (ObjectCounter i = 0; i < (*map_ctx)->map.num_objects; ++i) {
        (*map_ctx)->map.objects[i] = (Object){0};
        FREE((*map_ctx)->map.objects[i].label);
    }
    FREE((*map_ctx)->map.objects);
    (*map_ctx)->map.num_objects = 0;

    FREE((*map_ctx)->map.meta.author);
    FREE((*map_ctx)->map.meta.name);
    FREE(*map_ctx);
}

void MapInit(MapContext* map_ctx) {
    map_ctx->map.dim = (Vector3){
        .x=GROUND_TOTAL_LENGTH,
        .y=GROUND_TOTAL_LENGTH, /* Filler value, will be replaced later */
        .z=GROUND_TOTAL_LENGTH
    };
    map_ctx->map.meta = (MapMetadata){
        .author="",
        .name=""
    };

    ObjectsInit(map_ctx);
}

void ShadersInit(EditorContext* ectx, RenderContext* r_ctx) {
    ectx->screen_w = GetRenderWidth();
    ectx->screen_h = GetRenderHeight();

    int outline_size = OUTLINE_WIDTH;
    const float outlineColor[4] = OUTLINE_COLOR;

    r_ctx->outline_shader = LoadShader(0, OUTLINE_SHADER_LOCATION);

    SetShaderValue(r_ctx->outline_shader, GetShaderLocation(r_ctx->outline_shader, "width"), &ectx->screen_w, SHADER_UNIFORM_INT);
    SetShaderValue(r_ctx->outline_shader, GetShaderLocation(r_ctx->outline_shader, "height"), &ectx->screen_h, SHADER_UNIFORM_INT);
    SetShaderValue(r_ctx->outline_shader, GetShaderLocation(r_ctx->outline_shader, "color"), outlineColor, SHADER_UNIFORM_VEC4);

    r_ctx->sizeLoc = GetShaderLocation(r_ctx->outline_shader, "size");
    SetShaderValue(r_ctx->outline_shader, r_ctx->sizeLoc, &outline_size, SHADER_UNIFORM_INT);

    r_ctx->renderTexture = LoadRenderTexture(ectx->screen_w, ectx->screen_h);
}

void ShadersDeInit(RenderContext** r_ctx) {
    UnloadShader((*r_ctx)->outline_shader);
    UnloadRenderTexture((*r_ctx)->renderTexture);
    FREE(*r_ctx);
}

AppContext* InitGlobal(void) {
    AppContext* app_ctx = MALLOCFUNC(sizeof(AppContext));
    app_ctx->e_ctx = MALLOCFUNC(sizeof(EditorContext));
    app_ctx->r_ctx = MALLOCFUNC(sizeof(RenderContext));
    app_ctx->m_ctx = MALLOCFUNC(sizeof(MapContext));

    app_ctx->e_ctx->cursor_disabled = false;

    app_ctx->c_ctx = CameraInit();

    app_ctx->e_ctx->move_state = OMS_Stationary;

    app_ctx->m_ctx->ground.min = GROUND_MIN_BOUND;
    app_ctx->m_ctx->ground.max = GROUND_MAX_BOUND;
    MapInit(app_ctx->m_ctx);

    app_ctx->e_ctx->default_line_width = rlGetLineWidth();
    ShadersInit(app_ctx->e_ctx, app_ctx->r_ctx);

    InitGUI();
    return app_ctx;
}

void SetMapMetadata(MapContext* m_ctx, MapMetadata new_metadata) {
    m_ctx->map.meta = new_metadata;
}

void ExportMap(MapContext* m_ctx) {
    m_ctx->map.meta.author = (strcmp(m_ctx->map.meta.author, "") == 0) ? DEFAULT_MAP_AUTHOR : m_ctx->map.meta.author;
    m_ctx->map.meta.name = (strcmp(m_ctx->map.meta.name, "") == 0) ? DEFAULT_MAP_NAME : m_ctx->map.meta.name;

    FILE *fp = fopen(m_ctx->map.meta.name, "wb");
    assert(fp != NULL);

    fwrite(&m_ctx->map.num_objects, sizeof(ObjectCounter), 1, fp);

    for (ObjectCounter i = 0; i < m_ctx->map.num_objects; ++i) {
        Object cur_obj = m_ctx->map.objects[i];
        fwrite(&cur_obj.pos, sizeof(Vector3), 1, fp);
        fwrite(&cur_obj.type, sizeof(ObjectType), 1, fp);
        fwrite(&cur_obj.col, sizeof(Color), 1, fp);
        fwrite(&cur_obj.data, sizeof(cur_obj.data), 1, fp);
        fwrite(cur_obj.label, sizeof(char) * (strlen(cur_obj.label) + 1), 1, fp);
    }

    /* Meta is the last to be stored */
    fwrite(m_ctx->map.meta.name, sizeof(char) * (strlen(m_ctx->map.meta.name) + 1), 1, fp);
    fwrite(m_ctx->map.meta.author, sizeof(char) * (strlen(m_ctx->map.meta.author) + 1), 1, fp);

    fclose(fp);
}

void ImportMap(MapContext* m_ctx, char* file) {
    FILE *fp = fopen(file, "rb");
    assert(fp != NULL);

    TraceLog(LOG_DEBUG, "Reading file in ImportMap() function. File: %s", file);

    fread(&m_ctx->map.num_objects, sizeof(ObjectCounter), 1, fp);

    for (ObjectCounter i = 0; i < m_ctx->map.num_objects; ++i) {
        fread(&m_ctx->map.objects[i].pos, sizeof(Vector3), 1, fp);
        fread(&m_ctx->map.objects[i].type, sizeof(ObjectType), 1, fp);
        fread(&m_ctx->map.objects[i].col, sizeof(Color), 1, fp);
        fread(&m_ctx->map.objects[i].data, sizeof(m_ctx->map.objects[i].data), 1, fp);
        m_ctx->map.objects[i].label = ReadStringFromStream(fp);
    }

    fclose(fp);
}

void CreatePrimitive(MapContext* m_ctx, CameraContext* c_ctx, Object new_obj) {
    Ray ray;
    ray.position = c_ctx->cam->position;
    ray.direction = GetCameraForward(c_ctx->cam);

    RayCollision ground_collision = GetRayCollisionBox(ray, m_ctx->ground);

    if (ground_collision.hit) {
        Vector3 col_point = ground_collision.point;
        new_obj.pos = (Vector3){col_point.x, col_point.y + 0.5f, col_point.z};

        if ((m_ctx->map.num_objects + 1) % OBJECTS_MEMORY_RESERVE == 0) {
            REALLOC(m_ctx->map.objects, sizeof(Object) * (m_ctx->map.num_objects + OBJECTS_MEMORY_RESERVE));
            TraceLog(LOG_DEBUG, "Realloc'd for %d more objects! Objects total: %d", OBJECTS_MEMORY_RESERVE, m_ctx->map.num_objects + 1);

            for (ObjectCounter i = m_ctx->map.num_objects; i < (m_ctx->map.num_objects + OBJECTS_MEMORY_RESERVE); ++i) {
                m_ctx->map.objects[i] = (Object){0};
            }
        }

        MALLOC(new_obj.label, sizeof(char) * LABEL_DATA_MEMORY_RESERVE);
        strncpy(new_obj.label, DEFAULT_OBJECT_LABEL, LABEL_DATA_MEMORY_RESERVE);

        m_ctx->map.objects[m_ctx->map.num_objects] = new_obj;
        m_ctx->map.num_objects++;
    }
}

void CreateSphere(MapContext* m_ctx, CameraContext* c_ctx) {
    CreatePrimitive(m_ctx, c_ctx, (Object){
        .pos  = Vector3Zero(),
        .type = OT_Sphere,
        .data.Sphere.radius = 0.5f,
        .col  = RED
    });
}

void CreateCube(MapContext* m_ctx, CameraContext* c_ctx) {
    CreatePrimitive(m_ctx, c_ctx, (Object) {
        .pos  = Vector3Zero(),
        .type = OT_Cube,
        .data.Cube.dim = (Vector3){1.0f, 1.0f, 1.0f},
        .col  = RED
    });
}

ObjectCounter GetObjectIndexUnderMouse(MapContext* m_ctx, CameraContext* c_ctx, Vector2 mousePosition) {
    Ray ray = GetMouseRay(mousePosition, *(c_ctx->cam));

    for (ObjectCounter i = 0; i < m_ctx->map.num_objects; ++i) {
        Object cur_obj = m_ctx->map.objects[i];

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

void DrawObjects(MapContext* m_ctx) {
    for (ObjectCounter i = 0; i < m_ctx->map.num_objects; ++i) {
        Object cur_obj = m_ctx->map.objects[i];

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

void DrawSelectedObjects(MapContext* m_ctx) {
    /* Draw silhouettes of our selected objects so that the shader can properly draw an outline for them */
    if (m_ctx->num_selected_objects != (ObjectCounter)-1) {
        for (ObjectCounter i = 0; i <= m_ctx->num_selected_objects; ++i) {
            Object* cur_obj = m_ctx->selected_objects[i];

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

void DeInitGlobal(AppContext** app_ctx) {
    FREE((*app_ctx)->c_ctx->cam);
    DeInitObjects(&(*app_ctx)->m_ctx);
    ShadersDeInit(&(*app_ctx)->r_ctx);
    DeInitGUI();
    FREE(*app_ctx);
}

void LockCursor(EditorContext* e_ctx) {
    /* Lock cursor (ONLY if it's unlocked) */
    if (!e_ctx->cursor_disabled) {
        DisableCursor();
        LockGUI();
        e_ctx->cursor_disabled = true;
    }
}

void UnlockCursor(EditorContext* e_ctx) {
    /* Unlock cursor (ONLY if it's locked) */
    if (e_ctx->cursor_disabled) {
        EnableCursor();
        UnlockGUI();
        e_ctx->cursor_disabled = false;
    }
}

void EmptyObjectSelection(MapContext* m_ctx) {
    if (m_ctx->num_selected_objects != (ObjectCounter)-1) {
        for (ObjectCounter i = 0; i < m_ctx->num_selected_objects; ++i) {
            m_ctx->selected_objects[i] = NULL;
        }
        if (m_ctx->num_selected_objects > SELECTED_OBJECTS_MEMORY_RESERVE) {
            REALLOC(m_ctx->selected_objects, sizeof(Object*) * SELECTED_OBJECTS_MEMORY_RESERVE);
        }
        m_ctx->num_selected_objects = (ObjectCounter)-1;
    }
}

void ResizeObjectSelection(MapContext* m_ctx) {
    /* Resize selected_objects array if needed */
    if ((m_ctx->num_selected_objects + 1) % SELECTED_OBJECTS_MEMORY_RESERVE == 0) {
        REALLOC(m_ctx->selected_objects, sizeof(Object*) * (m_ctx->num_selected_objects + 1 + SELECTED_OBJECTS_MEMORY_RESERVE));
    }
}

void HandleResolutionChange(EditorContext* e_ctx, RenderContext* r_ctx) {
    int cur_render_w = GetRenderWidth();
    int cur_render_h = GetRenderHeight();
    if (e_ctx->screen_w != cur_render_w || e_ctx->screen_h != cur_render_h) {
        e_ctx->screen_w = cur_render_w;
        e_ctx->screen_h = cur_render_h;

        UnloadRenderTexture(r_ctx->renderTexture);
        r_ctx->renderTexture = LoadRenderTexture(e_ctx->screen_w, e_ctx->screen_h);
    }
}

void CameraUpdate(CameraContext* c_ctx) {
    if (c_ctx->cam_state) {
        const Vector2 mouseDelta = GetMouseDelta();

        switch (c_ctx->cam_state) {
        case CS_WantsToMoveFreely: {
            CameraYaw(c_ctx->cam, -mouseDelta.x*CAMERA_MOUSE_SENSITIVITY, false);
            CameraPitch(c_ctx->cam, -mouseDelta.y*CAMERA_MOUSE_SENSITIVITY, true, false, false);
        } break;

        case CS_WantsToPan: {
            if (mouseDelta.x != 0.0f) {
                CameraMoveRight(c_ctx->cam, CAMERA_PAN_SPEED * mouseDelta.x, false);
            }
            if (mouseDelta.y != 0.0f) {
                CameraMoveUp(c_ctx->cam, CAMERA_PAN_SPEED * -mouseDelta.y);
            }

        } break;

        default: {} break;
        }
    }
}

void DrawMoveHelpers(EditorContext* e_ctx) {
    rlSetLineWidth(XYZLINES_THICKNESS);
    if (e_ctx->move_state & OMS_WantsToMoveOnX) {
        rlBegin(RL_LINES);
            rlColor4ub(XLINE_COLOR.r, XLINE_COLOR.g, XLINE_COLOR.b, XLINE_COLOR.a);
            rlVertex3f(-GROUND_TOTAL_LENGTH, 0.002f, 0.0f);
            rlVertex3f(GROUND_TOTAL_LENGTH, 0.002f, 0.0f);
        rlEnd();
    } if (e_ctx->move_state & OMS_WantsToMoveOnZ) {
        rlBegin(RL_LINES);
            rlColor4ub(ZLINE_COLOR.r, ZLINE_COLOR.g, ZLINE_COLOR.b, ZLINE_COLOR.a);
            rlVertex3f(0.0f, 0.002f, -GROUND_TOTAL_LENGTH);
            rlVertex3f(0.0f, 0.002f, GROUND_TOTAL_LENGTH);
        rlEnd();
    } if (e_ctx->move_state & OMS_WantsToMoveOnY) {
        rlBegin(RL_LINES);
            rlColor4ub(YLINE_COLOR.r, YLINE_COLOR.g, YLINE_COLOR.b, YLINE_COLOR.a);
            rlVertex3f(0.0f, -GROUND_TOTAL_LENGTH, 0.0f);
            rlVertex3f(0.0f, GROUND_TOTAL_LENGTH, 0.0f);
        rlEnd();
    }

    rlSetLineWidth(e_ctx->default_line_width);
}

/* add address of an object from the object array to the selected_objects array */
void SelectObjectAtIndex(MapContext* m_ctx, ObjectCounter idx) {
    if (idx == (ObjectCounter)-1) return;
    bool active = false;

    for (ObjectCounter j = 0; j <= m_ctx->num_selected_objects && m_ctx->num_selected_objects != (ObjectCounter)-1; ++j) {
        active = (m_ctx->selected_objects[j] == &m_ctx->map.objects[idx]);
        if (active) break;
    }

    if (!active) {
        ResizeObjectSelection(m_ctx);
        m_ctx->num_selected_objects++;
        m_ctx->selected_objects[m_ctx->num_selected_objects] = &m_ctx->map.objects[idx];
    }
}

void DeSelectObjectAtIndex(MapContext* m_ctx, ObjectCounter idx) {
    if (idx == (ObjectCounter)-1 || idx > m_ctx->num_selected_objects || m_ctx->num_selected_objects == (ObjectCounter)-1) return;

    m_ctx->selected_objects[idx] = NULL;

    for (ObjectCounter i = idx; i < m_ctx->num_selected_objects; ++i) {
        m_ctx->selected_objects[i] = m_ctx->selected_objects[i + 1];
    }
    m_ctx->num_selected_objects--;

    ResizeObjectSelection(m_ctx);
}

/*                                  Dispatchers                                  */
void HandleEvents(AppContext* app_ctx) {
    HandleResolutionChange(app_ctx->e_ctx, app_ctx->r_ctx);

    if (!IsInteractingWithGUI()) {
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            app_ctx->c_ctx->cam_state = CS_WantsToMoveFreely;
            app_ctx->e_ctx->move_state = OMS_Stationary;
            LockCursor(app_ctx->e_ctx);
        } else if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE) & IsKeyDown(KEY_LEFT_SHIFT)) {
            app_ctx->c_ctx->cam_state = CS_WantsToPan;
            app_ctx->e_ctx->move_state = OMS_Stationary;
            LockCursor(app_ctx->e_ctx);
        } else {
            app_ctx->c_ctx->cam_state = CS_Static;
            UnlockCursor(app_ctx->e_ctx);
        }

        app_ctx->c_ctx->cam_speed = (IsKeyDown(KEY_LEFT_SHIFT)) ? CAMERA_MAX_SPEED : CAMERA_MOVE_SPEED;

    }

    switch(app_ctx->c_ctx->cam_state) {
    case CS_Static: {
        /* Handle shortcuts or whatever */

        if (!IsInteractingWithGUI()) {
            Vector2 mouse_position = GetMousePosition();

            /*                Object selection                */
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                ObjectCounter obj_index = GetObjectIndexUnderMouse(app_ctx->m_ctx, app_ctx->c_ctx, mouse_position);

                if (IsKeyDown(KEY_LEFT_CONTROL)) {
                    /*      Multiple object selection      */
                    if (obj_index != (ObjectCounter)-1) {
                        /* User clicked on an object */
                        SelectObjectAtIndex(app_ctx->m_ctx, obj_index);
                    }
                    /* ----------------------------------- */
                } else {
                    /*       Single object selection       */
                    EmptyObjectSelection(app_ctx->m_ctx);
                    if (obj_index != (ObjectCounter)-1) {
                        /* User clicked on an object */
                        SelectObjectAtIndex(app_ctx->m_ctx, obj_index);
                    }
                    /* ----------------------------------- */
                }

            }
            /* ---------------------------------------------- */

            float mouse_wheel_delta = GetMouseWheelMove();
            /*Having an if (mouse_wheel_delta) before this should be technically more correct,
            but i prefer this look. If issues arise then no doubt i'll change it*/
            CameraMoveForward(app_ctx->c_ctx->cam, CAMERA_ZOOM_SPEED*mouse_wheel_delta, false);

            if (IsKeyPressed(KEY_G) && app_ctx->m_ctx->num_selected_objects != (ObjectCounter)-1) {
                app_ctx->e_ctx->move_state = OMS_WantsToMoveOnX | OMS_WantsToMoveOnZ;
            }
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) app_ctx->e_ctx->move_state = OMS_Stationary;

        if (app_ctx->e_ctx->move_state != OMS_Stationary) {
            Vector2 mouse_delta = GetMouseDelta();

            if (IsKeyDown(KEY_X)) {
                app_ctx->e_ctx->move_state = OMS_WantsToMoveOnX;
            } else if (IsKeyDown(KEY_Y)) {
                app_ctx->e_ctx->move_state = OMS_WantsToMoveOnY;
            } else if (IsKeyDown(KEY_Z)) {
                app_ctx->e_ctx->move_state = OMS_WantsToMoveOnZ;
            }

            for (ObjectCounter i = 0; i <= app_ctx->m_ctx->num_selected_objects; ++i) {
                if (app_ctx->e_ctx->move_state & OMS_WantsToMoveOnX) {
                    app_ctx->m_ctx->selected_objects[i]->pos.x += (mouse_delta.x) * OBJECT_MOVE_SPEED;
                } if (app_ctx->e_ctx->move_state & OMS_WantsToMoveOnZ) {
                    app_ctx->m_ctx->selected_objects[i]->pos.z += (mouse_delta.y) * OBJECT_MOVE_SPEED;
                } if (app_ctx->e_ctx->move_state & OMS_WantsToMoveOnY) {
                    app_ctx->m_ctx->selected_objects[i]->pos.y += (mouse_delta.x + -mouse_delta.y) * OBJECT_MOVE_SPEED;
                }
            }
        }

        if (IsKeyDown(KEY_LEFT_CONTROL)) {
            if (IsKeyPressed(KEY_S)) ExportMap(app_ctx->m_ctx);
        }

        if (IsKeyDown(KEY_LEFT_SHIFT)) {
            if (IsKeyPressed(KEY_C)) CreateCube(app_ctx->m_ctx, app_ctx->c_ctx);
            if (IsKeyPressed(KEY_S)) CreateSphere(app_ctx->m_ctx, app_ctx->c_ctx);
        }

    } break;

    case CS_WantsToMoveFreely: {
        /* Handle camera movement for free cam */

        char z = IsKeyDown(KEY_W) - IsKeyDown(KEY_S);
        char x = IsKeyDown(KEY_D) - IsKeyDown(KEY_A);

        CameraMoveForward(app_ctx->c_ctx->cam, app_ctx->c_ctx->cam_speed * z, false);
        CameraMoveRight(app_ctx->c_ctx->cam, app_ctx->c_ctx->cam_speed * x, false);

    } break;

    default: {
    } break;
    }
}

void Update(CameraContext* c_ctx) {
    CameraUpdate(c_ctx);
}

void Draw(AppContext* app_ctx) {
    BeginDrawing(); {
        ClearBackground(RAYWHITE);

        BeginMode3D(*app_ctx->c_ctx->cam); {
            DrawGrid((int)GROUND_TOTAL_LENGTH, 1.0f);
            DrawMoveHelpers(app_ctx->e_ctx);
            DrawObjects(app_ctx->m_ctx);

        } EndMode3D();

        /*Next section is HEAVILY based on https://github.com/denysmaistruk/raylib-model-outline
          Huge thanks to Denys Maistruk for this wonderful code!*/
        BeginTextureMode(app_ctx->r_ctx->renderTexture); {
            ClearBackground(WHITE);

            BeginMode3D(*app_ctx->c_ctx->cam); {
                DrawSelectedObjects(app_ctx->m_ctx);
            } EndMode3D();
        } EndTextureMode();

        BeginShaderMode(app_ctx->r_ctx->outline_shader); {
            DrawTextureRec(app_ctx->r_ctx->renderTexture.texture,
                (Rectangle){ 0.0f, 0.0f, (float)app_ctx->r_ctx->renderTexture.texture.width, (float)-app_ctx->r_ctx->renderTexture.texture.height }, (Vector2){ 0.0f, 0.0f }, WHITE);
        } EndShaderMode();
        /*--------------------------------------------------------------------------------------*/

        DrawGUI(app_ctx);
    } EndDrawing();
}

/* ----------------------------------------------------------------------------- */

void AskToLeave(AppContext** app_ctx) {
    DeInitGlobal(app_ctx);
    CloseWindow();
}

int main(void) {
    SetConfigFlags(FLAG_WINDOW_UNDECORATED | FLAG_MSAA_4X_HINT);
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);
    SetTargetFPS(TARGET_FPS);

    AppContext* app_ctx = InitGlobal();

    while (!WindowShouldClose()) {
        HandleEvents(app_ctx);

        Update(app_ctx->c_ctx);

        Draw(app_ctx);
    }

    AskToLeave(&app_ctx);

    return 0;
}
