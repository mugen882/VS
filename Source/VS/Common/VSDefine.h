#pragma once

inline constexpr float ADD_SHIELD_RADIUS = 50.f;
inline constexpr float MIN_COOLDOWN_TIME = 0.1f;
inline constexpr float BASE_LEVEL_XP = 5.f;	// 다음 레벨까지 필요 XP
inline constexpr float DEFAULT_HEALTH = 100.f;
inline constexpr float DEFAULT_SPEED = 600.f;
inline constexpr float ATTACK_RANGE = 100.f;
inline constexpr float MIN_SPAWN_RADIUS = 1500.f;
inline constexpr float MAX_SPAWN_RADIUS = 3000.f;
inline constexpr int32 MAX_ENEMY = 500;
inline constexpr float XP_TO_NEXTLV_MULIPLY = 1.2f;
inline constexpr float MAX_ENEMY_SCALE = 2.f;
inline constexpr float BOSS_ENEMY_SCALE = 1.8f;
inline constexpr float BOSS_SPAWN_DIST = 2000.f;
inline constexpr float BOSS_ATTACK_START_TIME = 3.f;
inline constexpr float BOSS_HEADBAR_FALLBACK_HEIGHT = 330.f;   // 메시 바운드를 못 구했을 때만 쓰는 대체 높이
inline constexpr float BOSS_MESH_RADIUS = 100.f;			// 메시 반지름 기본값
inline constexpr float MESH_YAW_OFFSET = -90.f;            // 액터 정면(+X) 대비 메시가 틀어진 각도 (데이터 기본값)
inline constexpr float BOSS_ROTATE_SPEED_DEG = 240.f;      // 보스 기본 선회 속도 (도/초)
inline constexpr float BOSS_AIM_ROTATE_SPEED_DEG = 120.f;  // 돌진 조준 중 선회 속도 (낮을수록 회피 쉬움)
inline constexpr int32 MAX_WEAPON_LEVEL = 20;	// 무기 강화 레벨 상한
inline constexpr int32 MAX_PASSIVE_LEVEL = 20;	// 패시브 스택 상한

// 패시브 누적 값 상한 (base 대비 배율. 예: 1.0 = +100%).
// 도달하면 더 찍어도 수치가 안 오르므로 업그레이드 후보에서 제외된다.
inline constexpr float MAX_MOVESPEED_BONUS      = 1.0f;   // 이동 속도 +100%
inline constexpr float MAX_MAXHEALTH_BONUS      = 3.0f;   // 최대 체력 +300%
inline constexpr float MAX_PICKUPRANGE_BONUS    = 4.0f;   // 획득 범위 +400%
inline constexpr float MAX_GLOBALDAMAGE_BONUS   = 3.0f;   // 전체 데미지 +300%
inline constexpr int32 BOSS_TARGET_INDEX = -2; // FindNearestEnemy가 "보스가 최근접 타겟"임을 알리는 특수 인덱스