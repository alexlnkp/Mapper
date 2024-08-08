#ifndef   UI_H
#define   UI_H

#include "editor.h"

void DrawGUI(ObjectCounter selected_object_index, Object* selected_object, ObjectCounter num_objects, Object* objects);

void UnlockGUI(void);
void LockGUI(void);

void DisableGUI(void);
void EnableGUI(void);

#endif /* UI_H */