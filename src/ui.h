#ifndef   UI_H
#define   UI_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ImFont ImFont;
typedef struct ImGuiFileDialog ImGuiFileDialog;
typedef struct AppContext AppContext;

typedef struct {
    ImFont* main_font;
    ImGuiFileDialog* ig_fd;

    bool show_map_meta_edit;
} GUIContext;

GUIContext* InitGUI(void);
void DeInitGUI(void);

void DrawGUI(AppContext* app_ctx);

bool IsInteractingWithGUI(void);

void UnlockGUI(void);
void LockGUI(void);

#ifdef __cplusplus
}
#endif

#endif /* UI_H */
