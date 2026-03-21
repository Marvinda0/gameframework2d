#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "gfc_shape.h"  
#include "gfc_vector.h"

/*
@brief get the camera´s position in world space.
@return cameras x and y coordinates
*/
GFC_Vector2D camera_get_position();

/*
@brief get the offset to draw things relative to the camera
@return offset´s x and y
*/
GFC_Vector2D camera_get_offset();

/*
@brief set the camera position in a world space 
*/
void camera_set_position(GFC_Vector2D position);

/*
@brief 
*/
void camera_set_dimension(GFC_Vector2D dimensions);

/*
@brief snap camera into bounds
*/
void camera_apply_bounds();

/*
@brief set camera bounds
*/
void camera_set_bounds(GFC_Rect bounds);

/*
@brief 
*/
void camera_enable_binding(Bool bindCamera);

/*
@brief set camer size
*/
void camera_set_size(GFC_Vector2D size);

/*
@brief center camera on a target
*/
void camera_center_on(GFC_Vector2D targetPos);


#endif
