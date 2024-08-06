#ifndef   EDITOR_H
#define   EDITOR_H

#ifdef __cplusplus
extern "C" {
#endif

void InitGlobal(void);
void DeInitGlobal(void);

void HandleEvents(void);
void Update(void);
void Draw(void);

void CreateCube(void);

void CameraUpdate(void);

typedef enum CameraState {
    Static,
    WantsToMoveFreely,
    WantsToPan
} CameraState;

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

#ifdef __cplusplus
}
#endif

#endif /* EDITOR_H */