#ifndef   CONFIG_H
#define   CONFIG_H

#define WINDOW_WIDTH  1280
#define WINDOW_HEIGHT 720
#define WINDOW_TITLE "Map editor"

#define TARGET_FPS 60

#define CAMERA_MOUSE_SENSITIVITY 0.003f
#define CAMERA_MOVE_SPEED 0.6f
#define CAMERA_MAX_SPEED 1.5f
#define CAMERA_PAN_SPEED 0.08f
#define CAMERA_ZOOM_SPEED 0.2f
#define CAMERA_FOV 100.0f

#define GROUND_TOTAL_LENGTH 1000.0f
#define GROUND_LENGTH GROUND_TOTAL_LENGTH / 2.0f
#define GROUND_MIN_BOUND (Vector3){-GROUND_LENGTH, 0.0f, -GROUND_LENGTH}
#define GROUND_MAX_BOUND (Vector3){GROUND_LENGTH, 0.0f, GROUND_LENGTH}

#define OBJECTS_MEMORY_RESERVE 10 /* how many objects we can reserve in advance during reallocation */
#define SELECTED_OBJECTS_MEMORY_RESERVE 10 /* how many selected objects we can reserve in memory in advance during reallocation */
#define LABEL_DATA_MEMORY_RESERVE 10 /* how many chars we can reserve in memory in advance for label data */
#define LABEL_DATA_MAX_SIZE 2056
#define MAP_META_FIELD_MAX_SIZE 2056
#define MAP_META_FIELD_MEMORY_RESERVE 10 /* how many chars we can reserve in memory in advance for map META fields' data */

#define OBJECT_MOVE_SPEED 0.02f

#define XYZLINES_THICKNESS 2.0f
#define XLINE_COLOR (Color){.a=255, .r=249, .g=62,  .b=86}
#define ZLINE_COLOR (Color){.a=255, .r=4,   .g=146, .b=90}
#define YLINE_COLOR (Color){.a=255, .r=232, .g=193, .b=12}

#define OUTLINE_SHADER_LOCATION "shaders/outline.frag"
#define OUTLINE_WIDTH 4
#define OUTLINE_COLOR {1.0f, 0.79f, 0.29f, 1.0f} // Yellow-ish orange

#define DEFAULT_MAP_NAME "new_map"
#define DEFAULT_MAP_AUTHOR "unspecified"

#define DEFAULT_OBJECT_LABEL "NONE"

#endif /* CONFIG_H */