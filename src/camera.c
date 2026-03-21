#include "simple_logger.h"

#include "camera.h"

typedef struct
{
    GFC_Vector2D position;
    GFC_Vector2D size;
    GFC_Rect bounds; 
    Bool bindCamera;// if true keep the camera inbounds

}Camera;

static Camera _camera = {0};

void camera_set_bounds(GFC_Rect bounds)
{
    gfc_rect_copy(_camera.bounds,bounds);
}

void camera_enable_binding(Bool bindCamera)
{
 _camera.bindCamera = bindCamera;
}

GFC_Vector2D camera_get_position()
{
    return _camera.position;
}

GFC_Vector2D camera_get_offset()
{
    return gfc_vector2d(-_camera.position.x, - _camera.position.y);
}

void camera_set_position(GFC_Vector2D position)
{
    gfc_vector2d_copy(_camera.position,position);
    if(_camera.bindCamera)
    {
        camera_apply_bounds();
    }
}


void camera_set_size(GFC_Vector2D size)
{
    gfc_vector2d_copy(_camera.size,size);
}

void camera_apply_bounds()
{
    if((_camera.position.x + _camera.size.x) > (_camera.bounds.x + _camera.bounds.w))
    {
        _camera.position.x = (_camera.bounds.x + _camera.bounds.w) - _camera.size.x;
    }
    if((_camera.position.y + _camera.size.y) > (_camera.bounds.y + _camera.bounds.h))
    {
        _camera.position.y = (_camera.bounds.y + _camera.bounds.h) - _camera.size.y;
    }
    if(_camera.position.x < _camera.bounds.x) _camera.position.x = _camera.bounds.x;
    if(_camera.position.y < _camera.bounds.y) _camera.position.y = _camera.bounds.y;
}

void camera_center_on(GFC_Vector2D targetPos)
{
    GFC_Vector2D pos;
    pos.x = targetPos.x - (_camera.size.x*0.5);
    pos.y = targetPos.y - (_camera.size.y*0.5);
    camera_set_position(pos);

    if(_camera.bindCamera)
    {
        camera_apply_bounds();
    }

    
}
/*eof@eof*/
