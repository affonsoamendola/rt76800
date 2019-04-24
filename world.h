#ifndef WORLD_H
#define WORLD_H

#include "vector2.h"
#include "gfx.h"

typedef struct EDGE_
{
	int v_start;
	float v_start_floor_height;
	float v_start_ceiling_height;

	int v_end;
	float v_end_floor_height;
	float v_end_ceiling_height;

	GFX_TEXTURE_PARAM text_param;

	int is_portal;
	int neighbor_sector_id;
}
EDGE;

typedef struct SECTOR_
{
	int sector_id;
	int e_num;

	GFX_TEXTURE_PARAM text_param_ceil;
	GFX_TEXTURE_PARAM text_param_floor;

	EDGE * e;
}
SECTOR;

typedef struct LEVEL_
{
	int v_num;
	VECTOR2 * vertexes;
	int s_num;
	SECTOR * sectors;
}
LEVEL;

void WORLD_Init();

void level_load(const char * file_location);
VECTOR2 get_vertex_at(int index);

EDGE * get_edge_at(SECTOR * sector, int edge_index);

VECTOR2 get_vertex_from_sector(SECTOR * sector, int edge_index, int start_or_end);

#endif