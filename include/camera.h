#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "gfc_shape.h"  
#include "gfc_vector.h"

/*
@brief get the camera´s position in world space.
@return cameras x and y coordinates
*/
GFC_Vector2D camera_get_position;

/*
@brief get the offset to draw things relative to the camera
@return offset´s x and y
*/
GFC_Vector2D camera_get_offset;

/*
@brief set the camera position in a world space 
*/
void camera_set_camera_get_position(gfc_vector2d position);

/*
@brief 
*/
void camera_set_dimension(gfc_vector2d dimensions);

#endif
