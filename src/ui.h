#ifndef   UI_H
#define   UI_H

#include "editor.h"

void DrawGUI(Object** selected_objects, ObjectCounter* num_selected_objects, ObjectCounter num_objects, Object* objects);

void UnlockGUI(void);
void LockGUI(void);

void DisableGUI(void);
void EnableGUI(void);

#endif /* UI_H */