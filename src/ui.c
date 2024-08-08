#include <raygui.h>

#include <math.h>
#include "ui.h"

void DrawGUI(ObjectCounter selected_object_index, Object* selected_object, ObjectCounter num_objects, Object* objects) {
    /*      Right panel      */
    float right_panel_width = 80.0f;
    Rectangle right_panel = {
        .height=(float)GetRenderHeight(),
        .width=right_panel_width,
        .x=(float)GetRenderWidth() - right_panel_width,
        .y=0.0f
    };
    GuiPanel(right_panel, "Objects");

    float object_entry_height = 20.0f;
    for (ObjectCounter i = 0; i < num_objects; ++i) {
        Rectangle object_entry = {
            .height=object_entry_height,
            .width=right_panel_width,
            .x=(float)GetRenderWidth() - right_panel_width,
            .y=25.0f + object_entry_height * i
        };

        bool active = (i == selected_object_index);
        if (GuiToggle(object_entry, TextFormat("%d: (TYPE)%d", i, objects[i].type), &active)) {
            selected_object = &objects[i];
        }
    }

    /* --------------------- */

    if (GuiButton((Rectangle){.height=30.0f, .width=100.0f, .x=0.0f, .y=0.0f}, "#80#Create cube")) {
        CreateCube();
    }
}

void LockGUI(void) { GuiLock(); }
void UnlockGUI(void) { GuiUnlock(); }

void DisableGUI(void) { GuiDisable(); }
void EnableGUI(void) { GuiEnable(); }