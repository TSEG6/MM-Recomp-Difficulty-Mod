#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_en_mkk.h"
#include "assets/objects/object_mkk/object_mkk.h"

void func_80A4E58C(EnMkk* this);

// Spawns in friends based on difficulty (1 on normal and 2 on hard)
RECOMP_HOOK("EnMkk_Init") void YoureSeeingDouble(Actor* thisx, PlayState* play) {
    EnMkk* this = (EnMkk*)thisx;

    if (this->actor.params & 0x8000) {
        return;
    }

    int Difficulty = (int)recomp_get_config_double("diff_option");

    s16 spawnParams = this->actor.params | 0x8000;

    switch (Difficulty) {
    case 0:
        for (int i = 0; i < 1; i++) {
            Actor_Spawn(&play->actorCtx, play, ACTOR_EN_MKK,
                this->actor.world.pos.x, this->actor.world.pos.y, this->actor.world.pos.z,
                0, 0, 0, spawnParams);
        }
        break;

    case 1:
        for (int i = 0; i < 2; i++) {
            Actor_Spawn(&play->actorCtx, play, ACTOR_EN_MKK,
                this->actor.world.pos.x, this->actor.world.pos.y, this->actor.world.pos.z,
                0, 0, 0, spawnParams);
        }
        break;

    default:
        break;
    }
}

// Increases speed and randomly enables the hitbox
RECOMP_HOOK("EnMkk_Update") void BoeUpdate(Actor* thisx, PlayState* play) {
    EnMkk* this = (EnMkk*)thisx;

    int Difficulty = (int)recomp_get_config_double("diff_option");
    float speedMultiplier = 1.0f;

    switch (Difficulty) {
    case 0:
        speedMultiplier = 1.25f;
        if (Rand_ZeroOne() < 0.02f) this->collider.base.atFlags |= AT_ON;
        else this->collider.base.atFlags &= ~AT_ON;
        break;
    case 1:
        speedMultiplier = 1.75f;
        if (Rand_ZeroOne() < 0.075f) this->collider.base.atFlags |= AT_ON;
        else this->collider.base.atFlags &= ~AT_ON;
        break;
    default:
        break;
    }

    if (this->actor.speed <= 6.0f) {
        this->actor.speed *= speedMultiplier;
    }
}

// Jumping adjustments
RECOMP_HOOK("func_80A4E2E8") void JumpMoreBoe(EnMkk* this, PlayState* play) {

    Player* player = GET_PLAYER(play);
    s32 sp20;

    if ((this->unk_149 == 0) && !(player->stateFlags3 & PLAYER_STATE3_100) &&
        (Player_GetMask(play) != PLAYER_MASK_STONE) && (this->actor.bgCheckFlags & BGCHECKFLAG_GROUND) &&
        Actor_IsFacingPlayer(&this->actor, 0x1800) &&
        (fabsf(this->actor.playerHeightRel) < 100.0f)) {
        func_80A4E58C(this);
    }
}

// Speed increases
RECOMP_HOOK("func_80A4E58C") void JumpSpeedBoe(EnMkk* this) {

    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        this->actor.speed = 5.0f;
        this->actor.velocity.y = 7.0f;
        break;

    case 1:
        this->actor.speed = 7.0f;
        this->actor.velocity.y = 9.0f;
        break;

    default:
        break;
    }
}