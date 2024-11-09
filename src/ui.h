#ifndef   UI_H
#define   UI_H

#include "editor.h"

#ifdef __cplusplus
extern "C" {
#endif

void InitGUI(void);
void DeInitGUI(void);

void DrawGUI(AppContext* app_ctx);

bool IsInteractingWithGUI(void);

void UnlockGUI(void);
void LockGUI(void);

#ifdef __cplusplus
}
#endif

#endif /* UI_H */
