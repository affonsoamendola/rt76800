#include <stdio.h>
#include <stdlib.h>

//#include "list.h"
//#include "console.h"
#include "ff_vector2.h"
//#include "player.h"
#include "ff_linked_list.h"

#include "world.h"

World world;

void init_world()
{
	/*
	world.vertex_size = sizeof(vertex_test)/sizeof(Vector2f);
	world.vertexes = (Vector2f*)&vertex_test;

	world.edge_size = sizeof(edge_test)/sizeof(Edge);
	world.edges = (Edge*)&edge_test;
	*/

	ff_initialize_list(&world.vertexes);
	ff_initialize_list(&world.edges);
	ff_initialize_list(&world.sectors);
	ff_initialize_list(&world.entities);
}

void quit_world()
{
}

void level_add_edge(Vertex* start, Vertex* end)
{
	Edge new_edge = {start, end, {0}, 0, {NULL,NULL}};
	ff_pushback_list(&world.edges, &new_edge);
}

void level_add_vertex(Vector2f new_vertex_)
{
 	Vertex new_vertex = {new_vertex_};
	ff_pushback_list(&world.vertexes, &new_vertex);
}

//Destroyers removers of lists
void level_destroy_sector(Sector * sector)
{
	uint32_t found_index;
	if(ff_find_list(&world.sectors, &found_index, sector))
	{
		ff_remove_at_list(&world.sectors, found_index);
	}
}

void level_destroy_edge(Edge * edge)
{
	uint32_t found_index;
	if(ff_find_list(&world.edges, &found_index, edge))
	{
		ff_remove_at_list(&world.edges, found_index);
	}
}

void level_destroy_vertex(Vertex * vertex)
{
	uint32_t found_index;
	if(ff_find_list(&world.vertexes, &found_index, vertex))
	{
		ff_remove_at_list(&world.vertexes, found_index);
	}
}
//End destroyers

//Removes a sector from the level, does all the hierarquical solution for the lower
//levels, goes trough vertexes and edges and removes those who dont need to exist anymore.
//Also removes the neighbor information from edges that dont need it anymore.
void level_remove_sector(Sector* sector)
{
	bool is_last_edge_portal = sector->edges[sector->edge_size-1]->is_portal;

	//Goes through every edge in the sector
	for(int e = 0; e < sector->edge_size; e++)
	{
		Edge* current_edge = sector->edges[e];

		//If last and current edge arent portals, it means the vertex between them
		//is only used by this sector, and can be destroyed.
		if(!is_last_edge_portal && !current_edge->is_portal)
		{
			level_destroy_vertex(current_edge->vertex_start);
		}

		//If edge is not a portal, it means that it is only used by this sector and can be destroyed.
		if(!current_edge->is_portal)
		{
			level_destroy_edge(current_edge);
		}
		else //If IT IS a portal then it wont be after this sector is removed, and that edge wont neighbor this sector anymore.
		{
			current_edge->is_portal = false;

			if(current_edge->neighboring_sectors[0] == sector) current_edge->neighboring_sectors[0] = NULL;
			if(current_edge->neighboring_sectors[1] == sector) current_edge->neighboring_sectors[1] = NULL;
		}
	}

	//Frees the edges array, since it was malloced on creation.
	free(sector->edges);

	level_destroy_sector(sector);
}
/*
Vector2f WORLD_get_vertex_at(const uint32_t index)
{
	if(index < world.vertex_size)
	{
		return *(world.vertexes + index);
	}
	else
	{
		return vector2f(0, 0);
	}
}
/*
void WORLD_delete_edge_at(int sector_index, int edge_index)
{
	EDGE * new_e;
	SECTOR * sector;

	sector = loaded_level.sectors + sector_index;

	new_e = malloc(sizeof(EDGE) * (sector->e_num-1));

	for(int i = 0; i < edge_index; i++)
	{
		new_e[i] = sector->e[i];
	}

	for(int i = edge_index + 1; i < sector->e_num; i++)
	{
		new_e[i-1] = sector->e[i];
	}

	sector->e_num -= 1;

	free(sector->e);

	if(sector->e_num == 0)
	{
		WORLD_delete_sector_at(sector_index);
	}
	else
	{
		sector->e = new_e;
	}
}
/*
void WORLD_delete_sector_at(int index)
{
	SECTOR * new_sectors;

	new_sectors = malloc(sizeof(SECTOR) * (loaded_level.s_num - 1));

	for(int i = 0; i < index; i++)
	{
		new_sectors[i] = loaded_level.sectors[i];
	}

	for(int i = index + 1; i < loaded_level.s_num; i++)
	{
		loaded_level.sectors[i].sector_id--;
		new_sectors[i-1] = loaded_level.sectors[i];
	}

	free(loaded_level.sectors);

	loaded_level.sectors = new_sectors;
	loaded_level.s_num -= 1;
}

void WORLD_delete_vertex_at(int index)
{
	if(loaded_level.v_num > 0)
	{
		VECTOR2 * new_vertexes;

		new_vertexes = malloc(sizeof(VECTOR2) * (loaded_level.v_num - 1));

		for(int i = 0; i < index; i++)
		{
			new_vertexes[i] = loaded_level.vertexes[i];
		}

		for(int i = index + 1; i < loaded_level.v_num; i ++)
		{
			new_vertexes[i - 1] = loaded_level.vertexes[i];
		}

		loaded_level.v_num -= 1;

		free(loaded_level.vertexes);
		loaded_level.vertexes = new_vertexes;

		SECTOR * current_sector;
		EDGE * current_edge;
		EDGE * next_edge;
		int next_edge_index;

		int sector_amount;

		sector_amount = loaded_level.s_num;

		for(int s = 0; s < sector_amount; s++)
		{
			current_sector = loaded_level.sectors + s;

			for(int e = 0; e < current_sector->e_num; e++)
			{	
				current_edge = current_sector->e + e;

				if(e == current_sector->e_num - 1)
				{
					next_edge_index = 0;
				}
				else
				{
					next_edge_index = e + 1;
				}

				next_edge = current_sector->e + next_edge_index;

				if(current_edge->v_end == index)
				{
					current_edge->v_end = next_edge->v_end;

					delete_edge_at(s, next_edge_index);

					if(sector_amount > loaded_level.s_num)
					{
						sector_amount = loaded_level.s_num;
						s--;
						break;
					}
				}
			}
		}

		for(int s = 0; s < loaded_level.s_num; s++)
		{
			current_sector = loaded_level.sectors + s;

			for(int e = 0; e < current_sector->e_num; e++)
			{	
				current_edge = current_sector->e + e;

				if(current_edge->v_start > index)
					current_edge->v_start--;

				if(current_edge->v_end > index)
					current_edge->v_end--;
			}
		}
	}
}

EDGE * get_edge_from_level(int sector_index, int edge_index)
{
	SECTOR * sector;
	EDGE * edge;

	edge = NULL;

	if(sector_index < loaded_level.s_num)
	{
		sector = loaded_level.sectors + sector_index;
		
		if(edge_index < sector->e_num)
		{	
			edge = sector->e + edge_index;
		}
	}

	return edge;
}

EDGE * get_edge_at(SECTOR * sector, int edge_index)
{
	if(edge_index >= 0 && edge_index < sector->e_num)
	{
		return sector->e + edge_index;
	}
	else
	{
		return sector->e;
	}
}

SECTOR * get_sector_at(int sector_index)
{
	if(sector_index >= 0 && sector_index < loaded_level.s_num)
	{
		return loaded_level.sectors + sector_index;
	}
	else
	{
		return loaded_level.sectors;
	}
}

VECTOR2 get_vertex_from_sector(SECTOR * sector, int edge_index, int start_or_end)
{
	if(start_or_end == 0)
	{
		return get_vertex_at(get_edge_at(sector, edge_index)->v_start);
	}
	else
	{
		return get_vertex_at(get_edge_at(sector, edge_index)->v_end);
	}
}

void level_load(const char * file_location)
{
	FILE * level_file;

	LEVEL new_level;

	int success = 1;

	int vertex_num;
	int sector_num;

	int sector_size;

	float x;
	float y;

	level_file = fopen(file_location, "r");

	if(level_file == NULL)
	{
		success = 0;
	}
	else
	{
		fscanf(level_file, "%u", &vertex_num);
		fscanf(level_file, "%u", &sector_num);

		new_level.v_num = vertex_num;
		new_level.vertexes = malloc(vertex_num * sizeof(VECTOR2));
		new_level.s_num = sector_num;
		new_level.sectors = malloc(sector_num * sizeof(SECTOR));

		for(int v = 0; v < vertex_num; v++)
		{
			fscanf(level_file, "%f %f", &x, &y);

			*(new_level.vertexes + v) = vector2(x, y);
		}

		for(int s = 0; s < sector_num; s++)
		{
			SECTOR * current_sector;

			current_sector = new_level.sectors + s;

			fscanf(level_file, "%u", &sector_size);

			current_sector->sector_id = s;
			current_sector->e_num = sector_size;
			current_sector->e = malloc(sector_size * sizeof(EDGE));

			fscanf(level_file, "%f", &(current_sector->floor_height));
			fscanf(level_file, "%f", &(current_sector->ceiling_height));
			
			int * sector_vertexes;

			sector_vertexes = malloc(sector_size * sizeof(int));

			for(int v = 0; v < sector_size; v++)
			{
				fscanf(level_file, "%u", sector_vertexes + v);
			}

			float r;
			float g;
			float b;

			fscanf(level_file, "%f %f %f", &r, &g, &b);

			current_sector->tint = GFX_Tint(r,g,b);

			for(int e = 0; e < sector_size; e++)
			{
				EDGE new_edge;
				int end_vertex;

				new_edge.v_start = *(sector_vertexes + e);

				fscanf(level_file, "%u", &(new_edge.text_param.id));
				fscanf(level_file, "%u", &(new_edge.text_param.parallax));
				fscanf(level_file, "%u", &(new_edge.text_param.u_offset));
				fscanf(level_file, "%u", &(new_edge.text_param.v_offset));
				fscanf(level_file, "%f", &(new_edge.text_param.u_scale));
				fscanf(level_file, "%f", &(new_edge.text_param.v_scale));

				end_vertex = e + 1;

				if(end_vertex >= sector_size)
				{
					end_vertex = 0;
				}

				new_edge.v_end = *(sector_vertexes + end_vertex);

				new_edge.is_portal = false;
				new_edge.neighbor_sector_id = -1;

				for(int old_s = 0; old_s < s; old_s ++)
				{
					SECTOR * checking_sector;

					checking_sector = new_level.sectors + old_s;

					for(int old_e = 0; old_e < checking_sector->e_num; old_e++)
					{
						EDGE * checking_edge;

						checking_edge = checking_sector->e + old_e;

						if( (new_edge.v_start == checking_edge->v_start && new_edge.v_end == checking_edge->v_end) ||
							(new_edge.v_start == checking_edge->v_end && new_edge.v_end == checking_edge->v_start))
						{
							new_edge.is_portal = true;
							checking_edge->is_portal = true;

							new_edge.neighbor_sector_id = old_s;
							checking_edge->neighbor_sector_id = s;
						}
					}
				}

				*(current_sector->e + e) = new_edge;
			}

			fscanf(level_file, "%u", &(current_sector->text_param_ceil.id));
			fscanf(level_file, "%u", &(current_sector->text_param_ceil.parallax));
			fscanf(level_file, "%u", &(current_sector->text_param_ceil.u_offset));
			fscanf(level_file, "%u", &(current_sector->text_param_ceil.v_offset));
			fscanf(level_file, "%f", &(current_sector->text_param_ceil.u_scale));
			fscanf(level_file, "%f", &(current_sector->text_param_ceil.v_scale));

			fscanf(level_file, "%u", &(current_sector->text_param_floor.id));
			fscanf(level_file, "%u", &(current_sector->text_param_floor.parallax));
			fscanf(level_file, "%u", &(current_sector->text_param_floor.u_offset));
			fscanf(level_file, "%u", &(current_sector->text_param_floor.v_offset));
			fscanf(level_file, "%f", &(current_sector->text_param_floor.u_scale));
			fscanf(level_file, "%f", &(current_sector->text_param_floor.v_scale));
		}

		fclose(level_file);
	}

	if(success == 1)
	{
		loaded_level = new_level;
		printf_console("\nLoaded ");
		printf_console((char *)file_location);
	}
	else
	{
		printf_console("\nError loading level at ");
		printf_console((char *)file_location);
	}
}
*/
/*
uint32_t WORLD_add_vertex(const Vector2f vertex)
{
	world.vertexes = realloc(world.vertexes, sizeof(Vector2f) * (world.vertex_size + 1));

	world.vertexes[world.vertex_size] = vertex;
	world.vertex_size += 1;

	return world.vertex_size;
}
/*
void WORLD_remove_n_vertexes(int n)
{
	VECTOR2 * new_vertexes;

	int new_vertexes_size;

	new_vertexes_size = loaded_level.v_num - n;

	new_vertexes = malloc(sizeof(VECTOR2) * (new_vertexes_size));

	for(int v = 0; v < new_vertexes_size; v ++)
	{
		new_vertexes[v] = loaded_level.vertexes[v];
	}

	loaded_level.v_num -= n;

	free(loaded_level.vertexes);

	loaded_level.vertexes = new_vertexes;
}

int WORLD_add_edge_to_sector(SECTOR * sector, int vertex_start_index, int vertex_end_index)
{
	int new_edge_index = sector->e_num;

	EDGE new_edge;

	EDGE * new_e;

	new_e = malloc(sizeof(EDGE) * (new_edge_index + 1));	

	for(int i = 0; i < new_edge_index; i++)
	{
		new_e[i] = sector->e[i];
	}

	new_edge.v_start = vertex_start_index;
	new_edge.v_end = vertex_end_index;

	new_edge.text_param = DEFAULT_TEXTURE_PARAM;

	new_edge.is_portal = 0;
	new_edge.neighbor_sector_id = -1;

	SECTOR * old_sector;
	EDGE * old_edge;

	for(int old_s = 0; old_s < loaded_level.s_num; old_s ++)
	{
		old_sector = loaded_level.sectors + old_s;

		for(int old_e = 0; old_e < old_sector->e_num; old_e ++)
		{
			old_edge = old_sector->e + old_e;

			if( (new_edge.v_start == old_edge->v_start && new_edge.v_end == old_edge->v_end) ||
				(new_edge.v_start == old_edge->v_end && new_edge.v_end == old_edge->v_start))
			{
				new_edge.is_portal = true;
				old_edge->is_portal = true;

				new_edge.neighbor_sector_id = old_s;
				old_edge->neighbor_sector_id = sector->sector_id;
			}
		}	
	}

	new_e[new_edge_index] = new_edge;

	free(sector->e);
	sector->e = new_e;

	sector->e_num = (sector->e_num) + 1;

	return new_edge_index;
}	

int WORLD_add_sector_to_level(SECTOR * sector)
{
	int new_sector_index = loaded_level.s_num;

	SECTOR * new_sectors;

	new_sectors = malloc(sizeof(SECTOR) * (new_sector_index + 1));

	for(int i = 0; i < new_sector_index; i++)
	{
		new_sectors[i] = loaded_level.sectors[i];
	}

	sector->sector_id = new_sector_index;
	sector->tint = GFX_Tint(1., 1., 1.);
	new_sectors[new_sector_index] = *sector;
	loaded_level.s_num += 1;

	free(loaded_level.sectors);
	loaded_level.sectors = new_sectors;

	return new_sector_index;
}

void get_closest_vertex(VECTOR2 pos, VECTOR2 * closest, int * vertex_index, float * distance)
{
	VECTOR2 current_vector;
	float current_distance;

	int closest_vector_i = 0;
	VECTOR2 closest_vector = loaded_level.vertexes[0];
	float closest_distance = norm_v2(sub_v2(pos, closest_vector));

	for(int v = 0; v < loaded_level.v_num; v++)
	{
		current_vector = loaded_level.vertexes[v];

		current_distance = norm_v2(sub_v2(pos, loaded_level.vertexes[v]));

		if(current_distance < closest_distance)
		{
			closest_vector_i = v;
			closest_vector = current_vector;
			closest_distance = current_distance;
		}
	}

	if(closest != NULL)
	{
		*closest = closest_vector;
	}

	if(vertex_index != NULL)
	{
		*vertex_index = closest_vector_i;
	}

	if(distance != NULL)
	{
		*distance = closest_distance;
	}
}

void get_closest_edge(VECTOR2 pos, EDGE ** edge, VECTOR2 * projection, int * edge_index, int * sector_index, float * distance)
{
	float closest_edge_index;
	float closest_sector_index;
	float closest_distance;
	VECTOR2 closest_projection;

	SECTOR * current_sector;
	EDGE * current_edge;
	VECTOR2 current_projection;

	float current_distance;

	current_distance = distance_v2_to_segment(pos, get_vertex_at(loaded_level.sectors->e->v_start), get_vertex_at(loaded_level.sectors->e->v_end), &current_projection);
	
	closest_edge_index = 0;
	closest_sector_index = 0;
	closest_distance = current_distance;
	closest_projection = current_projection;

	for(int s = 0; s < loaded_level.s_num; s++)
	{
		current_sector = loaded_level.sectors + s;

		for(int e = 0; e < current_sector->e_num; e++)
		{
			current_edge = current_sector->e + e;

			current_distance = distance_v2_to_segment(pos, get_vertex_at(current_edge->v_start), get_vertex_at(current_edge->v_end), &current_projection);
		
			if(	current_distance < closest_distance && point_side_v2(pos, get_vertex_at(current_edge->v_start), get_vertex_at(current_edge->v_end)) == -1)
			{
				closest_edge_index = e;
				closest_sector_index = s;
				closest_distance = current_distance;
				closest_projection = current_projection;
			}
		}
	}

	if(edge != NULL)
		*edge = get_edge_from_level(closest_sector_index, closest_edge_index);

	if(projection != NULL)
		*projection = closest_projection;

	if(edge_index != NULL)
		*edge_index = closest_edge_index;

	if(sector_index != NULL)
		*sector_index = closest_sector_index;

	if(distance != NULL)
		*distance = closest_distance;
}

VECTOR2 convert_ss_to_ws(CAMERA * camera, POINT2 screen_space, float height)
{
	VECTOR2 world_space;

	world_space.y = -(((float)(height)) * camera->camera_parameters_y)/(float)(screen_space.y - engine.screen_res_y/2);
	world_space.x = ((float)(screen_space.x - engine.screen_res_x/2) * world_space.y) / camera->camera_parameters_x;

	world_space = rot_v2(world_space, -(player->facing));
	world_space = sum_v2(world_space, player->pos);

	return world_space;
}

VECTOR2 convert_ss_to_rs(CAMERA * camera, POINT2 screen_space, float height)
{
	VECTOR2 relative_space;

	relative_space.y = -(((float)(height)) * camera->camera_parameters_y)/(float)(screen_space.y - engine.screen_res_y/2);
	relative_space.x = ((float)(screen_space.x - engine.screen_res_x/2) * relative_space.y) / camera->camera_parameters_x;

	return relative_space;
}

VECTOR2 convert_rs_to_ws(VECTOR2 relative_space)
{
	VECTOR2 world_space;

	world_space = rot_v2(world_space, -(player->facing));
	world_space = sum_v2(world_space, player->pos);

	return world_space;
}

POINT2 convert_ws_to_ss(CAMERA * camera, VECTOR2 world_space, float height)
{
	VECTOR2 transformed_pos;
	float transformed_height;

	POINT2 screen_space;

	transformed_pos = sub_v2(world_space, player->pos);
	transformed_pos = rot_v2(transformed_pos, player->facing);

	transformed_height = height - player->pos_height;

	float xscale = camera->camera_parameters_x / transformed_pos.y;
	float yscale = camera->camera_parameters_y / transformed_pos.y;

	float proj_x = transformed_pos.x * xscale;
	float proj_height = transformed_height * yscale;

	int x = (int)(proj_x) + engine.screen_res_x/2;
	int y = engine.screen_res_y/2 - (int)(proj_height);

	screen_space.x = x;
	screen_space.y = y;

	return screen_space;
}

void convert_ws_to_rs(	VECTOR2 world_space, float world_height,
						VECTOR2 * relative_space, float * relative_height)
{
	*relative_space = sub_v2(world_space, player->pos);
	*relative_space = rot_v2(*relative_space, player->facing);

	*relative_height = world_height - player->pos_height;
}

POINT2 convert_rs_to_ss(CAMERA * camera, VECTOR2 relative_space, float relative_height)
{
	POINT2 screen_space;

	float xscale = camera->camera_parameters_x / relative_space.y;
	float yscale = camera->camera_parameters_y / relative_space.y;

	float proj_x = relative_space.x * xscale;
	float proj_height = relative_height * yscale;

	int x = (int)(proj_x) + engine.screen_res_x/2;
	int y = engine.screen_res_y/2 - (int)(proj_height);

	screen_space.x = x;
	screen_space.y = y;

	return screen_space;
}

//Checks collision in the world, takes a lot of parameters,
//start_sector, is the sector which the start_pos is located, move amount is a vector which shows how much to move from the start pos
//intersected pos is a pointer to a vector 2, will be filled with a position which is an intersection of start_pos+move_amounte and a wall
//check_knees will check if a ledge is above a certain knee_height value, if it is, it wont allow movement and act like a wall
//pos height is the current height of start_pos, and height is how tall its hitbox is, to check for head collisions with inverted ledges.

//Returns NO_COLLISION if no collision, COLLIDED if collision, and NO_COLLISION_SECTOR_CHANGE if 
int WORLD_Check_Collision(	int start_sector, VECTOR2 start_pos, VECTOR2 move_amount, VECTOR2 * intersected_position, int * end_sector, 
							int check_knees, float pos_height, float height, float knee_height)
{
	SECTOR * current_sector; 
	EDGE * current_edge;
	EDGE * neighbor_edge;

	VECTOR2 intersection_location;

	VECTOR2 to_location;

	to_location = sum_v2(start_pos, move_amount);

	current_sector = get_sector_at(start_sector);
	int changed_sector = 0;

	for(int e = 0; e < current_sector->e_num; e++)
	{
		current_edge = get_edge_at(current_sector, e);

		if(intersect_check_v2(start_pos, to_location, get_vertex_from_sector(current_sector, e, 0),  get_vertex_from_sector(current_sector, e, 1), &intersection_location) 
			== 1)
		{
			if(current_edge->is_portal)
			{
				for(int i  = 0; i < 3; i++)
				{
					SECTOR * neighbor_sector = get_sector_at(current_edge->neighbor_sector_id);

					if(check_knees)
					{
						if(pos_height + knee_height > neighbor_sector->floor_height)
						{
							changed_sector = 1;
							*end_sector = current_edge->neighbor_sector_id;
						}
						else return COLLIDED;
					}
					else
					{
						if(pos_height > neighbor_sector->floor_height)
						{
							changed_sector = 1;
							*end_sector = current_edge->neighbor_sector_id;
						}
						else return COLLIDED;
					}

					for(int e_n = 0; e_n < neighbor_sector->e_num; e_n++)
					{
						neighbor_edge = get_edge_at(neighbor_sector, e_n);

						if(neighbor_edge->neighbor_sector_id != current_sector->sector_id)
						{
							if(intersect_check_v2(start_pos, to_location, get_vertex_from_sector(neighbor_sector, e_n, 0),  get_vertex_from_sector(neighbor_sector, e_n, 1), &intersection_location) 
								== 1)
							{
								if(neighbor_edge->is_portal) 
								{
									current_sector = neighbor_sector;
									current_edge = neighbor_edge;
									break;
								}
								else
								{
									return COLLIDED;
								}
							}
						}
					}
				}
			}
		 	else
			{
				return COLLIDED;
			}
			break;	
		}
	}

	if(changed_sector) return NO_COLLISION_SECTOR_CHANGE;
	else return NO_COLLISION;
}
*/