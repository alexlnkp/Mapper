#ifndef   UI_H
#define   UI_H

#include "editor.h"

void InitGUI(void);
void DeInitGUI(void);

void DrawGUI(Object** selected_objects, ObjectCounter* num_selected_objects, ObjectCounter num_objects, Object* objects);

bool IsHoveringOverAnyGUIElement(void);

void UnlockGUI(void);
void LockGUI(void);

#endif /* UI_H */