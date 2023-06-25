#include "GameServer.h"

void cGameServer::Process_Move_Test(const int user_id, void* buff)
{
	auto start_time = chrono::steady_clock::now();
	cs_packet_move_test* packet = reinterpret_cast<cs_packet_move_test*>(buff);
	m_clients[user_id].set_user_velocity(packet->velocity);
	m_clients[user_id].set_user_yaw(packet->yaw);
	m_clients[user_id].update_rotation(packet->yaw);
	m_clients[user_id].set_look(packet->look);
	m_clients[user_id].set_right(packet->right);
	m_clients[user_id].set_inputKey(packet->input_key);
	m_clients[user_id].set_isjump(packet->is_jump);
	Room& join_room = *m_room_manager->Get_Room_Info(m_clients[user_id].get_join_room_number());

	XMFLOAT3 current_player_position = m_clients[user_id].get_user_position();

	XMFLOAT3 current_shift = packet->xmf3Shift;
	XMFLOAT3 calculate_player_position = Add(current_player_position, current_shift);
	bool collision_up_face = false;

	if (m_clients[user_id].get_user_position().y < 0)
		calculate_player_position.y = 0;

	if (m_clients[user_id].get_join_room_number() >= 0) {
		CollisionInfo player_check = join_room.is_collision_player_to_player(user_id, current_player_position, current_shift);
		if (player_check.is_collision)
		{
			calculate_player_position = Add(current_player_position, player_check.SlidingVector);
			if (m_clients[user_id].get_user_position().y < 0)
				calculate_player_position.y = 0;
			current_shift = player_check.SlidingVector;
			if (player_check.collision_face_num == 4)
				collision_up_face = true;
		}

		CollisionInfo wall_check = join_room.is_collision_wall_to_player(user_id, current_player_position, current_shift);
		if (wall_check.is_collision)
		{
			calculate_player_position = Add(current_player_position, wall_check.SlidingVector);
			if (m_clients[user_id].get_user_position().y < 0)
				calculate_player_position.y = 0;
			current_shift = wall_check.SlidingVector;
			if (wall_check.collision_face_num == 4)
				collision_up_face = true;
		}

		CollisionInfo fix_object_check = join_room.is_collision_fix_object_to_player(user_id, current_player_position, current_shift);
		if (fix_object_check.is_collision)
		{
			calculate_player_position = Add(current_player_position, fix_object_check.SlidingVector);
			if (m_clients[user_id].get_user_position().y < 0)
				calculate_player_position.y = 0;
			current_shift = fix_object_check.SlidingVector;
			if (fix_object_check.collision_face_num == 4)
				collision_up_face = true;
		}

		CollisionInfo door_check = join_room.is_collision_player_to_door(user_id, current_player_position, current_shift);
		if (door_check.is_collision)
		{
			calculate_player_position = Add(current_player_position, door_check.SlidingVector);
			if (m_clients[user_id].get_user_position().y < 0)
				calculate_player_position.y = 0;
			current_shift = door_check.SlidingVector;
			if (door_check.collision_face_num == 4)
				collision_up_face = true;
		}

		CollisionInfo object_check = join_room.is_collision_player_to_object(user_id, current_player_position, current_shift);
		if (object_check.is_collision)
		{
			// 이쪽은 오브젝트와 충돌한것을 처리하는 부분입니다.
			if (object_check.collision_face_num == 4)
				collision_up_face = true;
		}
	}

	m_clients[user_id].set_user_position(calculate_player_position);
	m_clients[user_id].update_bounding_box_pos(calculate_player_position);
	m_clients[user_id].set_collied_up_face(collision_up_face);

	sc_packet_calculate_move_test move_packet;

	move_packet.size = sizeof(move_packet);
	move_packet.type = SC_PACKET::SC_PACKET_CALCULATE_MOVE;
	move_packet.id = user_id;
	move_packet.pos.x = static_cast<int>(m_clients[user_id].get_user_position().x * 10000);
	move_packet.pos.y = static_cast<int>(m_clients[user_id].get_user_position().y * 10000);
	move_packet.pos.z = static_cast<int>(m_clients[user_id].get_user_position().z * 10000);
	move_packet.is_collision_up_face = m_clients[user_id].get_user_collied_up_face();
	move_packet.move_time = packet->move_time;

	m_clients[user_id].do_send(sizeof(move_packet), &move_packet);

	Room& room = *m_room_manager->Get_Room_Info(m_clients[user_id].get_join_room_number());
	if (m_clients[user_id].m_befor_send_move == false)
	{
		m_clients[user_id].m_befor_send_move = true;
		for (auto p : room.in_player)
		{
			if (p == user_id)
				continue;
			if (p == -1)
				continue;
			send_move_packet(p, user_id);
		}
	}
	else
		m_clients[user_id].m_befor_send_move = false;
	auto end_time = chrono::steady_clock::now();

	//cout << chrono::duration_cast<chrono::milliseconds>(end_time - start_time).count() << "ms" << endl;
}