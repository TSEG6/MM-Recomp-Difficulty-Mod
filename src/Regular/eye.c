#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_en_egol.h"

#include "z64olib.h"

#include "assets/objects/object_eg/object_eg.h"
#include "assets/objects/gameplay_keep/gameplay_keep.h"
#include "overlays/actors/ovl_En_Clear_Tag/z_en_clear_tag.h"
#include "overlays/actors/ovl_En_Estone/z_en_estone.h"
#include "overlays/effects/ovl_Effect_Ss_Hitmark/z_eff_ss_hitmark.h"

typedef enum {
    /* 0 */ EYEGORE_EFFECT_IMPACT,
    /* 1 */ EYEGORE_EFFECT_PIECE_LARGE,
    /* 2 */ EYEGORE_EFFECT_PIECE_SMALL,
    /* 3 */ EYEGORE_EFFECT_DEBRIS
} EnEgolEffectType;

typedef enum {
    /* 0 */ EYEGORE_LASER_OFF,
    /* 1 */ EYEGORE_LASER_START,
    /* 2 */ EYEGORE_LASER_CHARGING,
    /* 3 */ EYEGORE_LASER_FIRE,
    /* 7 */ EYEGORE_LASER_ON = 7
} EnEgolLaserState;

void EnEgol_SetupWalk(EnEgol* this);
void EnEgol_SetupSlam(EnEgol* this);
void EnEgol_SetupStunEnd(EnEgol* this);

void EnEgol_SpawnEffect(EnEgol* this, Vec3f* pos, Vec3s* rot, s16 lifetime, f32 scale, s16 type);

extern void EnEgol_DestroyBlocks(EnEgol*, PlayState*, Vec3f, Vec3f);

// Slam animation speed increases
RECOMP_HOOK_RETURN("EnEgol_SlamWait") void WaitLess(EnEgol* this) {

    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        this->skelAnime.playSpeed = 4.0f;
        break;

    case 1:
        this->skelAnime.playSpeed = 8.0f;
        break;

    default:
        break;
    }

}

// Decreases stun time
RECOMP_HOOK("EnEgol_Stunned") void StunLessPre(EnEgol* this) {

    int Difficulty = (int)recomp_get_config_double("diff_option");
    int StunMax = 80;

    switch (Difficulty) {
    case 0:
        StunMax = 60;
        break;

    case 1:
        StunMax = 30;
        break;

    default:
        break;
    }
    if (this->actionTimer > StunMax) {
        this->eyeCollider.elements[0].base.elemMaterial = ELEM_MATERIAL_UNK2;
        EnEgol_SetupStunEnd(this);
    }
}

// Pre slamp animation speed increases
RECOMP_HOOK_RETURN("EnEgol_SetupSlam") void WaitLessPre(EnEgol* this) {

    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        this->skelAnime.playSpeed = 1.5f;
        break;

    case 1:
        this->skelAnime.playSpeed = 2.0f;
        break;

    default:
        break;
    }
}

// Lazer difficulty adjustments
RECOMP_PATCH void EnEgol_Laser(EnEgol* this, PlayState* play) {
    static s16 sLaserAngles[3] = { 0x1F40, 0xBB8, 0x7D0 };
    s32 pad1;
    Vec3f stonePos;
    s32 pad2;
    CollisionPoly* colPoly;
    Vec3f hitPos;
    s32 bgId;

    int Difficulty = (int)recomp_get_config_double("diff_option");

    s16 waitTime = 8 - (Difficulty * 3);
    s16 turnSpeed = 0x250 + (Difficulty * 0x150);
    f32 laserSize = 0.04f + (Difficulty * 0.02f);
    s16 recoveryTime = 4 - Difficulty;

    if ((ABS_ALT((s16)(this->actor.world.rot.y - this->actor.yawTowardsPlayer)) < 0x3000) &&
        (this->actor.xzDistToPlayer < 100.0f)) {
        this->chargingLaser = false;
        this->chargeLevel = 0;
        this->actionTimer = 0;
        this->laserState = EYEGORE_LASER_OFF;
        Math_Vec3f_Copy(&this->laserScale, &gZeroVec3f);
        Math_Vec3f_Copy(&this->laserScaleTarget, &gZeroVec3f);
        Math_Vec3f_Copy(&this->laserCollider.dim.quad[1], &this->laserBase);
        Math_Vec3f_Copy(&this->laserCollider.dim.quad[3], &this->laserBase);
        Math_Vec3f_Copy(&this->laserCollider.dim.quad[0], &this->laserBase);
        Math_Vec3f_Copy(&this->laserCollider.dim.quad[2], &this->laserBase);
        EnEgol_SetupSlam(this);
    }
    else if (this->actor.xzDistToPlayer < this->minLaserRange) {
        this->chargingLaser = false;
        this->chargeLevel = 0;
        this->actionTimer = 0;
        this->laserState = EYEGORE_LASER_OFF;
        Math_Vec3f_Copy(&this->laserScale, &gZeroVec3f);
        Math_Vec3f_Copy(&this->laserScaleTarget, &gZeroVec3f);
        Math_Vec3f_Copy(&this->laserCollider.dim.quad[1], &this->laserBase);
        Math_Vec3f_Copy(&this->laserCollider.dim.quad[3], &this->laserBase);
        Math_Vec3f_Copy(&this->laserCollider.dim.quad[0], &this->laserBase);
        Math_Vec3f_Copy(&this->laserCollider.dim.quad[2], &this->laserBase);
        EnEgol_SetupWalk(this);
    }
    else {
        if (this->chargingLaser) {
            switch (this->chargeLevel) {
            case 0:
                this->waitTimer = waitTime;
                this->chargeLevel++;
                break;

            case 1:
                Math_ApproachF(&this->chargeLightScale, 8.0f, 0.5f, 2.5f);
                if (this->waitTimer == 0) {
                    this->waitTimer = waitTime;
                    this->chargeLevel++;
                }
                break;

            case 2:
                Math_ApproachF(&this->chargeLightScale, 1.0f, 0.5f, 1.0f);
                if (this->waitTimer == 0) {
                    this->chargeLevel = 0;
                    this->laserState = EYEGORE_LASER_FIRE;
                    this->chargingLaser = false;
                    this->chargeLightScale = 0.0f;
                }
                break;

            default:
                break;
            }
            this->chargeLightRot += 0x7D0;
        }
        if ((this->laserState != EYEGORE_LASER_OFF) || (this->laserCount != 0)) {
            Math_SmoothStepToS(&this->headRot, -0x2710, 5, turnSpeed, 5);
        }
        else {
            Math_SmoothStepToS(&this->headRot, 0, 5, turnSpeed, 5);
        }
        if (this->laserState == EYEGORE_LASER_OFF) {
            if (this->laserCount >= 3) {
                EnEgol_SetupWalk(this);
            }
            else {
                this->laserRot.x = sLaserAngles[this->laserCount];
                this->laserScaleTarget.z = 0.0f;
                this->laserScale.z = 0.0f;
                if (this->laserCount == 0) {
                    this->laserState = EYEGORE_LASER_START;
                }
                else {
                    this->laserState = EYEGORE_LASER_FIRE;
                    this->laserScaleTarget.x = laserSize;
                    this->laserScaleTarget.y = laserSize;
                }
                this->laserCount++;
            }
        }
        else if (this->laserState >= EYEGORE_LASER_FIRE) {
            Actor_PlaySfx(&this->actor, NA_SE_EN_EYEGOLE_BEAM - SFX_FLAG);
            if (this->laserState != EYEGORE_LASER_OFF) {
                EnEgol_DestroyBlocks(this, play, this->laserCollider.dim.quad[0], this->laserCollider.dim.quad[1]);
            }
            if ((this->actionTimer == 0) && BgCheck_EntityLineTest1(&play->colCtx, &this->laserCollider.dim.quad[3],
                &this->laserCollider.dim.quad[1], &hitPos, &colPoly,
                true, true, false, true, &bgId)) {
                Vec3s rotToNorm;
                f32 nx;
                f32 ny;
                f32 nz;
                s32 pad3;
                s32 i;
                s32 quakeYOffset;
                Player* player = GET_PLAYER(play);

                nx = COLPOLY_GET_NORMAL(colPoly->normal.x);
                ny = COLPOLY_GET_NORMAL(colPoly->normal.y);
                nz = COLPOLY_GET_NORMAL(colPoly->normal.z);

                rotToNorm.x = RAD_TO_BINANG(Math_FAtan2F(nz, ny));
                rotToNorm.z = RAD_TO_BINANG(Math_FAtan2F(-nx, sqrtf(1.0f - SQ(nx))));

                if ((this->actor.world.pos.y - 50.0f) <= player->actor.world.pos.y) {
                    EnEgol_SpawnEffect(this, &hitPos, &rotToNorm, 100, 0.02f, EYEGORE_EFFECT_IMPACT);
                }
                quakeYOffset = 4 - (s32)(fabsf(player->actor.world.pos.y - this->actor.world.pos.y) * 0.02f);
                if (quakeYOffset > 4) {
                    quakeYOffset = 4;
                }
                else if (quakeYOffset < 1) {
                    quakeYOffset = 1;
                }
                if (player->stateFlags3 != PLAYER_STATE3_1000000) {
                    Actor_RequestQuakeAndRumble(&this->actor, play, quakeYOffset, 2);
                }
                Actor_PlaySfx(&this->actor, NA_SE_EV_EXPLOSION);
                func_800B31BC(play, &hitPos, 40, -2, 255, 20);
                func_800BBFB0(play, &hitPos, 6.0f, 2, 120, 20, true);

                if ((this->actor.world.pos.y - 50.0f <= player->actor.world.pos.y) &&
                    (this->actor.floorBgId == BGCHECK_SCENE)) {
                    Math_Vec3f_Copy(&stonePos, &hitPos);
                    stonePos.x += Math_SinS(this->actor.world.rot.y) * 60.0f;
                    stonePos.z += Math_CosS(this->actor.world.rot.y) * 60.0f;
                    for (i = 0; i < 3; i++) {
                        Actor_Spawn(&play->actorCtx, play, ACTOR_EN_ESTONE, stonePos.x, stonePos.y, stonePos.z, 0,
                            this->actor.world.rot.y, 0, ENESTONE_TYPE_SMALL);
                    }
                }
                if (this->actor.world.pos.y - 50.0f <= player->actor.world.pos.y) {
                    for (i = 0; i < 10; i++) {
                        EnEgol_SpawnEffect(this, &hitPos, &gZeroVec3s, 30, (Rand_ZeroFloat(1.0f) * 0.1f) + 0.2f,
                            EYEGORE_EFFECT_DEBRIS);
                    }
                }
                this->actionTimer = 1;
            }
            if (this->actionTimer != 0) {
                this->actionTimer++;
                if (this->actionTimer >= recoveryTime) {
                    this->laserState = EYEGORE_LASER_OFF;
                    Math_Vec3f_Copy(&this->laserScale, &gZeroVec3f);
                    Math_Vec3f_Copy(&this->laserScaleTarget, &gZeroVec3f);
                    Math_Vec3f_Copy(&this->laserCollider.dim.quad[1], &this->laserBase);
                    Math_Vec3f_Copy(&this->laserCollider.dim.quad[3], &this->laserBase);
                    Math_Vec3f_Copy(&this->laserCollider.dim.quad[0], &this->laserBase);
                    Math_Vec3f_Copy(&this->laserCollider.dim.quad[2], &this->laserBase);
                    this->actionTimer = 0;
                }
            }
            Math_ApproachF(&this->laserScale.x, this->laserScaleTarget.x, 0.5f, 0.5f);
            Math_ApproachF(&this->laserScale.y, this->laserScaleTarget.y, 0.5f, 0.5f);
            Math_ApproachF(&this->laserScale.z, this->laserScaleTarget.z, 0.5f, 0.5f);
        }
    }
}

// Increases defense on Hard
RECOMP_HOOK("EnEgol_Update") void EyegoreUpdate(Actor* thisx, PlayState* play) {
	EnEgol* this = (EnEgol*)thisx;

    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        break;

    case 1:
        this->actor.colChkInfo.damage = (this->actor.colChkInfo.damage) / 2;
        break;

    default:
        break;
    }
}