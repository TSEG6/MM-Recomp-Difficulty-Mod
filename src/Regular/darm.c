#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_en_famos.h"
#include "z64rumble.h"
#include "overlays/actors/ovl_En_Bom/z_en_bom.h"
#include "assets/objects/gameplay_keep/gameplay_keep.h"

void EnFamos_SetupAttackAim(EnFamos* this);
void EnFamos_SetupScanForPlayer(EnFamos* this);
void EnFamos_SetupFinishAttack(EnFamos* this);
void EnFamos_SetupAttackRebound(EnFamos* this);
void EnFamos_SetupDeathSlam(EnFamos* this);

typedef enum {
    /* 0 */ FAMOS_ANIMATED_MAT_NORMAL, // normal is greenish
    /* 1 */ FAMOS_ANIMATED_MAT_FLIPPED // flipped is orange/yellowish
} FamosAnimatedMatArrayIndexes;

extern void EnFamos_UpdateBobbingHeight(EnFamos*);
extern void EnFamos_SetupAttackDebris(EnFamos*);

RECOMP_PATCH void EnFamos_Chase(EnFamos* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    Vec3f targetPos;
    FloorProperty surfaceType;
    s16 targetYaw;
    float predictionFrames = 6.9f;
    float targetSpeed = 6.0f;
    float speedStep = 0.5f;

    int Difficulty = (int)recomp_get_config_double("diff_option");

    EnFamos_UpdateBobbingHeight(this);

    switch (Difficulty) {
    case 0:
        predictionFrames = 15.0f;
        targetSpeed = 6.0f;
        speedStep = 0.5f;
        break;

    case 1:
        predictionFrames = this->actor.xzDistToPlayer / 6.0f;

        if (predictionFrames > 30.0f) {
            predictionFrames = 30.0f;
        }
        targetSpeed = 9.0f;
        speedStep = 0.75f;
        break;

    default:
        break;
    }

    targetPos.x = player->actor.world.pos.x + (player->actor.velocity.x * predictionFrames);
    targetPos.y = player->actor.world.pos.y + 100.0f + (player->actor.velocity.y * predictionFrames);
    targetPos.z = player->actor.world.pos.z + (player->actor.velocity.z * predictionFrames);
    targetYaw = Math_Vec3f_Yaw(&this->actor.world.pos, &targetPos);

    Math_ScaledStepToS(&this->actor.shape.rot.y, targetYaw, 0x800);
    this->actor.world.rot.y = this->actor.shape.rot.y;
    this->actor.world.rot.x = -Actor_WorldPitchTowardPoint(&this->actor, &targetPos);
    Math_StepToF(&this->actor.speed, targetSpeed, speedStep);

    surfaceType = SurfaceType_GetFloorProperty2(&play->colCtx, this->actor.floorPoly, this->actor.floorBgId);
    if ((this->actor.xzDistToPlayer < 30.0f) && (this->actor.floorHeight > BGCHECK_Y_MIN) &&
        ((surfaceType != FLOOR_PROPERTY_12) && (surfaceType != FLOOR_PROPERTY_13))) {
        EnFamos_SetupAttackAim(this);
    }
    else if ((Player_GetMask(play) == PLAYER_MASK_STONE) ||
        (this->aggroDistance < Actor_WorldDistXZToPoint(&GET_PLAYER(play)->actor, &this->calmPos)) ||
        !Actor_IsFacingPlayer(&this->actor, 0x6000)) {
        EnFamos_SetupScanForPlayer(this);
    }
}

RECOMP_PATCH void EnFamos_Attack(EnFamos* this, PlayState* play) {
    s32 hitFloor;
    u32 surfaceType;
    f32 speedMultiplier = 1;
    Player* player = GET_PLAYER(play);
    Vec3f targetPos;
    s16 targetYaw;
    float predictionFrames = 6.9f; //nice

    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        speedMultiplier = 1.5f;
        predictionFrames = 15.0f;
        break;

    case 1:
        speedMultiplier = 2.0f;
        predictionFrames = this->actor.xzDistToPlayer / (20.0f * speedMultiplier);
        if (predictionFrames > 30.0f) {
            predictionFrames = 30.0f;
        }
        break;

    default:
        break;
    }

    targetPos.x = player->actor.world.pos.x + (player->actor.velocity.x * predictionFrames);
    targetPos.z = player->actor.world.pos.z + (player->actor.velocity.z * predictionFrames);
    targetYaw = Math_Vec3f_Yaw(&this->actor.world.pos, &targetPos);

    Math_ScaledStepToS(&this->actor.shape.rot.y, targetYaw, 0x800);
    this->actor.world.rot.y = this->actor.shape.rot.y;
    Math_StepToF(&this->actor.speed, 20.0f * speedMultiplier, 2.0f * speedMultiplier);

    this->stateTimer--;
    if (this->stateTimer == 0) {
        this->emblemCollider.base.acFlags &= ~AC_ON;
    }

    surfaceType = SurfaceType_GetFloorProperty2(&play->colCtx, this->actor.floorPoly, this->actor.floorBgId);
    hitFloor = this->actor.bgCheckFlags & BGCHECKFLAG_GROUND;

    if (hitFloor || (this->actor.floorHeight == BGCHECK_Y_MIN) || (surfaceType == FLOOR_PROPERTY_12) ||
        (surfaceType == FLOOR_PROPERTY_13)) {
        this->collider1.base.atFlags &= ~AT_ON;
        this->collider2.base.atFlags |= AT_ON;

        if (hitFloor) {
            Camera_AddQuake(GET_ACTIVE_CAM(play), 2, 15, 10);
            Rumble_Request(this->actor.xyzDistToPlayerSq, 180, 20, 100);
            EnFamos_SetupAttackDebris(this);

            // spawn crater on floor
            Actor_SpawnAsChild(&play->actorCtx, &this->actor, play, ACTOR_EN_TEST, this->actor.world.pos.x,
                this->actor.floorHeight, this->actor.world.pos.z, 0, 0, 0, 0x0);

            if (this->actor.child != NULL) {
                Actor_SetScale(this->actor.child, 0.015f);
            }

            if (this->animatedMaterialIndex != FAMOS_ANIMATED_MAT_NORMAL) {
                this->cratorDespawnTimer = 70;
                EnFamos_SetupDeathSlam(this);
            }
            else {
                this->cratorDespawnTimer = 20;
                EnFamos_SetupFinishAttack(this);
            }
        }
        else {
            this->emblemCollider.base.acFlags |= AC_ON;
            EnFamos_SetupAttackRebound(this);
        }
    }
    else {
        Actor_PlaySfx_Flagged(&this->actor, NA_SE_EN_LAST1_FALL_OLD - SFX_FLAG);
    }
}

RECOMP_HOOK("EnFamos_AttackAim") void DAAttackAnim(EnFamos* this) {

    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        this->skelAnime.playSpeed = 1.5f;
        break;

    case 1:
        this->skelAnime.playSpeed = 2.5f;
        break;

    default:
        break;
    }
}