#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_en_rd.h"
#include "z64rumble.h"
#include "attributes.h"
#include "assets/objects/object_rd/object_rd.h"
#include "overlays/actors/ovl_Obj_Ice_Poly/z_obj_ice_poly.h"

void EnRd_WalkToPlayer(EnRd* this, PlayState* play);
void EnRd_SetupWalkToHome(EnRd* this, PlayState* play);
void EnRd_SetupWalkToParent(EnRd* this);
void EnRd_SetupGrab(EnRd* this);
void EnRd_SetupAttemptPlayerFreeze(EnRd* this);
void EnRd_SetupGrabFail(EnRd* this);

extern void EnRd_SetupWalkToPlayer(EnRd*, PlayState*);

static bool AnyRedeadAttacked = false;

RECOMP_HOOK("Play_Init") void Play_Init_Hook(PlayState* play) {
    AnyRedeadAttacked = false;
}

RECOMP_HOOK("EnRd_Damage") void DamageRD(EnRd* this, PlayState* play) {
    AnyRedeadAttacked = true;
}

// On Hard redeads do not dance & if any were attacked on Normal dancing will be disabled
RECOMP_PATCH s32 EnRd_ShouldNotDance(PlayState* play) {
    int Difficulty = (int)recomp_get_config_double("diff_option");

    if (Difficulty == 1) {
        return true;
    }

    if (Difficulty == 0 && AnyRedeadAttacked) {
        return true;
    }

    if ((Player_GetMask(play) == PLAYER_MASK_GIBDO) ||
        (Player_GetMask(play) == PLAYER_MASK_CAPTAIN) ||
        (Player_GetMask(play) == PLAYER_MASK_GARO)) {
        return false;
    }
    return true;
}

// Increases walking movement speed
RECOMP_PATCH void EnRd_WalkToPlayer(EnRd* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    s32 pad;
    s16 yaw = this->actor.yawTowardsPlayer - this->actor.shape.rot.y - this->headRotY - this->torsoRotY;

    int Difficulty = (int)recomp_get_config_double("diff_option");
    float speedMultiplier = 1.0f;

    switch (Difficulty) {
    case 0:
        speedMultiplier = 1.33f;
        break;

    case 1:
        speedMultiplier = 2.0f;
        break;

    default:
        break;
    }

    this->actor.speed = speedMultiplier;
    this->skelAnime.playSpeed = (speedMultiplier * 1.5f);
    Math_SmoothStepToS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 1, (s16)(0xFA * speedMultiplier), 0);
    Math_SmoothStepToS(&this->headRotY, 0, 1, 0x64, 0);
    Math_SmoothStepToS(&this->torsoRotY, 0, 1, 0x64, 0);
    this->actor.world.rot.y = this->actor.shape.rot.y;
    SkelAnime_Update(&this->skelAnime);

    if (Actor_WorldDistXYZToPoint(&player->actor, &this->actor.home.pos) >= 150.0f) {
        this->actor.speed = 1.0f;
        this->skelAnime.playSpeed = 1.0f;
        EnRd_SetupWalkToHome(this, play);
        return;
    }

    if ((ABS_ALT(yaw) < 0x1554) && (Actor_WorldDistXYZToActor(&this->actor, &player->actor) <= 150.0f)) {
        if (!(player->stateFlags1 & (PLAYER_STATE1_DEAD | PLAYER_STATE1_2000 | PLAYER_STATE1_4000 |
            PLAYER_STATE1_40000 | PLAYER_STATE1_80000 | PLAYER_STATE1_200000)) &&
            !(player->stateFlags2 & (PLAYER_STATE2_80 | PLAYER_STATE2_4000))) {
            if (this->playerStunWaitTimer == 0) {
                if (!(this->flags & EN_RD_FLAG_CANNOT_FREEZE_PLAYER)) {
                    player->actor.freezeTimer = 40;
                    Player_SetAutoLockOnActor(play, &this->actor);
                    GET_PLAYER(play)->autoLockOnActor = &this->actor;
                    Rumble_Request(this->actor.xzDistToPlayer, 255, 20, 150);
                }
                this->playerStunWaitTimer = 60;
                Actor_PlaySfx(&this->actor, NA_SE_EN_REDEAD_AIM);
            }
        }
        else {
            this->actor.speed = 1.0f;
            this->skelAnime.playSpeed = 1.0f;
            EnRd_SetupWalkToHome(this, play);
            return;
        }
    }

    if (this->grabWaitTimer != 0) {
        this->grabWaitTimer--;
    }

    if (!this->grabWaitTimer && (Actor_WorldDistXYZToActor(&this->actor, &player->actor) <= 45.0f) &&
        Actor_IsFacingPlayer(&this->actor, 0x38E3)) {
        player->actor.freezeTimer = 0;
        if ((player->transformation == PLAYER_FORM_GORON) || (player->transformation == PLAYER_FORM_DEKU)) {
            if (Actor_WorldDistXYZToPoint(&this->actor, &this->actor.home.pos) < 150.0f) {
                this->actor.speed = 1.0f;
                this->skelAnime.playSpeed = 1.0f;
                EnRd_SetupGrabFail(this);
                return;
            }
            else {
                this->actor.speed = 1.0f;
                this->skelAnime.playSpeed = 1.0f;
                EnRd_SetupWalkToHome(this, play);
                return;
            }
        }
        else if (play->grabPlayer(play, player)) {
            this->actor.flags &= ~ACTOR_FLAG_ATTENTION_ENABLED;
            this->actor.speed = 1.0f;
            this->skelAnime.playSpeed = 1.0f;
            EnRd_SetupGrab(this);
            return;
        }
    }
    else if (EN_RD_GET_TYPE(&this->actor) > EN_RD_TYPE_DOES_NOT_MOURN_IF_WALKING) {
        if (this->actor.parent != NULL) {
            this->actor.speed = 1.0f;
            this->skelAnime.playSpeed = 1.0f;
            EnRd_SetupWalkToParent(this);
            return;
        }
        else {
            this->isMourning = false;
        }
    }

    if (Animation_OnFrame(&this->skelAnime, 10.0f) || Animation_OnFrame(&this->skelAnime, 22.0f)) {
        Actor_PlaySfx(&this->actor, NA_SE_EN_RIZA_WALK);
    }
    else if ((play->gameplayFrames & 0x5F) == 0) {
        Actor_PlaySfx(&this->actor, NA_SE_EN_REDEAD_CRY);
    }
}

// Player freezing adjustments
RECOMP_PATCH void EnRd_AttemptPlayerFreeze(EnRd* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    s16 yaw = this->actor.yawTowardsPlayer - this->actor.shape.rot.y - this->headRotY - this->torsoRotY;
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
        if (!(this->flags & EN_RD_FLAG_CANNOT_FREEZE_PLAYER)) {
            player->actor.freezeTimer = freezeDuration;
            Rumble_Request(this->actor.xzDistToPlayer, 255, 20, 150);
            Player_SetAutoLockOnActor(play, &this->actor);
        }
        Actor_PlaySfx(&this->actor, NA_SE_EN_REDEAD_AIM);
        EnRd_SetupWalkToPlayer(this, play);
    }
}

// Defense & disables HUD when the player is frozen
RECOMP_HOOK("EnRd_Update") void RDUpdate(Actor* thisx, PlayState* play) {
    Player* player = GET_PLAYER(play);

    EnRd* this = (EnRd*)thisx;

    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        this->actor.colChkInfo.damage = (this->actor.colChkInfo.damage) / 2;
        break;

    case 1:
        this->actor.colChkInfo.damage = (this->actor.colChkInfo.damage + 2) / 3;
        break;

    default:
        break;
    }

    if (player->actor.freezeTimer > 0) {

        Interface_SetHudVisibility(HUD_VISIBILITY_NONE);
    }
    else {

        Interface_SetHudVisibility(HUD_VISIBILITY_ALL);
    }
}