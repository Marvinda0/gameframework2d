#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "gfc_shape.h"
#include "gfc_vector.h"

typedef struct
{
    GFC_Rect bounds;

}Camera;

GFC_Vector2D camera_get_position;

GFC_Vector2D camera_get_offset;

void camera_set_camera_get_position(gfc_vector2d position);

void camera_set_dimension(gfc_vector2d dimensions);

#endif
