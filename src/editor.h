#ifndef   EDITOR_H
#define   EDITOR_H

#include <raylib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum CameraState {
    CS_Static,
    CS_WantsToMoveFreely,
    CS_WantsToPan
} CameraState;

typedef unsigned ObjectCounter;

typedef enum ObjType {
    OT_Cube,
    OT_Sphere
} ObjectType;

static const char* obj_types[] = {"Cube", "Sphere"};

typedef struct Object {
    Vector3 pos; /* Center point of the object in the world space */
    ObjectType type;
    Color col;

    union {
        struct {
            Vector3 dim; /* Dimensionality of the cube; x for width, y for height, z for length */
        } Cube;

        struct {
            float radius;
        } Sphere;
    } data;

    char* label; /* ANY DATA */
} Object;

typedef enum {
    OMS_Stationary = 1 << 0,
    OMS_WantsToMoveOnX = 1 << 1,
    OMS_WantsToMoveOnZ = 1 << 2,
    OMS_WantsToMoveOnY = 1 << 3
} ObjectMoveState;

typedef struct MapMetadata {
    char* name;
    char* author;
} MapMetadata;

typedef struct Map {
    Vector3 dim; /* Dimensionality of the WHOLE MAP; x for width, y for height, z for length */
    Object* objects;
    ObjectCounter num_objects;
    MapMetadata meta;
} Map;

typedef struct {
    Camera* cam;
    CameraState cam_state;
    float cam_speed;
} CameraContext;

typedef struct {
    BoundingBox ground;
    Map map;

    Object** selected_objects; /* Holds the addresses to the selected objects in the map.objects array */
    ObjectCounter num_selected_objects; /* (ObjectCounter)-1 means no object selected, 0 means one is selected */
} MapContext;

typedef struct {
    bool cursor_disabled;
    ObjectMoveState move_state;
    float default_line_width;
    int screen_w; /* current screen width */
    int screen_h; /* current screen height */
} EditorContext;

typedef struct {
    RenderTexture renderTexture;
    Shader outline_shader;
    int sizeLoc;
} RenderContext;

typedef struct {
    CameraContext* c_ctx;
    EditorContext* e_ctx;
    MapContext* m_ctx;
    RenderContext* r_ctx;
} AppContext;

void AskToLeave(AppContext** app_ctx);

AppContext* InitGlobal(void);
void DeInitGlobal(AppContext** app_ctx);

void HandleEvents(AppContext* app_ctx);
void Update(CameraContext* c_ctx);
void Draw(AppContext* app_ctx);

void CreateSphere(MapContext* m_ctx, CameraContext* c_ctx);
void CreateCube(MapContext* m_ctx, CameraContext* c_ctx);

void ExportMap(MapContext* m_ctx);
void ImportMap(MapContext* m_ctx, char* file);

void ResizeObjectSelection(MapContext* m_ctx);

void CameraUpdate(CameraContext* c_ctx);

void SelectObjectAtIndex(MapContext* m_ctx, ObjectCounter idx);
void DeSelectObjectAtIndex(MapContext* m_ctx, ObjectCounter idx);

#ifdef __cplusplus
}
#endif

#endif /* EDITOR_H */
