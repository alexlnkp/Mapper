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
#define CAMERA_FOV 100.0f

#define GROUND_TOTAL_LENGTH 1000.0f
#define GROUND_LENGTH GROUND_TOTAL_LENGTH / 2.0f
#define GROUND_MIN_BOUND (Vector3){-GROUND_LENGTH, 0.0f, -GROUND_LENGTH}
#define GROUND_MAX_BOUND (Vector3){GROUND_LENGTH, 0.0f, GROUND_LENGTH}

#define CUBES_RESERVED_MEMORY 10 /* how much cubes we can reserve in advance during reallocation */

#endif /* CONFIG_H */