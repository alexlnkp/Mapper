#ifndef   EDITOR_H
#define   EDITOR_H

#ifdef __cplusplus
extern "C" {
#endif

void AskToLeave(void);

void InitGlobal(void);
void DeInitGlobal(void);

void HandleEvents(void);
void Update(void);
void Draw(void);

void CreateCube(void);

void ExportMap(void);

void CameraUpdate(void);

typedef enum CameraState {
    Static,
    WantsToMoveFreely,
    WantsToPan
} CameraState;

typedef unsigned ObjectCounter;

typedef enum ObjType {
    CUBE,
    SPHERE
} ObjectType;

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
} Object;

typedef enum {
    MS_Stationary = 1 << 0,
    MS_WantsToMoveOnX = 1 << 1,
    MS_WantsToMoveOnZ = 1 << 2,
    MS_WantsToMoveOnY = 1 << 3
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

#ifdef __cplusplus
}
#endif

#endif /* EDITOR_H */