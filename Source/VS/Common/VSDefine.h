#pragma once

const float ADD_SHIELD_RADIUS = 50.f;
const float MIN_COOLDOWN_TIME = 0.1f;
const float BASE_LEVEL_XP = 5.f;	// 다음 레벨까지 필요 XP
const float DEFAULT_HEALTH = 100.f;
const float DEFAULT_SPEED = 600.f;
const float ATTACK_RANGE = 100.f;
const float MIN_SPAWN_RADIUS = 1500.f;
const float MAX_SPAWN_RADIUS = 3000.f;
const int32 MAX_ENEMY = 500.f;
const float XP_TO_NEXTLV_MULIPLY = 1.2f;
const float BOSS_ENEMY_SCALE = 1.8f;
const float BOSS_SPAWN_DIST = 2000.f;
const float BOSS_ATTACK_START_TIME = 3.f;
const float BOSS_HEADBAR_FALLBACK_HEIGHT = 330.f;   // 메시 바운드를 못 구했을 때만 쓰는 대체 높이
const float BOSS_MESH_RADIUS = 100.f;			// 메시 반지름 기본값
const float MESH_YAW_OFFSET = -90.f;            // 액터 정면(+X) 대비 메시가 틀어진 각도 (데이터 기본값)
const float BOSS_ROTATE_SPEED_DEG = 240.f;      // 보스 기본 선회 속도 (도/초)
const float BOSS_AIM_ROTATE_SPEED_DEG = 120.f;  // 돌진 조준 중 선회 속도 (낮을수록 회피 쉬움)
const int32 MAX_WEAPON_LEVEL = 20;	// 무기 강화 레벨 상한
const int32 MAX_PASSIVE_LEVEL = 20;	// 패시브 스택 상한