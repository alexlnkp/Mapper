#ifndef   EDITOR_H
#define   EDITOR_H

void InitGlobal();
void DeInitGlobal();

void HandleEvents();
void Update();
void Draw();

void CameraUpdate();

typedef enum CameraState {
    Static,
    WantsToMoveFreely,
    WantsToPan
} CameraState;

typedef struct {
    Vector3 pos; /* Center point of the cube in the world space */
    Vector3 dim; /* Dimensionality of the cube; x for width, y for height, z for length */
    Color col;
} Cube;

#endif /* EDITOR_H */