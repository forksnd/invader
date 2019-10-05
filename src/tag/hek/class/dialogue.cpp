// SPDX-License-Identifier: GPL-3.0-only

#include "../compile.hpp"
#include "dialogue.hpp"

namespace Invader::HEK {
    void compile_dialogue_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(Dialogue)
        ADD_DEPENDENCY_ADJUST_SIZES(tag.idle_noncombat);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.idle_combat);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.idle_flee);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.pain_body_minor);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.pain_body_major);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.pain_shield);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.pain_falling);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.scream_fear);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.scream_pain);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.maimed_limb);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.maimed_head);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.death_quiet);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.death_violent);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.death_falling);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.death_agonizing);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.death_instant);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.death_flying);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.damaged_friend);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.damaged_friend_player);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.damaged_enemy);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.damaged_enemy_cm);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.hurt_friend);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.hurt_friend_re);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.hurt_friend_player);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.hurt_enemy);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.hurt_enemy_re);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.hurt_enemy_cm);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.hurt_enemy_bullet);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.hurt_enemy_needler);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.hurt_enemy_plasma);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.hurt_enemy_sniper);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.hurt_enemy_grenade);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.hurt_enemy_explosion);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.hurt_enemy_melee);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.hurt_enemy_flame);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.hurt_enemy_shotgun);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.hurt_enemy_vehicle);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.hurt_enemy_mountedweapon);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.killed_friend);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.killed_friend_cm);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.killed_friend_player);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.killed_friend_player_cm);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.killed_enemy);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.killed_enemy_cm);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.killed_enemy_player);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.killed_enemy_player_cm);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.killed_enemy_covenant);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.killed_enemy_covenant_cm);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.killed_enemy_floodcombat);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.killed_enemy_floodcombat_cm);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.killed_enemy_floodcarrier);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.killed_enemy_floodcarrier_cm);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.killed_enemy_sentinel);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.killed_enemy_sentinel_cm);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.killed_enemy_bullet);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.killed_enemy_needler);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.killed_enemy_plasma);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.killed_enemy_sniper);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.killed_enemy_grenade);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.killed_enemy_explosion);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.killed_enemy_melee);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.killed_enemy_flame);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.killed_enemy_shotgun);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.killed_enemy_vehicle);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.killed_enemy_mountedweapon);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.killing_spree);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.player_kill_cm);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.player_kill_bullet_cm);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.player_kill_needler_cm);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.player_kill_plasma_cm);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.player_kill_sniper_cm);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.anyone_kill_grenade_cm);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.player_kill_explosion_cm);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.player_kill_melee_cm);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.player_kill_flame_cm);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.player_kill_shotgun_cm);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.player_kill_vehicle_cm);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.player_kill_mountedweapon_cm);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.player_killling_spree_cm);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.friend_died);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.friend_player_died);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.friend_killed_by_friend);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.friend_killed_by_friendly_player);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.friend_killed_by_enemy);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.friend_killed_by_enemy_player);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.friend_killed_by_covenant);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.friend_killed_by_flood);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.friend_killed_by_sentinel);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.friend_betrayed);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.new_combat_alone);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.new_enemy_recent_combat);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.old_enemy_sighted);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.unexpected_enemy);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.dead_friend_found);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.alliance_broken);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.alliance_reformed);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.grenade_throwing);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.grenade_sighted);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.grenade_startle);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.grenade_danger_enemy);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.grenade_danger_self);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.grenade_danger_friend);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.new_combat_group_re);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.new_combat_nearby_re);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.alert_friend);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.alert_friend_re);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.alert_lost_contact);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.alert_lost_contact_re);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.blocked);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.blocked_re);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.search_start);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.search_query);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.search_query_re);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.search_report);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.search_abandon);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.search_group_abandon);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.group_uncover);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.group_uncover_re);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.advance);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.advance_re);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.retreat);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.retreat_re);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.cover);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.sighted_friend_player);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.shooting);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.shooting_vehicle);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.shooting_berserk);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.shooting_group);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.shooting_traitor);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.taunt);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.taunt_re);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.flee);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.flee_re);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.flee_leader_died);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.attempted_flee);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.attempted_flee_re);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.lost_contact);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.hiding_finished);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.vehicle_entry);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.vehicle_exit);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.vehicle_woohoo);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.vehicle_scared);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.vehicle_collision);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.partially_sighted);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.nothing_there);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.pleading);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.surprise);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.berserk);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.melee_attack);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.dive);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.uncover_exclamation);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.leap_attack);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.resurrection);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.celebration);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.check_body_enemy);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.check_body_friend);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.shooting_dead_enemy);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.shooting_dead_enemy_player);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.alone);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.unscathed);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.seriously_wounded);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.seriously_wounded_re);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.massacre);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.massacre_re);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.rout);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.rout_re);
        FINISH_COMPILE
    }
}
