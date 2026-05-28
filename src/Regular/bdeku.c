#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_boss_05.h"
#include "attributes.h"

void Boss05_WalkingHead_SetupIdle(Boss05* this, PlayState* play);
void Boss05_WalkingHead_SetupSpottedPlayer(Boss05* this, PlayState* play);
void Boss05_WalkingHead_SpottedPlayer(Boss05* this, PlayState* play);
void Boss05_WalkingHead_SetupCharge(Boss05* this, PlayState* arg1);

#define TIMER_CURRENT_ACTION 0

RECOMP_PATCH void Boss05_WalkingHead_SetupSpottedPlayer(Boss05* this, PlayState* play) {
    this->actionFunc = Boss05_WalkingHead_SpottedPlayer;
    Animation_MorphToPlayOnce(&this->headSkelAnime, &gBioDekuBabaHeadSpotAnim, 0.0f);
    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        this->timers[TIMER_CURRENT_ACTION] = 10;
        break;

    case 1:
        this->timers[TIMER_CURRENT_ACTION] = 5;
        break;

    default:
        break;
    }
}

s32 Boss05_WalkingHead_IsLookingAtPlayer(Boss05* this, PlayState* play) {
    s16 yawDiff = this->dyna.actor.yawTowardsPlayer - this->dyna.actor.shape.rot.y;

    if (ABS_ALT(yawDiff) < 0x3000) {
        return true;
    }
    else {
        return false;
    }
}

void Boss05_WalkingHead_TrySpottingPlayer(Boss05* this, PlayState* play) {
    if (Boss05_WalkingHead_IsLookingAtPlayer(this, play) && (this->dyna.actor.xyzDistToPlayerSq <= SQ(200.0f)) &&
        (fabsf(this->dyna.actor.playerHeightRel) < 70.0f)) {
        Boss05_WalkingHead_SetupSpottedPlayer(this, play);
    }
}

RECOMP_PATCH void Boss05_WalkingHead_Walk(Boss05* this, PlayState* play) {
    f32 diffX;
    f32 diffZ;
    s16 targetYaw;
    f32 stopDistance = 50.0f;
    int difficulty = (int)recomp_get_config_double("diff_option");

    if (difficulty == 1) {
        Player* player = GET_PLAYER(play);
        this->walkTargetPos.x = player->actor.world.pos.x;
        this->walkTargetPos.z = player->actor.world.pos.z;
        stopDistance = 40.0f;
    }

    Actor_PlaySfx(&this->dyna.actor, NA_SE_EN_MIZUBABA2_WALK - SFX_FLAG);
    SkelAnime_Update(&this->headSkelAnime);
    Math_ApproachF(&this->dyna.actor.speed, 5.0f, 1.0f, 2.0f);

    diffX = this->walkTargetPos.x - this->dyna.actor.world.pos.x;
    diffZ = this->walkTargetPos.z - this->dyna.actor.world.pos.z;

    targetYaw = Math_Atan2S(diffX, diffZ);

    if (this->dyna.actor.bgCheckFlags & BGCHECKFLAG_WALL) {
        targetYaw = this->dyna.actor.wallYaw;
    }

    Math_ApproachS(&this->dyna.actor.world.rot.y, targetYaw, 5, this->walkAngularVelocityY);
    Math_ApproachF(&this->walkAngularVelocityY, 2000.0f, 1.0f, 100.0f);

    if ((this->timers[TIMER_CURRENT_ACTION] == 0) || ((SQ(diffX) + SQ(diffZ)) < SQ(stopDistance))) {
        Boss05_WalkingHead_SetupIdle(this, play);
    }

    Boss05_WalkingHead_TrySpottingPlayer(this, play);
}

RECOMP_HOOK("Boss05_Update") void BBabaUpdate(Actor* thisx, PlayState* play) {

	Boss05* this = (Boss05*)thisx;
	int Difficulty = (int)recomp_get_config_double("diff_option");
    float speedMultiplier = 1.0f;

    switch (Difficulty) {
    case 0:
        speedMultiplier = 1.5f;
        this->dyna.actor.colChkInfo.damage = (this->dyna.actor.colChkInfo.damage) / 2;
        break;

    case 1:
        speedMultiplier = 2.5f;
        this->dyna.actor.colChkInfo.damage = (this->dyna.actor.colChkInfo.damage + 2) / 3;
        break;

    default:
        break;
    }

    if (this->dyna.actor.colChkInfo.health != 0 && this->actionFunc != Boss05_WalkingHead_SpottedPlayer && this->actionFunc != Boss05_WalkingHead_SetupCharge) {
        this->headSkelAnime.playSpeed = speedMultiplier;
    }

    if (this->dyna.actor.speed <= 6.0f && this->actionFunc != Boss05_WalkingHead_SpottedPlayer && this->actionFunc != Boss05_WalkingHead_SetupCharge) {
        this->dyna.actor.speed *= speedMultiplier;
    }

    if (this->dyna.actor.colChkInfo.health == 0) this->dyna.actor.speed = 0;

}