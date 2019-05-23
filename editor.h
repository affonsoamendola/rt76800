#ifndef EDITOR_H
#define EDITOR_H

#include "vector2.h"

void move_cursor(VECTOR2 amount);
void move_view(VECTOR2 amount);

POINT2 convert_ws_to_editor_ss(VECTOR2 pos);
VECTOR2 convert_editor_ss_to_ws(POINT2 pos);

void EDITOR_Render();
void EDITOR_Handle_Input();
void EDITOR_Loop();

void EDITOR_Init();

void EDITOR_open_texture_select(int ceil_floor_wall);
void EDITOR_draw_texture_select();
void EDITOR_move_texture_select(POINT2 amount);
void EDITOR_apply_texture_select();
void EDITOR_cancel_texture_select();

void EDIT_MODE_Handle_Input();

#endif
