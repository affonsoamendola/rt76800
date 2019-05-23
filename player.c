#include "vector2.h"
#include "world.h"
#include "engine.h"

#include "player.h"

extern LEVEL loaded_level;

extern VECTOR2 editor_cursor;

extern int closest_vector_index;
extern int closest_edge_index;
extern int closest_sector_index;
extern VECTOR2 closest_edge_projection;

extern VECTOR2 closest_vector;
extern EDGE * closest_edge;
extern SECTOR * closest_sector;

extern float closest_vector_distance;
extern float closest_edge_distance;

PLAYER * player;

void PLAYER_Init(PLAYER ** player)
{
	PLAYER initted_player;

	initted_player.pos = vector2(PLAYER_START_X, PLAYER_START_Y);

	initted_player.height = PLAYER_START_HEIGHT;

	initted_player.pos_height = PLAYER_START_HEIGHT;
	initted_player.h_velocity = 0.;

	initted_player.facing = 0.00001;

	initted_player.noclip = 0;

	initted_player.walk_speed = 2.0f;
	initted_player.run_speed = 4.0f;

	initted_player.walk_turn_speed = 2.0f;
	initted_player.run_turn_speed = 3.0f;

	initted_player.speed = 1.0f;
	initted_player.turn_speed = 0.5f;

	initted_player.current_sector = 0;

	initted_player.max_health = PLAYER_MAX_DEFAULT_HEALTH;
	initted_player.health = PLAYER_MAX_DEFAULT_HEALTH;

	initted_player.max_armor = PLAYER_MAX_DEFAULT_ARMOR;
	initted_player.armor = PLAYER_MAX_DEFAULT_ARMOR;

	initted_player.current_weapon = 2;

	initted_player.movement_blocked = 0;

	for(int i = 0; i < 10; i ++) initted_player.ammo[i] = 33;
	
	initted_player.max_ammo[0] = WEAPON_0_MAX_AMMO; 
	initted_player.max_ammo[1] = WEAPON_1_MAX_AMMO; 
	initted_player.max_ammo[2] = WEAPON_2_MAX_AMMO; 
	initted_player.max_ammo[3] = WEAPON_3_MAX_AMMO; 
	initted_player.max_ammo[4] = WEAPON_4_MAX_AMMO; 
	initted_player.max_ammo[5] = WEAPON_5_MAX_AMMO; 
	initted_player.max_ammo[6] = WEAPON_6_MAX_AMMO; 
	initted_player.max_ammo[7] = WEAPON_7_MAX_AMMO; 
	initted_player.max_ammo[8] = WEAPON_8_MAX_AMMO; 
	initted_player.max_ammo[9] = WEAPON_9_MAX_AMMO; 

	get_closest_vertex(initted_player.pos, &initted_player.closest_vector, &closest_vector_index, &initted_player.closest_vector_distance);
	get_closest_edge(initted_player.pos, &initted_player.closest_edge, &closest_edge_projection, &closest_edge_index, &closest_sector_index, &initted_player.closest_edge_distance);
	initted_player.closest_sector = get_sector_at(closest_sector_index);
	
	*player = malloc(sizeof(PLAYER));
	**player = initted_player;
}

void PLAYER_Update()
{
	if(player->noclip == 0)
	{
		player->is_grounded = 0;

		if(player->h_velocity > 0.) 
			player->is_grounded = 0;
		else if(player->pos_height - player->height <= loaded_level.sectors[player->current_sector].floor_height)
		{
			player->is_grounded = 1;
			player->h_velocity = 0.;
			player->pos_height = loaded_level.sectors[player->current_sector].floor_height + player->height;	
		}

		if(player->is_grounded == 0)
		{
			player->h_velocity = player->h_velocity + (GRAVITY * ENGINE_delta_time());
			player->pos_height = player->pos_height + (player->h_velocity * ENGINE_delta_time());
		}
	}
}

void PLAYER_Move(PLAYER * player, VECTOR2 amount)
{
	SECTOR * current_sector;

	int allow_move = 1;

	VECTOR2 to_pos = sum_v2(player->pos, amount);

	current_sector = loaded_level.sectors + player->current_sector;

	for(int e = 0; e < current_sector->e_num; e ++)
	{
		EDGE * current_edge = get_edge_at(current_sector, e);

		VECTOR2 edge_vertex_0 = get_vertex_from_sector(current_sector, e, 0);
		VECTOR2 edge_vertex_1 = get_vertex_from_sector(current_sector, e, 1);

		if(intersect_box_v2(player->pos, to_pos, edge_vertex_0, edge_vertex_1))
		{
			if(point_side_v2(to_pos, edge_vertex_0, edge_vertex_1) > 0)
			{
				if(current_edge->is_portal)
				{
					float neighbor_sector_floor;

					neighbor_sector_floor = loaded_level.sectors[current_edge->neighbor_sector_id].floor_height;

					if(current_sector->floor_height + player->pos_height - player->height + PLAYER_KNEE > neighbor_sector_floor)
					{
						allow_move = 1;
						player->current_sector = current_edge->neighbor_sector_id;
					}
					else
					{
						if(player->noclip == 0)
							allow_move = 0;
						else
							allow_move = 1;
					}
				}
				else
				{
					if(player->noclip == 0)
						allow_move = 0;
				}
			}
		}
	}

	if(allow_move && !player->movement_blocked)
	{
		player->pos = to_pos;
		
		get_closest_vertex(player->pos, &player->closest_vector, &closest_vector_index, &player->closest_vector_distance);
		get_closest_edge(player->pos, &player->closest_edge, &closest_edge_projection, &closest_edge_index, &closest_sector_index, &player->closest_edge_distance);
		player->closest_sector = get_sector_at(closest_sector_index);
	}
}

void PLAYER_Turn(PLAYER * player, float amount)
{
	if(!player->movement_blocked)
	{
		player->facing = player->facing + amount;

		if(player->facing >= 2.*PI) player->facing -= 2*PI;
		if(player->facing < 0.) player->facing += 2*PI;
	}	
}

void PLAYER_Jump()
{
	if(player->is_grounded)
	{
		player->h_velocity = JUMP_VELOCITY;
	}
}
