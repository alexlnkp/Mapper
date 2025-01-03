#include <assert.h>
#include <string.h>

#include "editor.h"
#include "imgui_impl_raylib.h"
#include "rlcimgui.h"
#include <cimgui.h>

#include <ImGuiFileDialog.h>

#include "config.h"
#include "ui.h"

#include "funkymacros.h"

void StringEdit(char* buf, char** dest, unsigned mem_reserve) {
    unsigned new_len_dest = strlen(buf) + 1;
    if (new_len_dest % mem_reserve == 0) {
        new_len_dest += mem_reserve;
        REALLOC(*dest, sizeof(char) * new_len_dest);
        TraceLog(LOG_DEBUG, "Realloc'd for '%s'", dest);
    }

    strncpy(*dest, buf, new_len_dest);
}

void SetupGUIStyle(void) {
	/* Fork of Future Dark style from ImThemes */
	ImGuiStyle* style = igGetStyle();

	style->Alpha = 1.0f;
	style->DisabledAlpha = 0.6000000238418579f;
	style->WindowPadding = (ImVec2){12.0f, 12.0f};
	style->WindowRounding = 0.0f;
	style->WindowBorderSize = 1.0f;
	style->WindowMinSize = (ImVec2){20.0f, 20.0f};
	style->WindowTitleAlign = (ImVec2){0.5f, 0.5f};
	style->WindowMenuButtonPosition = ImGuiDir_None;
	style->ChildRounding = 0.0f;
	style->ChildBorderSize = 1.0f;
	style->PopupRounding = 0.0f;
	style->PopupBorderSize = 1.0f;
	style->FramePadding = (ImVec2){5.300000190734863f, 3.599999904632568f};
	style->FrameRounding = 0.0f;
	style->FrameBorderSize = 0.0f;
	style->ItemSpacing = (ImVec2){5.0f, 5.0f};
	style->ItemInnerSpacing = (ImVec2){6.0f, 3.0f};
	style->CellPadding = (ImVec2){6.0f, 5.0f};
	style->IndentSpacing = 10.0f;
	style->ColumnsMinSpacing = 5.0f;
	style->ScrollbarSize = 12.0f;
	style->ScrollbarRounding = 0.0f;
	style->GrabMinSize = 10.0f;
	style->GrabRounding = 2.900000095367432f;
	style->TabRounding = 0.0f;
	style->TabBorderSize = 0.0f;
	style->TabMinWidthForCloseButton = 0.0f;
	style->ColorButtonPosition = ImGuiDir_Right;
	style->ButtonTextAlign = (ImVec2){0.5f, 0.5f};
	style->SelectableTextAlign = (ImVec2){0.0f, 0.0f};

	style->Colors[ImGuiCol_Text] = (ImVec4){1.0f, 1.0f, 1.0f, 1.0f};
	style->Colors[ImGuiCol_TextDisabled] = (ImVec4){0.2745098173618317f, 0.3176470696926117f, 0.4509803950786591f, 1.0f};
	style->Colors[ImGuiCol_WindowBg] = (ImVec4){0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f};
	style->Colors[ImGuiCol_ChildBg] = (ImVec4){0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f};
	style->Colors[ImGuiCol_PopupBg] = (ImVec4){0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f};
	style->Colors[ImGuiCol_Border] = (ImVec4){0.1568627506494522f, 0.168627455830574f, 0.1921568661928177f, 1.0f};
	style->Colors[ImGuiCol_BorderShadow] = (ImVec4){0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f};
	style->Colors[ImGuiCol_FrameBg] = (ImVec4){0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f};
	style->Colors[ImGuiCol_FrameBgHovered] = (ImVec4){0.1568627506494522f, 0.168627455830574f, 0.1921568661928177f, 1.0f};
	style->Colors[ImGuiCol_FrameBgActive] = (ImVec4){0.2352941185235977f, 0.2156862765550613f, 0.5960784554481506f, 1.0f};
	style->Colors[ImGuiCol_TitleBg] = (ImVec4){0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f};
	style->Colors[ImGuiCol_TitleBgActive] = (ImVec4){0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f};
	style->Colors[ImGuiCol_TitleBgCollapsed] = (ImVec4){0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f};
	style->Colors[ImGuiCol_MenuBarBg] = (ImVec4){0.09803921729326248f, 0.105882354080677f, 0.1215686276555061f, 1.0f};
	style->Colors[ImGuiCol_ScrollbarBg] = (ImVec4){0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f};
	style->Colors[ImGuiCol_ScrollbarGrab] = (ImVec4){0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f};
	style->Colors[ImGuiCol_ScrollbarGrabHovered] = (ImVec4){0.1568627506494522f, 0.168627455830574f, 0.1921568661928177f, 1.0f};
	style->Colors[ImGuiCol_ScrollbarGrabActive] = (ImVec4){0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f};
	style->Colors[ImGuiCol_CheckMark] = (ImVec4){0.4980392158031464f, 0.5137255191802979f, 1.0f, 1.0f};
	style->Colors[ImGuiCol_SliderGrab] = (ImVec4){0.4980392158031464f, 0.5137255191802979f, 1.0f, 1.0f};
	style->Colors[ImGuiCol_SliderGrabActive] = (ImVec4){0.5372549295425415f, 0.5529412031173706f, 1.0f, 1.0f};
	style->Colors[ImGuiCol_Button] = (ImVec4){0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f};
	style->Colors[ImGuiCol_ButtonHovered] = (ImVec4){0.196078434586525f, 0.1764705926179886f, 0.5450980663299561f, 1.0f};
	style->Colors[ImGuiCol_ButtonActive] = (ImVec4){0.2352941185235977f, 0.2156862765550613f, 0.5960784554481506f, 1.0f};
	style->Colors[ImGuiCol_Header] = (ImVec4){0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f};
	style->Colors[ImGuiCol_HeaderHovered] = (ImVec4){0.196078434586525f, 0.1764705926179886f, 0.5450980663299561f, 1.0f};
	style->Colors[ImGuiCol_HeaderActive] = (ImVec4){0.2352941185235977f, 0.2156862765550613f, 0.5960784554481506f, 1.0f};
	style->Colors[ImGuiCol_Separator] = (ImVec4){0.1568627506494522f, 0.1843137294054031f, 0.250980406999588f, 1.0f};
	style->Colors[ImGuiCol_SeparatorHovered] = (ImVec4){0.1568627506494522f, 0.1843137294054031f, 0.250980406999588f, 1.0f};
	style->Colors[ImGuiCol_SeparatorActive] = (ImVec4){0.1568627506494522f, 0.1843137294054031f, 0.250980406999588f, 1.0f};
	style->Colors[ImGuiCol_ResizeGrip] = (ImVec4){0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f};
	style->Colors[ImGuiCol_ResizeGripHovered] = (ImVec4){0.196078434586525f, 0.1764705926179886f, 0.5450980663299561f, 1.0f};
	style->Colors[ImGuiCol_ResizeGripActive] = (ImVec4){0.2352941185235977f, 0.2156862765550613f, 0.5960784554481506f, 1.0f};
	style->Colors[ImGuiCol_Tab] = (ImVec4){0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f};
	style->Colors[ImGuiCol_TabHovered] = (ImVec4){0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f};
	style->Colors[ImGuiCol_TabSelected] = (ImVec4){0.09803921729326248f, 0.105882354080677f, 0.1215686276555061f, 1.0f};
	style->Colors[ImGuiCol_TabDimmed] = (ImVec4){0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f};
	style->Colors[ImGuiCol_TabDimmedSelected] = (ImVec4){0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f};
	style->Colors[ImGuiCol_PlotLines] = (ImVec4){0.5215686559677124f, 0.6000000238418579f, 0.7019608020782471f, 1.0f};
	style->Colors[ImGuiCol_PlotLinesHovered] = (ImVec4){0.03921568766236305f, 0.9803921580314636f, 0.9803921580314636f, 1.0f};
	style->Colors[ImGuiCol_PlotHistogram] = (ImVec4){1.0f, 0.2901960909366608f, 0.5960784554481506f, 1.0f};
	style->Colors[ImGuiCol_PlotHistogramHovered] = (ImVec4){0.9960784316062927f, 0.4745098054409027f, 0.6980392336845398f, 1.0f};
	style->Colors[ImGuiCol_TableHeaderBg] = (ImVec4){0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f};
	style->Colors[ImGuiCol_TableBorderStrong] = (ImVec4){0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f};
	style->Colors[ImGuiCol_TableBorderLight] = (ImVec4){0.0f, 0.0f, 0.0f, 1.0f};
	style->Colors[ImGuiCol_TableRowBg] = (ImVec4){0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f};
	style->Colors[ImGuiCol_TableRowBgAlt] = (ImVec4){0.09803921729326248f, 0.105882354080677f, 0.1215686276555061f, 1.0f};
	style->Colors[ImGuiCol_TextSelectedBg] = (ImVec4){0.2352941185235977f, 0.2156862765550613f, 0.5960784554481506f, 1.0f};
	style->Colors[ImGuiCol_DragDropTarget] = (ImVec4){0.4980392158031464f, 0.5137255191802979f, 1.0f, 1.0f};
	style->Colors[ImGuiCol_NavHighlight] = (ImVec4){0.4980392158031464f, 0.5137255191802979f, 1.0f, 1.0f};
	style->Colors[ImGuiCol_NavWindowingHighlight] = (ImVec4){0.4980392158031464f, 0.5137255191802979f, 1.0f, 1.0f};
	style->Colors[ImGuiCol_NavWindowingDimBg] = (ImVec4){0.196078434586525f, 0.1764705926179886f, 0.5450980663299561f, 0.501960813999176f};
	style->Colors[ImGuiCol_ModalWindowDimBg] = (ImVec4){0.196078434586525f, 0.1764705926179886f, 0.5450980663299561f, 0.501960813999176f};
}

Color Float4ToRLColor(float* ic) {
    ImU32 cs = igColorConvertFloat4ToU32((ImVec4){ic[0], ic[1], ic[2], ic[3]});

    Color cl = {
        .r=cs & 0xFF,
        .g=(cs >> 8)  & 0xFF,
        .b=(cs >> 16) & 0xFF,
        .a=(cs >> 24) & 0xFF
    };

    return cl;
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

void DrawMenuBar(AppContext** app_ctx) {
    if (igBeginMenuBar()) {
        if (igBeginMenu("File", true)) {
            if (igMenuItem_Bool("Open", "", false, true)) {
                (*app_ctx)->gui_ctx->ig_fd = IGFD_Create();
                struct IGFD_FileDialog_Config config = IGFD_FileDialog_Config_Get();

                config.flags = ImGuiFileDialogFlags_DontShowHiddenFiles | ImGuiFileDialogFlags_DisableCreateDirectoryButton;
                IGFD_OpenDialog((*app_ctx)->gui_ctx->ig_fd, "ChooseFileDlgKey", "Open file", "((.*))", config);
            }
            if (igMenuItem_Bool("Edit", "", false, true)) (*app_ctx)->gui_ctx->show_map_meta_edit = true;
            if (igMenuItem_Bool("Export map", "Ctrl+S", false, true)) ExportMap((*app_ctx)->m_ctx);
            if (igMenuItem_Bool("Exit", "Esc", false, true)) AskToLeave(app_ctx);

            igEndMenu();
        }
        if (igBeginMenu("Create", true)) {
            if (igMenuItem_Bool("Cube", "Shift+C", false, true)) CreateCube((*app_ctx)->m_ctx, (*app_ctx)->c_ctx);
            if (igMenuItem_Bool("Sphere", "Shift+P", false, true)) CreateSphere((*app_ctx)->m_ctx, (*app_ctx)->c_ctx);

            igEndMenu();
        }

        igEndMenuBar();
    }
}

void DrawMapMetaEditor(GUIContext* gui_ctx, Map* map) {
    if (gui_ctx->show_map_meta_edit) {
        static char buf_name[MAP_META_FIELD_MAX_SIZE];
        if (strcmp(buf_name, map->meta.name) != 0)
            strncpy(buf_name, map->meta.name, MAP_META_FIELD_MAX_SIZE);

        static char buf_author[MAP_META_FIELD_MAX_SIZE];
        if (strcmp(buf_author, map->meta.author) != 0)
            strncpy(buf_author, map->meta.author, MAP_META_FIELD_MAX_SIZE);

        ImGuiWindowFlags wf = ImGuiWindowFlags_Popup | ImGuiWindowFlags_AlwaysAutoResize;

        bool show = (igBegin("Edit map metadata", &gui_ctx->show_map_meta_edit, wf)); {
            if (igInputText("Map name", buf_name, MAP_META_FIELD_MAX_SIZE, 0, 0, NULL)) {
                StringEdit(buf_name, &map->meta.name, MAP_META_FIELD_MEMORY_RESERVE);
            }

            if (igInputText("Map author", buf_author, MAP_META_FIELD_MAX_SIZE, 0, 0, NULL)) {
                StringEdit(buf_author, &map->meta.author, MAP_META_FIELD_MEMORY_RESERVE);
            }

        } igEnd();

    }
}

void DrawFileDialog(MapContext* m_ctx, GUIContext* gui_ctx) {
    if (gui_ctx->ig_fd == NULL) return;
    if (IGFD_DisplayDialog(gui_ctx->ig_fd, "ChooseFileDlgKey", 0, (ImVec2){200.0f, 200.0f}, (ImVec2){900.0f, 900.0f})) {
        if (IGFD_IsOk(gui_ctx->ig_fd)) {
            char* file_path_name = IGFD_GetFilePathName(gui_ctx->ig_fd, IGFD_ResultMode_KeepInputFile);

            if (file_path_name != NULL) {
                TraceLog(LOG_DEBUG, "Picked map file in DrawFileDialog() ; File: %s", file_path_name);
                ImportMap(m_ctx, file_path_name);
                FREE(file_path_name);
            }
        }

        IGFD_CloseDialog(gui_ctx->ig_fd);
    }
}

void DrawObjectListPanel(MapContext* m_ctx) {
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse;

    /* that's so stupid i literally can't even comprehend why there's no better alternative in the imgui itself */
    igPushStyleColor_U32(ImGuiCol_ResizeGrip, 0);

    bool show = (igBegin("Objects", NULL, windowFlags)); {
        for (ObjectCounter i = 0; i < m_ctx->map.num_objects; ++i) {
            bool active = false;
            ObjectCounter cur_selected_obj;
            for (cur_selected_obj = 0; cur_selected_obj <= m_ctx->num_selected_objects && m_ctx->num_selected_objects != (ObjectCounter)-1; ++cur_selected_obj) {
                active = (m_ctx->selected_objects[cur_selected_obj] == &m_ctx->map.objects[i]);
                if (active) break;
            }
            if (igCheckbox(TextFormat("%d: %s", i, obj_types[m_ctx->map.objects[i].type]), &active)) {
                if (!active) { DeSelectObjectAtIndex(m_ctx, cur_selected_obj); }
                else { SelectObjectAtIndex(m_ctx, i); }
            }
        }
    } igEnd();
}

void DrawObjectContextMenu(MapContext* m_ctx) {
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse;

    bool show = (igBegin("Properties", NULL, windowFlags)); {
        if (m_ctx->num_selected_objects != (ObjectCounter)-1) {
            if (m_ctx->num_selected_objects < 1) {
                ObjectCounter cur_obj_idx = m_ctx->num_selected_objects;

                float new_x = m_ctx->selected_objects[cur_obj_idx]->pos.x;
                float new_y = m_ctx->selected_objects[cur_obj_idx]->pos.y;
                float new_z = m_ctx->selected_objects[cur_obj_idx]->pos.z;

                igLabelText("", "%s", "Position");

                if (igDragFloat("X", &new_x, 0.01f, -GROUND_TOTAL_LENGTH, GROUND_TOTAL_LENGTH, "%f", 0))
                    m_ctx->selected_objects[cur_obj_idx]->pos.x = new_x;

                if (igDragFloat("Y", &new_y, 0.01f, -GROUND_TOTAL_LENGTH, GROUND_TOTAL_LENGTH, "%f", 0))
                    m_ctx->selected_objects[cur_obj_idx]->pos.y = new_y;

                if (igDragFloat("Z", &new_z, 0.01f, -GROUND_TOTAL_LENGTH, GROUND_TOTAL_LENGTH, "%f", 0))
                    m_ctx->selected_objects[cur_obj_idx]->pos.z = new_z;

                Color cur_obj_col = m_ctx->selected_objects[cur_obj_idx]->col;

                ImVec4 ig_color = {0};
                ImU32 color_mould = cur_obj_col.r | (cur_obj_col.g << 8) | (cur_obj_col.b << 16) | (cur_obj_col.a << 24);
                igColorConvertU32ToFloat4(&ig_color, color_mould);

                float col[4] = {ig_color.x, ig_color.y, ig_color.z, ig_color.w};

                if (igColorEdit4("Color", col, ImGuiColorEditFlags_Uint8)) {
                    m_ctx->selected_objects[cur_obj_idx]->col = Float4ToRLColor(col);
                }

                static char buf[LABEL_DATA_MAX_SIZE];

                if (strcmp(buf, m_ctx->selected_objects[cur_obj_idx]->label) != 0)
                    strncpy(buf, m_ctx->selected_objects[cur_obj_idx]->label, LABEL_DATA_MAX_SIZE);

                if (igInputText("Label", buf, LABEL_DATA_MAX_SIZE, 0, 0, NULL)) {
                    StringEdit(buf, &m_ctx->selected_objects[cur_obj_idx]->label, LABEL_DATA_MEMORY_RESERVE);
                }

                int current_obj_type = m_ctx->selected_objects[cur_obj_idx]->type;
                if (igCombo_Str_arr("Type", &current_obj_type, obj_types, sizeof(obj_types) / sizeof(obj_types[0]), 0)) {
                    m_ctx->selected_objects[cur_obj_idx]->type = current_obj_type;
                }

                igLabelText("", "%s", "Type-specific vars");

                switch (m_ctx->selected_objects[cur_obj_idx]->type) {
                case OT_Cube: {
                    float new_dim_x = m_ctx->selected_objects[cur_obj_idx]->data.Cube.dim.x;
                    float new_dim_y = m_ctx->selected_objects[cur_obj_idx]->data.Cube.dim.y;
                    float new_dim_z = m_ctx->selected_objects[cur_obj_idx]->data.Cube.dim.z;

                    if (igDragFloat("Width", &new_dim_x, 0.01f, -GROUND_TOTAL_LENGTH, GROUND_TOTAL_LENGTH, "%f", 0))
                        m_ctx->selected_objects[cur_obj_idx]->data.Cube.dim.x = new_dim_x;

                    if (igDragFloat("Height", &new_dim_y, 0.01f, -GROUND_TOTAL_LENGTH, GROUND_TOTAL_LENGTH, "%f", 0))
                        m_ctx->selected_objects[cur_obj_idx]->data.Cube.dim.y = new_dim_y;

                    if (igDragFloat("Length", &new_dim_z, 0.01f, -GROUND_TOTAL_LENGTH, GROUND_TOTAL_LENGTH, "%f", 0))
                        m_ctx->selected_objects[cur_obj_idx]->data.Cube.dim.z = new_dim_z;

                } break;

                case OT_Sphere: {
                    float new_radius = m_ctx->selected_objects[cur_obj_idx]->data.Sphere.radius;
                    if (igDragFloat("Radius", &new_radius, 0.01f, 0.0f, GROUND_TOTAL_LENGTH / 2, "%f", 0))
                        m_ctx->selected_objects[cur_obj_idx]->data.Sphere.radius = new_radius;

                } break;

                default: {} break;

                }

            } else {
                /* TODO: handle object context menu for multiple selected objects */
            }
        }
    } igEnd();
}

void DrawGUI(AppContext* app_ctx) {
    BeginGUIDraw(); {
        igPushFont(app_ctx->gui_ctx->main_font);
        MakeDockSpace();
        DrawObjectListPanel(app_ctx->m_ctx);
        DrawObjectContextMenu(app_ctx->m_ctx);
        DrawMapMetaEditor(app_ctx->gui_ctx, &app_ctx->m_ctx->map);
        DrawMenuBar(&app_ctx);
        DrawFileDialog(app_ctx->m_ctx, app_ctx->gui_ctx);
        igPopFont();
    } EndGUIDraw();
}

GUIContext* InitGUI(void) {
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

    GUIContext* gui_ctx = MALLOCFUNC(sizeof(GUIContext));
    gui_ctx->main_font = ImFontAtlas_AddFontFromFileTTF(ioptr->Fonts, "res/0xProtoNerdFont-Regular.ttf", font_cfg->SizePixels, font_cfg, glyph_ranges);

    rligSetupFontAwesome();

    SetupGUIStyle();

    gui_ctx->show_map_meta_edit = false;

    /* required to be called to cache the font texture with raylib */
    ImGui_ImplRaylib_BuildFontAtlas();

    return gui_ctx;
}

void DeInitGUI(AppContext* app_ctx) {
    if (app_ctx->gui_ctx != NULL) FREE(app_ctx->gui_ctx);
    ImGui_ImplRaylib_Shutdown();
    igDestroyContext(NULL);
}

/* checks if user is interacting with gui in any way */
bool IsInteractingWithGUI(void) {
    return igIsWindowHovered(ImGuiHoveredFlags_AnyWindow) || igIsAnyItemActive();
}

void LockGUI(void) {  }
void UnlockGUI(void) {  }
