#include <assert.h>
#include <ctype.h>
#include <string.h>

#include "imgui_impl_raylib.h"
#include "rlcimgui.h"
#include <cimgui.h>

#include "config.h"
#include "ui.h"

#include "funkymacros.h"

ImFont* main_font;

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
    /* Code here is heavily based on this example:
        https://gist.github.com/moebiussurfing/8dbc7fef5964adcd29428943b78e45d2
    */
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
        ImGuiID dockspace_id = igGetID_Str("DockSpace");
        igDockSpace(dockspace_id, (ImVec2){0.0f, 0.0f}, dockspace_flags, ImGuiWindowClass_ImGuiWindowClass());

        /* Specifying default layout */
        if (igDockBuilderGetNode(dockspace_id) == NULL) {
            igDockBuilderRemoveNode(dockspace_id); // clear any previous layout
            igDockBuilderAddNode(dockspace_id, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
            igDockBuilderSetNodeSize(dockspace_id, viewport->Size);

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
            if (igMenuItem_Bool("Export map", "Ctrl+S", false, true)) ExportMap();
            
            igEndMenu();
        }
        if (igBeginMenu("Create", true)) {
            if (igMenuItem_Bool("Cube", "Shift+C", false, true)) CreateCube();
            if (igMenuItem_Bool("Sphere", "Shift+P", false, true)) CreateSphere();

            igEndMenu();
        }

        igEndMenuBar();
    }
}

char* TrimWhiteSpaces(char *str) {
    /* function from stackoverflow: https://stackoverflow.com/a/122721/25145880
    Author: Adam Rosenfield
    */
    char *end;

    // Trim leading space
    while(isspace((unsigned char)*str)) str++;

    if(*str == 0)  // All spaces?
        return str;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;

    // Write new null terminator character
    end[1] = '\0';

    return str;
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
                ResizeObjectSelection();
                selected_objects[*num_selected_objects] = &objects[i];
                (*num_selected_objects)++;
            }
        }
    } igEnd();
}

void DrawObjectContextMenu(Object** selected_objects, ObjectCounter* num_selected_objects) {
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse;

    bool show = (igBegin("Object context menu", NULL, windowFlags)); {
        if (*num_selected_objects > 0) {
            if (*num_selected_objects < 2) {
                ObjectCounter cur_obj_idx = *num_selected_objects - 1;

                float new_x = selected_objects[cur_obj_idx]->pos.x;
                float new_y = selected_objects[cur_obj_idx]->pos.y;
                float new_z = selected_objects[cur_obj_idx]->pos.z;

                igLabelText("", "%s", "Position");

                if (igDragFloat("X", &new_x, 0.01f, -GROUND_TOTAL_LENGTH, GROUND_TOTAL_LENGTH, "%f", 0))
                    selected_objects[cur_obj_idx]->pos.x = new_x;
                
                if (igDragFloat("Y", &new_y, 0.01f, -GROUND_TOTAL_LENGTH, GROUND_TOTAL_LENGTH, "%f", 0))
                    selected_objects[cur_obj_idx]->pos.y = new_y;
                
                if (igDragFloat("Z", &new_z, 0.01f, -GROUND_TOTAL_LENGTH, GROUND_TOTAL_LENGTH, "%f", 0))
                    selected_objects[cur_obj_idx]->pos.z = new_z;
                
                static char buf[LABEL_DATA_MAX_SIZE];

                if (strcmp(buf, selected_objects[cur_obj_idx]->label) != 0)
                    strncpy(buf, selected_objects[cur_obj_idx]->label, LABEL_DATA_MAX_SIZE);

                if (igInputText("Label", buf, LABEL_DATA_MAX_SIZE, 0, 0, NULL)) {
                    unsigned new_len = strlen(buf) + 1;
                    if (new_len % LABEL_DATA_MEMORY_RESERVE == 0) {
                        new_len += LABEL_DATA_MEMORY_RESERVE;
                        REALLOC(selected_objects[cur_obj_idx]->label, sizeof(char) * new_len);
                        TraceLog(LOG_DEBUG, "Realloc'd for label of selected_objects[cur_obj_idx]");
                    }

                    strncpy(selected_objects[cur_obj_idx]->label, buf, new_len);
                }

                int current_obj_type = selected_objects[cur_obj_idx]->type;
                if (igCombo_Str_arr("Type", &current_obj_type, obj_types, sizeof(obj_types) / sizeof(obj_types[0]), 0)) {
                    selected_objects[cur_obj_idx]->type = current_obj_type;
                }

                igLabelText("", "%s", "Type-specific vars");
                
                switch (selected_objects[cur_obj_idx]->type) {
                case OT_Cube: {
                    float new_dim_x = selected_objects[cur_obj_idx]->data.Cube.dim.x;
                    float new_dim_y = selected_objects[cur_obj_idx]->data.Cube.dim.y;
                    float new_dim_z = selected_objects[cur_obj_idx]->data.Cube.dim.z;

                    if (igDragFloat("Width", &new_dim_x, 0.01f, -GROUND_TOTAL_LENGTH, GROUND_TOTAL_LENGTH, "%f", 0))
                        selected_objects[cur_obj_idx]->data.Cube.dim.x = new_dim_x;
                    
                    if (igDragFloat("Height", &new_dim_y, 0.01f, -GROUND_TOTAL_LENGTH, GROUND_TOTAL_LENGTH, "%f", 0))
                        selected_objects[cur_obj_idx]->data.Cube.dim.y = new_dim_y;
                    
                    if (igDragFloat("Length", &new_dim_z, 0.01f, -GROUND_TOTAL_LENGTH, GROUND_TOTAL_LENGTH, "%f", 0))
                        selected_objects[cur_obj_idx]->data.Cube.dim.z = new_dim_z;

                } break;

                case OT_Sphere: {
                    float new_radius = selected_objects[cur_obj_idx]->data.Sphere.radius;
                    if (igDragFloat("Radius", &new_radius, 0.01f, 0.0f, GROUND_TOTAL_LENGTH / 2, "%f", 0))
                        selected_objects[cur_obj_idx]->data.Sphere.radius = new_radius;

                } break;

                default: {} break;

                }

            } else {

            }
        }
    } igEnd();
}

void DrawGUI(Object** selected_objects, ObjectCounter* num_selected_objects, ObjectCounter num_objects, Object* objects) {
    BeginGUIDraw(); {
        igPushFont(main_font);
        MakeDockSpace();
        DrawObjectListPanel(selected_objects, num_selected_objects, num_objects, objects);
        DrawObjectContextMenu(selected_objects, num_selected_objects);
        DrawContextMenu();
        igPopFont();
    } EndGUIDraw();
}

void InitGUI(void) {
    rligSetup(true);

    igCreateContext(NULL);
    ImGuiIO* ioptr = igGetIO();

#ifdef IMGUI_HAS_DOCK
    ioptr->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
#endif

    ImGui_ImplRaylib_Init();
    ImFontAtlas_AddFontDefault(ioptr->Fonts, NULL);
    ImFontConfig* font_cfg = ImFontConfig_ImFontConfig();
    font_cfg->OversampleH = 1;
    font_cfg->OversampleV = 1;
    font_cfg->PixelSnapH = true;

    font_cfg->SizePixels = 14.5f;
    font_cfg->EllipsisChar = (ImWchar)0x0085;

    const ImWchar* glyph_ranges = font_cfg->GlyphRanges != NULL ? font_cfg->GlyphRanges : ImFontAtlas_GetGlyphRangesDefault(ioptr->Fonts);

    main_font = ImFontAtlas_AddFontFromFileTTF(ioptr->Fonts, "res/0xProtoNerdFont-Regular.ttf", font_cfg->SizePixels, font_cfg, glyph_ranges);

    rligSetupFontAwesome();

    /* required to be called to cache the font texture with raylib */
    ImGui_ImplRaylib_BuildFontAtlas();
}

void DeInitGUI(void) { ImGui_ImplRaylib_Shutdown(); igDestroyContext(NULL); }

bool IsHoveringOverAnyGUIElement(void) {
    return igIsWindowHovered(ImGuiHoveredFlags_AnyWindow);
}

void LockGUI(void) {  }
void UnlockGUI(void) {  }
