#ifndef   UI_H
#define   UI_H

#include "editor.h"

#ifdef __cplusplus
extern "C" {
#endif

void InitGUI(void);
void DeInitGUI(void);

void DrawGUI(Object** selected_objects, ObjectCounter* num_selected_objects, Map* map);

bool IsInteractingWithGUI(void);

void UnlockGUI(void);
void LockGUI(void);

#ifdef __cplusplus
}
#endif

#endif /* UI_H */