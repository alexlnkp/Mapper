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
    Vector3 pos;
    Vector3 dim;
    Color col;
} Cube;

#endif /* EDITOR_H */