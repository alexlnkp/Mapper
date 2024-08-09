#include "imgui_impl_raylib.h"
#include "rlcimgui.h"
#include <cimgui.h>

#include "ui.h"

void DrawContextMenu(void) {
    igSetNextWindowPos((ImVec2){0, 0}, 0, (ImVec2){0, 0});
    igSetNextWindowSize((ImVec2){(float)GetScreenWidth(), (float)GetScreenHeight()}, 0);

    /*we just want to use this window as a host for the menubar and docking
    so turn off everything that would make it act like a window*/
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus |
        ImGuiWindowFlags_NoDocking |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_MenuBar |
        ImGuiWindowFlags_NoBackground;
    
    igPushStyleVar_Vec2(ImGuiStyleVar_WindowPadding, (ImVec2){0, 0});
    bool show = (igBegin("Main", NULL, windowFlags)); {
        igPopStyleVar(1);

        igDockSpace(igGetID_Str("Dockspace"), (ImVec2){0.0f, 0.0f}, ImGuiDockNodeFlags_PassthruCentralNode, NULL);

        if (show) {
            if (igBeginMenuBar()) {
                if (igBeginMenu("File", true)) {
                    if (igMenuItem_Bool("Exit", "x", false, true)) AskToLeave();
                    if (igMenuItem_Bool("Export map", "s", false, true)) ExportMap();
                    
                    igEndMenu();
                }
                if (igBeginMenu("Create", true)) {
                    if (igMenuItem_Bool("Cube", "C", false, true)) CreateCube();
                    igEndMenu();
                }

                igEndMenuBar();
            }
        }
    } igEnd();
}

void DrawObjectListPanel(Object** selected_objects, ObjectCounter* num_selected_objects, ObjectCounter num_objects, Object* objects) {
    float right_panel_width = 150.0f;
    
    igSetNextWindowPos((ImVec2){.x=GetScreenWidth() - right_panel_width, .y=0}, 1, (ImVec2){0, 0});
    igSetNextWindowSize((ImVec2){.x=right_panel_width, .y=(float)GetScreenHeight()}, 1);

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize;
    bool show = (igBegin("Object list", NULL, windowFlags)); {
        for (ObjectCounter i = 0; i < num_objects; ++i) {
            bool active = false;
            for (ObjectCounter j = 0; j < *num_selected_objects; ++j) {
                active = (selected_objects[j] == &objects[i]);
                if (active) break;
            }
            if (igCheckbox(TextFormat("%d: (TYPE)%d", i, objects[i].type), &active)) {
                selected_objects[*num_selected_objects] = &objects[i];
                (*num_selected_objects)++;
            }
        }
    } igEnd();
}

void BeginGUIDraw(void) {
    ImGui_ImplRaylib_ProcessEvents();

    /* start the Dear ImGui frame */
    ImGui_ImplRaylib_NewFrame();
    igNewFrame();
}

void EndGUIDraw(void) {
    igRender();
    ImGui_ImplRaylib_RenderDrawData(igGetDrawData());
}

void DrawGUI(Object** selected_objects, ObjectCounter* num_selected_objects, ObjectCounter num_objects, Object* objects) {
    BeginGUIDraw(); {
        DrawObjectListPanel(selected_objects, num_selected_objects, num_objects, objects);
        DrawContextMenu();
    } EndGUIDraw();
}

void InitGUI(void) {
    rligSetup(true);

    igCreateContext(NULL);
    ImGuiIO *ioptr = igGetIO();

#ifdef IMGUI_HAS_DOCK
    ioptr->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
#endif

    ImGui_ImplRaylib_Init();
    ImFontAtlas_AddFontDefault(ioptr->Fonts, NULL);
    rligSetupFontAwesome();

    /* required to be called to cache the font texture with raylib */
    ImGui_ImplRaylib_BuildFontAtlas();
}
void DeInitGUI(void) { ImGui_ImplRaylib_Shutdown(); igDestroyContext(NULL); }

void LockGUI(void) {  }
void UnlockGUI(void) {  }
