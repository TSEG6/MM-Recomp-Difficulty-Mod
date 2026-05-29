#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_en_railgibud.h"
#include "z64rumble.h"
#include "overlays/effects/ovl_Effect_Ss_Hahen/z_eff_ss_hahen.h"

void EnRailgibud_SetupAttemptPlayerFreeze(EnRailgibud* this);
void EnRailgibud_SetupWalkToPlayer(EnRailgibud* this);
void EnRailgibud_WalkToPlayer(EnRailgibud* this, PlayState* play);
void EnRailgibud_TurnTowardsPlayer(EnRailgibud* this, PlayState* play);

RECOMP_PATCH void EnRailgibud_AttemptPlayerFreeze(EnRailgibud* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    s16 rot = this->actor.shape.rot.y + this->headRot.y + this->torsoRot.y;
    s16 yaw = BINANG_SUB(this->actor.yawTowardsPlayer, rot);
    s16 visionCone = 0x2008;
    s16 freezeDuration = 60;

    int Difficulty = (int)recomp_get_config_double("diff_option");

    if (Difficulty == 0) {
        visionCone = 0x2800;
        freezeDuration = 70;
    }
    else if (Difficulty >= 1) {
        visionCone = 0x3000;
        freezeDuration = 80;
    }

    if (ABS_ALT(yaw) < visionCone) {
        player->actor.freezeTimer = freezeDuration;
        Rumble_Request(this->actor.xzDistToPlayer, 255, 20, 150);
        Player_SetAutoLockOnActor(play, &this->actor);
        Actor_PlaySfx(&this->actor, NA_SE_EN_REDEAD_AIM);
        EnRailgibud_SetupWalkToPlayer(this);
    }

    EnRailgibud_TurnTowardsPlayer(this, play);
}

RECOMP_HOOK("EnRailgibud_Damage") void FunnyDamageGibdoMBH(EnRailgibud* this, PlayState* play) {

    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        if (Rand_ZeroOne() < 0.002f) EnRailgibud_SetupAttemptPlayerFreeze(this);
        break;

    case 1:
        if (Rand_ZeroOne() < 0.02f) EnRailgibud_SetupAttemptPlayerFreeze(this);
        break;

    default:
        break;
    }
}

RECOMP_HOOK("EnRailgibud_WalkInCircles") void WalkingMBHG(EnRailgibud* this, PlayState* play) {

    int Difficulty = (int)recomp_get_config_double("diff_option");
    float freezeDist = 100.0f;

    switch (Difficulty) {
    case 0:
        freezeDist = 150.0f;
        break;

    case 1:
        freezeDist = 175.0;
        break;

    default:
        break;
    }

    if ((this->actor.xzDistToPlayer <= freezeDist) && func_800B715C(play) && (Player_GetMask(play) != PLAYER_MASK_GIBDO)) {
        this->actor.home = this->actor.world;
        EnRailgibud_SetupAttemptPlayerFreeze(this);

    }
}

RECOMP_HOOK("EnRailgibud_Update") void GibdoMBHUpdate(Actor* thisx, PlayState* play) {
    Player* player = GET_PLAYER(play);

    EnRailgibud* this = (EnRailgibud*)thisx;

    int Difficulty = (int)recomp_get_config_double("diff_option");
    float speedMultiplier = 1.0f;

    switch (Difficulty) {
    case 0:
        speedMultiplier = 1.25f;
        this->actor.colChkInfo.damage = (this->actor.colChkInfo.damage) / 2;
        break;

    case 1:
        if (this->actionFunc == EnRailgibud_WalkToPlayer) {
            speedMultiplier = 2.0f;
        }
        else {
            speedMultiplier = 1.5f;
        }
        this->actor.colChkInfo.damage = (this->actor.colChkInfo.damage + 2) / 3;
        break;

    default:
        break;
    }

    if (play->csCtx.state != 0) {
        speedMultiplier = 1.0f;
    }

    if (this->actor.parent != NULL) {

        if (this->actor.colChkInfo.health != 0 &&
            (this->actionFunc == EnRailgibud_WalkToPlayer)) {
            this->skelAnime.playSpeed = speedMultiplier;
        }

        if (this->actor.speed <= 3.0f &&
            (this->actionFunc == EnRailgibud_WalkToPlayer)) {
            this->actor.speed *= speedMultiplier;
        }
        if (this->actor.colChkInfo.health == 0) this->actor.speed = 0;
    }

    if (player->actor.freezeTimer > 0) {

        Interface_SetHudVisibility(HUD_VISIBILITY_NONE);
    }
    else {

        Interface_SetHudVisibility(HUD_VISIBILITY_ALL);
    }
}