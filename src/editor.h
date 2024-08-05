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
    WantsToMoveFreely
} CameraState;

#endif /* EDITOR_H */