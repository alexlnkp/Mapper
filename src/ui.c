#include <assert.h>

#include "imgui_impl_raylib.h"
#include "rlcimgui.h"
#include <cimgui.h>

#include "ui.h"

const char* GetObjectTypeString(ObjectType type) {
    switch (type) {
    case OT_Cube: return "Cube";
    case OT_Sphere: return "Sphere";
    default: return "IDK";
    }

    assert(false);
    return "";
}

void MakeDockSpace(void) {
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;

    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
    // because it would be confusing to have two docking targets within each others.
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

    ImGuiViewport* viewport = igGetMainViewport();
    igSetNextWindowPos(viewport->Pos, ImGuiCond_Always, (ImVec2){0,0});
    igSetNextWindowSize(viewport->Size, ImGuiCond_Always);
    igSetNextWindowViewport(viewport->ID);
    igPushStyleVar_Float(ImGuiStyleVar_WindowRounding, 0.0f);
    igPushStyleVar_Float(ImGuiStyleVar_WindowBorderSize, 0.0f);
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;


    // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;

    // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
    // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive, 
    // all active windows docked into it will lose their parent and become undocked.
    // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise 
    // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
    igPushStyleVar_Vec2(ImGuiStyleVar_WindowPadding, (ImVec2){0.0f, 0.0f});
    igBegin("DockSpace", NULL, window_flags);
    igPopStyleVar(1);
    igPopStyleVar(2);

    // DockSpace
    ImGuiIO* io = igGetIO();
    if (io->ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        ImGuiID dockspace_id = igGetID_Str("MyDockSpace");
        igDockSpace(dockspace_id, (ImVec2){0.0f, 0.0f}, dockspace_flags, ImGuiWindowClass_ImGuiWindowClass());

        static bool first_time = true;
        if (first_time) {
            first_time = false;

            igDockBuilderRemoveNode(dockspace_id); // clear any previous layout
            igDockBuilderAddNode(dockspace_id, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
            igDockBuilderSetNodeSize(dockspace_id, viewport->Size);

            // split the dockspace into 2 nodes -- DockBuilderSplitNode takes in the following args in the following order
            //   window ID to split, direction, fraction (between 0 and 1), the final two setting let's us choose which id we want (which ever one we DON'T set as NULL, will be returned by the function)
            //                                                              out_id_at_dir is the id of the node in the direction we specified earlier, out_id_at_opposite_dir is in the opposite direction
            ImGuiID dock_id_left = igDockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.2f, NULL, &dockspace_id);
            ImGuiID dock_id_down = igDockBuilderSplitNode(dockspace_id, ImGuiDir_Down, 0.25f, NULL, &dockspace_id);

            // we now dock our windows into the docking node we made above
            igDockBuilderDockWindow("Down", dock_id_down);
            igDockBuilderDockWindow("Left", dock_id_left);
            igDockBuilderFinish(dockspace_id);
        }
    }
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

void DrawContextMenu(void) {
    if (igBeginMenuBar()) {
        if (igBeginMenu("File", true)) {
            if (igMenuItem_Bool("Exit", "Esc", false, true)) AskToLeave();
            if (igMenuItem_Bool("Export map", "Ctrl+s", false, true)) ExportMap();
            
            igEndMenu();
        }
        if (igBeginMenu("Create", true)) {
            if (igMenuItem_Bool("Cube", "C", false, true)) CreateCube();
            igEndMenu();
        }

        igEndMenuBar();
    }
}

void DrawObjectListPanel(Object** selected_objects, ObjectCounter* num_selected_objects, ObjectCounter num_objects, Object* objects) {
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse;

    /* that's so stupid i literally can't even comprehend why there's no better alternative in the imgui itself */
    igPushStyleColor_U32(ImGuiCol_ResizeGrip, 0);

    bool show = (igBegin("Object list", NULL, windowFlags)); {
        for (ObjectCounter i = 0; i < num_objects; ++i) {
            bool active = false;
            for (ObjectCounter j = 0; j < *num_selected_objects; ++j) {
                active = (selected_objects[j] == &objects[i]);
                if (active) break;
            }
            if (igCheckbox(TextFormat("%d: %s", i, GetObjectTypeString(objects[i].type)), &active)) {
                selected_objects[*num_selected_objects] = &objects[i];
                (*num_selected_objects)++;
            }
        }
    } igEnd();
}

void DrawObjectContextMenu(Object** selected_objects, ObjectCounter* num_selected_objects) {
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse;

    bool show = (igBegin("Object context menu", NULL, windowFlags)); {
        
    } igEnd();
}

void DrawGUI(Object** selected_objects, ObjectCounter* num_selected_objects, ObjectCounter num_objects, Object* objects) {
    BeginGUIDraw(); {
        MakeDockSpace();
        DrawObjectListPanel(selected_objects, num_selected_objects, num_objects, objects);
        DrawObjectContextMenu(selected_objects, num_selected_objects);
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
