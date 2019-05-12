
#include <stdio.h>
#include <stdlib.h>

#include <math.h>

#include "editor.h"

#include "vector2.h"
#include "world.h"
#include "gfx.h"
#include "engine.h"

#define DEFAULT_FLOOR_HEIGHT 0.0f
#define DEFAULT_CEIL_HEIGHT 1.0f

VECTOR2 editor_center;
VECTOR2 editor_cursor;

int closest_vector_index;
int closest_edge_index;
int closest_sector_index;
VECTOR2 closest_edge_projection;

VECTOR2 closest_vector;
EDGE * closest_edge;

float closest_vector_distance;
float closest_edge_distance;

float grid_size = 0.1;

float editor_zoom = 50.;

float cursor_speed = 1.f;
#define crawl_cursor_speed 1.5f;
#define walk_cursor_speed 3.f;
#define run_cursor_speed 6.f;

#define cursor_color GFX_Color(255, 0, 0)

extern LEVEL loaded_level; 
extern SDL_Surface * screen;

extern float current_fps;
extern bool show_fps;

extern bool edit_mode;
extern bool game_mode;

int grabbed = 0;
int snap_to_grid = 0;
int occupied = 0;
int show_info = 0;

int grabbed_vector_index;
VECTOR2 grabbed_vector_start;
VECTOR2 grabbed_vector_location;

char buffer[128];

VECTOR2 top_left_border;
VECTOR2 bottom_right_border;

int new_vector_start_index;
VECTOR2 new_vector_start;

int first_vector_index;
VECTOR2 first_vector;

SECTOR * creating_sector;

int new_sector_size = 0;

int drawing_sector = 0;

void move_cursor(VECTOR2 amount)
{
	editor_cursor = sum_v2(editor_cursor, amount);

	if(	editor_cursor.x < top_left_border.x || editor_cursor.y >= top_left_border.y ||
		editor_cursor.x >= bottom_right_border.x || editor_cursor.y < bottom_right_border.y)
	{
		move_view(amount);
	}
}

void clip_cursor()
{
	if(editor_cursor.x < top_left_border.x)
		editor_cursor.x = top_left_border.x;

	if(editor_cursor.x > bottom_right_border.x)
		editor_cursor.x = bottom_right_border.x;

	if(editor_cursor.y > top_left_border.y)
		editor_cursor.y = top_left_border.y;

	if(editor_cursor.y < bottom_right_border.y)
		editor_cursor.y = bottom_right_border.y;
}

void update_borders()
{
	top_left_border = convert_editor_ss_to_ws(point2(0, 0));
	bottom_right_border = convert_editor_ss_to_ws(point2(SCREEN_RES_X, SCREEN_RES_Y));
}

void move_view(VECTOR2 amount)
{
	editor_center = sum_v2(editor_center, amount);
}

VECTOR2 get_closest_grid(VECTOR2 pos)
{
	int x;
	int y;

	VECTOR2 closest_grid;

	x = round(pos.x/grid_size);
	y = round(pos.y/grid_size);

	closest_grid.x = (float)x * grid_size;
	closest_grid.y = (float)y * grid_size;

	return closest_grid;
}

void set_grid_size(float size)
{
	grid_size = size;
}

void set_zoom(float zoom)
{
	editor_zoom = zoom;
}

POINT2 convert_ws_to_editor_ss(VECTOR2 pos)
{
	POINT2 editor_ss;

	editor_ss.x = (pos.x - editor_center.x)*editor_zoom + SCREEN_RES_X/2;
	editor_ss.y = SCREEN_RES_Y/2 - (pos.y - editor_center.y)*editor_zoom;

	return editor_ss;
}

VECTOR2 convert_editor_ss_to_ws(POINT2 pos)
{
	VECTOR2 ws;

	ws.x = (pos.x - SCREEN_RES_X/2)/editor_zoom + editor_center.x;
	ws.y = -((pos.y - SCREEN_RES_Y/2)/editor_zoom - editor_center.y);

	return ws;
}

void save_sector()
{

	WORLD_add_sector_to_level(creating_sector);
}

void new_sector()
{
	drawing_sector = 1;
	occupied = 1;

	creating_sector = malloc(sizeof(SECTOR));
	creating_sector->sector_id = loaded_level.s_num;

	creating_sector->e_num = 0;

	creating_sector->floor_height = DEFAULT_FLOOR_HEIGHT;
	creating_sector->ceiling_height = DEFAULT_CEIL_HEIGHT;

	creating_sector->text_param_ceil = DEFAULT_TEXTURE_PARAM;
	creating_sector->text_param_floor = DEFAULT_TEXTURE_PARAM;

	creating_sector->e = NULL;

	if(closest_vector_distance >= 0.1f)
	{
		if(snap_to_grid)
			new_vector_start = get_closest_grid(editor_cursor);
		else
			new_vector_start = editor_cursor;

		new_vector_start_index = WORLD_add_vertex(new_vector_start);

		new_sector_size += 1;
	}
	else
	{
		new_vector_start = closest_vector;
		new_vector_start_index = closest_vector_index;
	}

	first_vector = new_vector_start;
	first_vector_index = new_vector_start_index;
}

void new_edge()
{
	VECTOR2 new_vector_end;
	int new_vector_end_index;

	if(closest_vector_distance >= 0.1f)
	{
		if(snap_to_grid)
			new_vector_end = get_closest_grid(editor_cursor);
		else
			new_vector_end = editor_cursor;

		new_vector_end_index = WORLD_add_vertex(new_vector_end);

		WORLD_add_edge_to_sector(creating_sector, new_vector_start_index, new_vector_end_index);

		new_vector_start = new_vector_end;
		new_vector_start_index = new_vector_end_index;

		new_sector_size += 1;
	}
	else
	{
		new_vector_end = closest_vector;
		new_vector_end_index = closest_vector_index;

		WORLD_add_edge_to_sector(creating_sector, new_vector_start_index, new_vector_end_index);

		if(first_vector_index == closest_vector_index)
		{
			drawing_sector = 0;
			occupied = 0;
			new_sector_size = 0;
			save_sector();
			return;
		}

		new_vector_start = new_vector_end;
		new_vector_start_index = new_vector_end_index;
	}
}

void delete_vertex()
{

	WORLD_delete_vertex_at(closest_vector_index);
}

void draw_cursor()
{
	POINT2 cursor_ss;

	cursor_ss = convert_ws_to_editor_ss(editor_cursor);

	GFX_set_pixel(screen, cursor_ss.x, cursor_ss.y + 3, GFX_Map_Color(cursor_color), 0);
	GFX_set_pixel(screen, cursor_ss.x, cursor_ss.y + 2, GFX_Map_Color(cursor_color), 0);
	GFX_set_pixel(screen, cursor_ss.x, cursor_ss.y - 2, GFX_Map_Color(cursor_color), 0);
	GFX_set_pixel(screen, cursor_ss.x, cursor_ss.y - 3, GFX_Map_Color(cursor_color), 0);

	GFX_set_pixel(screen, cursor_ss.x + 3, cursor_ss.y, GFX_Map_Color(cursor_color), 0);
	GFX_set_pixel(screen, cursor_ss.x + 2, cursor_ss.y, GFX_Map_Color(cursor_color), 0);
	GFX_set_pixel(screen, cursor_ss.x - 2, cursor_ss.y, GFX_Map_Color(cursor_color), 0);
	GFX_set_pixel(screen, cursor_ss.x - 3, cursor_ss.y, GFX_Map_Color(cursor_color), 0);
}

void draw_editor_map()
{
	int last_v;
	unsigned int color;

	VECTOR2 last_v_vector;
	VECTOR2 current_v_vector;

	POINT2 current_line_start;
	POINT2 current_line_end;

	for(int s = 0; s < loaded_level.s_num; s++)
	{
		for(int e = 0; e < (loaded_level.sectors+s)->e_num; e++)
		{
			EDGE current_edge = *((loaded_level.sectors+s)->e + e);
			
			last_v_vector = *(loaded_level.vertexes + current_edge.v_start);
			current_v_vector = *(loaded_level.vertexes + current_edge.v_end);

			current_line_start = convert_ws_to_editor_ss(last_v_vector);
			current_line_end = convert_ws_to_editor_ss(current_v_vector);

			if(current_edge.is_portal)
			{
				color = SDL_MapRGB(screen->format, 60, 60, 60);
			}
			else
			{
				color = SDL_MapRGB(screen->format, 180, 180, 180);
			}

			GFX_draw_line(screen, current_line_start, current_line_end, color);
		}
	}
}

void draw_new_sector_preview()
{
	if(drawing_sector)
	{
		EDGE current_edge;

		for(int i = 0; i < creating_sector->e_num; i++)
		{
			current_edge = creating_sector->e[i];
			GFX_draw_line(	screen, 
							convert_ws_to_editor_ss(get_vertex_from_sector(creating_sector, i, 0)),
							convert_ws_to_editor_ss(get_vertex_from_sector(creating_sector, i, 1)),
							GFX_Map_Color(GFX_Color(255, 0, 0)));
		}

		VECTOR2 preview_cursor;

		if(snap_to_grid)
			preview_cursor = get_closest_grid(editor_cursor);
		else
			preview_cursor = editor_cursor;

		GFX_draw_line(	screen, 
						convert_ws_to_editor_ss(new_vector_start),
						convert_ws_to_editor_ss(preview_cursor),
						GFX_Map_Color(GFX_Color(255, 0, 0)));
	}
}

void draw_grid()
{
	int grid_x_amount;
	int grid_y_amount;

	grid_x_amount = (int)((bottom_right_border.x - top_left_border.x) / grid_size);
	grid_y_amount = (int)((top_left_border.y - bottom_right_border.y) / grid_size);

	VECTOR2 top_left_grid;
	POINT2 grid_screen_location;

	top_left_grid = get_closest_grid(sum_v2(top_left_border, vector2(grid_size/2., grid_size/2.)));

	for(int x = 0; x < grid_x_amount + 3; x++)
	{
		for(int y = 0; y < grid_y_amount + 3; y++)
		{
			grid_screen_location = convert_ws_to_editor_ss(sum_v2(top_left_grid, vector2((float)x*grid_size, -(float)y*grid_size)));
			
			GFX_set_pixel(screen, grid_screen_location.x, grid_screen_location.y, GFX_Map_Color(GFX_Color(30, 30, 30)), 0);
		}
	}
}

void draw_ui()
{
	if(snap_to_grid == 1)
		GFX_draw_string(point2(0,0), "Snap to grid", GFX_Map_Color(GFX_Color(255, 0, 100)));
	if(drawing_sector == 1)
		GFX_draw_string(point2(0,8), "Drawing Sector", GFX_Map_Color(GFX_Color(255, 0, 100)));

	if(grabbed == 1)
	{
		sprintf(buffer, "x = %f", grabbed_vector_location.x);
		GFX_draw_string(point2(0, 240-18), buffer, GFX_Map_Color(GFX_Color(255, 100, 0)));
		sprintf(buffer, "y = %f", grabbed_vector_location.y);
		GFX_draw_string(point2(0, 240-9), buffer, GFX_Map_Color(GFX_Color(255, 100, 0)));
	}

	if(show_info == 1)
	{
		sprintf(buffer, "SID = %i", closest_sector_index);
		GFX_draw_string(point2(120, 240-18), buffer, GFX_Map_Color(GFX_Color(255, 100, 0)));
		sprintf(buffer, "EID = %i", closest_edge_index);
		GFX_draw_string(point2(120, 240-9), buffer, GFX_Map_Color(GFX_Color(255, 100, 0)));
		sprintf(buffer, "PORTAL = %i", closest_edge->is_portal);
		GFX_draw_string(point2(188, 240-18), buffer, GFX_Map_Color(GFX_Color(255, 100, 0)));

		if(closest_edge->is_portal)
		{
			sprintf(buffer, "NID = %i", closest_edge->neighbor_sector_id);
			GFX_draw_string(point2(188, 240-9), buffer, GFX_Map_Color(GFX_Color(255, 100, 0)));
		}

		sprintf(buffer, "S=%i", loaded_level.s_num);
		GFX_draw_string(point2(274, 240-27), buffer, GFX_Map_Color(GFX_Color(255, 100, 0)));
		sprintf(buffer, "E=%i", (loaded_level.sectors + closest_sector_index)->e_num);
		GFX_draw_string(point2(274, 240-18), buffer, GFX_Map_Color(GFX_Color(255, 100, 0)));
		sprintf(buffer, "V=%i", loaded_level.v_num);
		GFX_draw_string(point2(274, 240-9), buffer, GFX_Map_Color(GFX_Color(255, 100, 0)));
		
		if(grabbed == 0)
		{
			sprintf(buffer, "x = %f", editor_cursor.x);
			GFX_draw_string(point2(0, 240-18), buffer, GFX_Map_Color(GFX_Color(255, 100, 0)));
			sprintf(buffer, "y = %f", editor_cursor.y);
			GFX_draw_string(point2(0, 240-9), buffer, GFX_Map_Color(GFX_Color(255, 100, 0)));
		}
	}
}

void draw_closest_markers()
{
	POINT2 closest_vector_ss;
	POINT2 closest_projection_ss;

	closest_vector_ss = convert_ws_to_editor_ss(closest_vector);
	closest_projection_ss = convert_ws_to_editor_ss(closest_edge_projection);

	unsigned int line_color;

	if(closest_edge->is_portal == 1)
	{
		line_color = GFX_Map_Color(GFX_Color(110, 0, 0));
	}
	else
	{
		line_color = GFX_Map_Color(GFX_Color(210, 0, 0));
	}

	GFX_draw_line(screen, convert_ws_to_editor_ss(get_vertex_at(closest_edge->v_start)), convert_ws_to_editor_ss(get_vertex_at(closest_edge->v_end)), line_color);

	GFX_set_pixel(screen, closest_projection_ss.x, closest_projection_ss.y, GFX_Map_Color(GFX_Color(0, 0, 255)), 0);

	GFX_set_pixel(screen, closest_vector_ss.x, closest_vector_ss.y, GFX_Map_Color(GFX_Color(255, 0, 0)), 0);
}

void EDITOR_Render()
{
	if(SDL_MUSTLOCK(screen))
	{
		if(SDL_LockSurface(screen) < 0)
		{
			printf("Couldnt lock screen: %s\n", SDL_GetError());
			return;
		}
	}

	GFX_clear_screen();

	draw_grid();

	draw_editor_map();
	draw_new_sector_preview();
	draw_closest_markers();

	draw_cursor();

	draw_ui();

	if(show_fps)
	{
		sprintf(buffer, "%f", current_fps);
		GFX_draw_string(point2(0, 0), buffer, SDL_MapRGB(screen->format, 255, 255, 0));
	}

	if(SDL_MUSTLOCK(screen))
	{
		SDL_UnlockSurface(screen);
	}

	SDL_UpdateRect(screen, 0, 0, SCREEN_RES_X * PIXEL_SCALE, SCREEN_RES_Y * PIXEL_SCALE);
}

void grab_vertex()
{
	grabbed = 1;
	occupied = 1;
	
	grabbed_vector_index = closest_vector_index;
	grabbed_vector_start = closest_vector;
}

void drop_vertex()
{
	grabbed = 0;
	occupied = 0;

	if(snap_to_grid)
		loaded_level.vertexes[grabbed_vector_index] = grabbed_vector_location;
	else
		loaded_level.vertexes[grabbed_vector_index] = grabbed_vector_location;
}

void cancel_grab()
{
	grabbed = 0;
	occupied = 0;

	loaded_level.vertexes[grabbed_vector_index] = grabbed_vector_start;
}

void cancel_new_sector()
{
	drawing_sector = 0;
	occupied = 0;

	free(creating_sector->e);
	free(creating_sector);

	WORLD_remove_n_vertexes(new_sector_size);

	new_sector_size = 0;
}

extern bool e_running;
SDL_Event event;

void EDITOR_Handle_Input()
{
	unsigned char * keystate = SDL_GetKeyState(NULL); 

	if(keystate[SDLK_LSHIFT])
	{
		cursor_speed = run_cursor_speed;
	}
	else if(keystate[SDLK_LCTRL])
	{
		cursor_speed = crawl_cursor_speed;
	}
	else
	{
		cursor_speed = walk_cursor_speed;
	}

	if(keystate[SDLK_w])
	{
		move_view(scale_v2(vector2(0, 1), cursor_speed * ENGINE_delta_time()));
	}
	
	if(keystate[SDLK_s])
	{
		move_view(scale_v2(vector2(0, -1), cursor_speed * ENGINE_delta_time()));
	}
	
	if(keystate[SDLK_d])
	{
		move_view(scale_v2(vector2(1, 0), cursor_speed * ENGINE_delta_time()));
	}
	
	if(keystate[SDLK_a])
	{
		move_view(scale_v2(vector2(-1, 0), cursor_speed * ENGINE_delta_time()));
	}

	if(keystate[SDLK_UP])
	{
		move_cursor(scale_v2(vector2(0, 1), cursor_speed * ENGINE_delta_time()));
	}
	
	if(keystate[SDLK_DOWN])
	{
		move_cursor(scale_v2(vector2(0, -1), cursor_speed * ENGINE_delta_time()));
	}
	
	if(keystate[SDLK_RIGHT])
	{
		move_cursor(scale_v2(vector2(1, 0), cursor_speed * ENGINE_delta_time()));
	}
	
	if(keystate[SDLK_LEFT])
	{
		move_cursor(scale_v2(vector2(-1, 0), cursor_speed * ENGINE_delta_time()));
	}
	
	if(keystate[SDLK_PAGEUP])
	{
		editor_zoom *= 1.0f + 2.0f * ENGINE_delta_time();
	}
	
	if(keystate[SDLK_PAGEDOWN])
	{
		editor_zoom *= 1.0f - 2.0f * ENGINE_delta_time();
	}

	while(SDL_PollEvent(&event) != 0)
	{
		if(event.type == SDL_QUIT)
		{
			e_running = false;
		}
		else if(event.type == SDL_KEYDOWN)
		{
			if(event.key.keysym.sym == 'm')
			{
				if(grabbed == 0 && occupied == 0)
					grab_vertex();
				else if(grabbed == 1)
					drop_vertex();
			}

			if(event.key.keysym.sym == 'p')
			{
				if(occupied == 0)
				{	
					edit_mode = !edit_mode;
					game_mode = !game_mode;
				}
			}

			if(event.key.keysym.sym == 'i')
			{
				show_info = !show_info;
			}

			if(event.key.keysym.sym == 'g')
				snap_to_grid = !snap_to_grid;

			if(event.key.keysym.sym == '+' || event.key.keysym.sym == '=')
			{
				grid_size += 0.1f;
			}

			if(event.key.keysym.sym == '_' || event.key.keysym.sym == '-')
			{	
				if(grid_size >= 0.15f) grid_size -= 0.1f;
			}
			
			if(event.key.keysym.sym == SDLK_RETURN)
			{
				if(grabbed)
					drop_vertex();
			}

			if(event.key.keysym.sym == SDLK_ESCAPE)
			{
				if(grabbed)
					cancel_grab();

				if(drawing_sector)
					cancel_new_sector();
			}

			if(event.key.keysym.sym == SDLK_DELETE && occupied == 0)
			{
				delete_vertex();
			}

			if(event.key.keysym.sym == SDLK_SPACE)
			{
				if(drawing_sector == 0 && occupied == 0)
					new_sector();
				else if(drawing_sector == 1)
					new_edge();
			}
		}
	}
}

void EDITOR_Loop()
{
	update_borders();

	clip_cursor();

	if(grabbed == 1 && snap_to_grid == 0)
	{
		grabbed_vector_location = editor_cursor;
		loaded_level.vertexes[grabbed_vector_index] = editor_cursor;
	}
	else if(grabbed == 1 && snap_to_grid == 1)
	{
		grabbed_vector_location = get_closest_grid(editor_cursor);
		loaded_level.vertexes[grabbed_vector_index] = get_closest_grid(editor_cursor);
	}

	get_closest_vertex(editor_cursor, &closest_vector, &closest_vector_index, &closest_vector_distance);
	get_closest_edge(editor_cursor, &closest_edge, &closest_edge_projection, &closest_edge_index, &closest_sector_index, &closest_edge_distance);
}

void EDITOR_Init()
{
	editor_center = vector2(0., 0.);
	editor_cursor = vector2(0., 0.);

	closest_edge = get_edge_from_level(0, 0);
}