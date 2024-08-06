#include <raygui.h>

#include "editor.h"

void DrawGUI(void) {
    if (GuiButton((Rectangle){.height=30.0f, .width=100.0f, .x=0.0f, .y=0.0f}, "#80#Create cube")) {
        CreateCube();
    }
}

void LockGUI(void) { GuiLock(); }
void UnlockGUI(void) { GuiUnlock(); }

void DisableGUI(void) { GuiDisable(); }
void EnableGUI(void) { GuiEnable(); }