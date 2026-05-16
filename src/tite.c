#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_en_tite.h"
#include "overlays/actors/ovl_En_Clear_Tag/z_en_clear_tag.h"

void func_80893ED4(EnTite* this);
void func_80893FD0(EnTite* this);
void func_8089484C(EnTite* this);
void func_8089408C(EnTite* this, PlayState* play);
void func_808942B4(EnTite* this, PlayState* play);
void func_808945EC(EnTite* this);
void func_808956B8(EnTite* this);

s32 func_80893ADC(EnTite* this) {
    if ((this->actor.params == ENTITE_MINUS_2) && (this->actor.bgCheckFlags & BGCHECKFLAG_WATER)) {
        return true;
    }
    return false;
}

static Color_RGBA8 D_80896B3C = { 250, 250, 250, 255 };
static Color_RGBA8 D_80896B40 = { 180, 180, 180, 255 };
static Vec3f D_80896B44 = { 0.0f, 0.45f, 0.0f };

s32 func_80893A34(EnTite* this, PlayState* play) {
    if ((this->actor.params == ENTITE_MINUS_2) && (this->actor.bgCheckFlags & BGCHECKFLAG_GROUND) &&
        (SurfaceType_GetFloorType(&play->colCtx, this->actor.floorPoly, this->actor.floorBgId) == FLOOR_TYPE_5)) {
        return true;
    }
    return false;
}

void func_80893A9C(EnTite* this, PlayState* play) {
    if (func_80893A34(this, play)) {
        func_808956B8(this);
    }
    else {
        func_80893FD0(this);
    }
}

s32 func_80893B10(EnTite* this) {
    Math_StepToF(&this->actor.velocity.y, 0.0f, 2.0f);
    return Math_StepToF(&this->actor.world.pos.y, (this->actor.world.pos.y + this->actor.depthInWater) - 1.0f, 2.0f);
}

void func_80893B70(EnTite* this) {
    if (this->actor.params == ENTITE_MINUS_2) {
        if (this->actor.bgCheckFlags & BGCHECKFLAG_WATER) {
            this->actor.gravity = 0.0f;
            func_80893B10(this);
        }
        else {
            this->actor.gravity = -1.0f;
        }
    }
}

RECOMP_HOOK("func_80893F30") void RangeTite(EnTite* this, PlayState* play) {

    int Difficulty = (int)recomp_get_config_double("diff_option");
    float sightDist = 300.0f;

    switch (Difficulty) {
    case 0:
        sightDist = 400.0f;
        break;

    case 1:
        sightDist = 500.0f;
        break;

    default:
        break;
    }

    if ((Player_GetMask(play) != PLAYER_MASK_STONE) && (this->actor.xzDistToPlayer < sightDist) &&
        (this->actor.playerHeightRel < 80.0f)) {
        func_808945EC(this);
    }

}

RECOMP_PATCH void func_80894638(EnTite* this, PlayState* play) {
    s16 temp_v0;
    s16 temp_v1;
    int Difficulty = (int)recomp_get_config_double("diff_option");

    func_80893B70(this);

    temp_v0 = this->actor.yawTowardsPlayer - this->actor.shape.rot.y;
    if (temp_v0 > 0) {
        temp_v1 = (s32)(temp_v0 * (1.0f / 42.0f)) + 10;
    }
    else {
        temp_v1 = (s32)(temp_v0 * (1.0f / 42.0f)) - 10;
    }

    s32 rotationStep = temp_v1 * 2;
    f32 animSpeed = temp_v1 * 0.01f;

    switch (Difficulty) {
    case 0:
        rotationStep *= 2;
        animSpeed *= 1.4f;
        break;

    case 1:
        rotationStep *= 3;
        animSpeed *= 1.4f;
        break;

    default:
        break;
    }

    animSpeed = CLAMP(animSpeed, -2.5f, 2.5f);

    this->actor.shape.rot.y += rotationStep;
    this->actor.world.rot.y = this->actor.shape.rot.y;

    this->skelAnime.playSpeed = animSpeed;
    SkelAnime_Update(&this->skelAnime);

    if (Animation_OnFrame(&this->skelAnime, 7.0f)) {
        if (func_80893ADC(this)) {
            Actor_PlaySfx(&this->actor, NA_SE_EN_TEKU_WALK_WATER);
        }
        else {
            Actor_PlaySfx(&this->actor, NA_SE_EN_TEKU_WALK);
        }
    }

    if ((Player_GetMask(play) == PLAYER_MASK_STONE) || (this->actor.xzDistToPlayer > 450.0f) ||
        (this->actor.playerHeightRel > 80.0f)) {
        func_80893ED4(this);
    }
    else if (((this->actor.bgCheckFlags & BGCHECKFLAG_GROUND) ||
        (func_80893ADC(this) && (this->actor.depthInWater < 10.0f))) &&
        Actor_IsFacingPlayer(&this->actor, 0xE38)) {
        if ((this->actor.xzDistToPlayer <= 180.0f) && (this->actor.playerHeightRel <= 80.0f)) {
            func_80893A9C(this, play);
        }
        else {
            func_8089484C(this);
        }
    }
}

RECOMP_PATCH void func_80894024(EnTite* this, PlayState* play) {
    if (SkelAnime_Update(&this->skelAnime)) {
        func_8089408C(this, play);
    }
    else {
        int Difficulty = (int)recomp_get_config_double("diff_option");
        s16 turnSpeed = 0x3E8;

        if (Difficulty == 1) {
            turnSpeed = 0x3000;
        }
        else {
            turnSpeed = 0x1000;
        }

        func_80893B70(this);

        Math_ScaledStepToS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, turnSpeed);
        this->actor.world.rot.y = this->actor.shape.rot.y;
    }
}

RECOMP_PATCH void func_8089408C(EnTite* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    int Difficulty = (int)recomp_get_config_double("diff_option");

    Vec3f predictedPos;
    f32 dx;
    f32 dz;
    f32 distance;
    f32 travelTime;
    f32 predictFactor;
    s16 targetYaw;

    Animation_PlayOnce(&this->skelAnime, &object_tite_Anim_0004F8);

    if (!func_80893ADC(this)) {
        Actor_PlaySfx(&this->actor, NA_SE_EN_TEKU_JUMP);
    }
    else {
        this->actor.world.pos.y += this->actor.depthInWater;
        Actor_PlaySfx(&this->actor, NA_SE_EN_TEKU_JUMP_WATER);
    }

    this->actor.speed = 4.0f;
    if (Difficulty == 1) {
        this->actor.speed *= 1.85f;
    }

    dx = player->actor.world.pos.x - this->actor.world.pos.x;
    dz = player->actor.world.pos.z - this->actor.world.pos.z;
    distance = sqrtf(SQ(dx) + SQ(dz));

    travelTime = distance / this->actor.speed;
    travelTime = CLAMP(travelTime, 0.0f, 15.0f);

    if (Difficulty == 1) {
        predictFactor = 0.9f;
    }
    else {
        predictFactor = 0.3f;
    }

    predictedPos.x = player->actor.world.pos.x + (player->actor.velocity.x * travelTime * predictFactor);
    predictedPos.y = player->actor.world.pos.y;
    predictedPos.z = player->actor.world.pos.z + (player->actor.velocity.z * travelTime * predictFactor);

    targetYaw = Math_Vec3f_Yaw(&this->actor.world.pos, &predictedPos);

    if (this->actor.shape.yOffset < 0.0f) {
        s32 i;
        Vec3f sp70;
        Vec3f sp64;

        sp64.y = 3.0f;
        this->actor.shape.yOffset = 0.0f;

        for (i = 0; i < 4; i++) {
            sp64.x = 2.0f * Math_SinS(this->actor.shape.rot.y);
            sp64.z = 2.0f * Math_CosS(this->actor.shape.rot.y);
            sp70.x = this->actor.world.pos.x + (12.5f * sp64.x);
            sp70.y = this->actor.world.pos.y + 15.0f;
            sp70.z = this->actor.world.pos.z + (12.5f * sp64.z);
            func_800B0DE0(play, &sp70, &sp64, &D_80896B44, &D_80896B3C, &D_80896B40, 500, 50);
            this->actor.shape.rot.y += 0x4000;
        }

        this->actor.shape.rot.y = targetYaw;
        this->actor.world.rot.y = this->actor.shape.rot.y;
        this->actor.shape.shadowDraw = ActorShadow_DrawCircle;
        this->actor.flags |= ACTOR_FLAG_ATTENTION_ENABLED;
        this->actor.velocity.y = 10.0f;
    }
    else {
        this->actor.velocity.y = 8.0f;
        this->actor.world.rot.y = targetYaw;
        this->actor.shape.rot.y = targetYaw;
    }

    if (Difficulty == 1) {
        this->actor.velocity.y *= 1.1f;
    }

    this->actor.bgCheckFlags &=
        ~(BGCHECKFLAG_GROUND | BGCHECKFLAG_GROUND_TOUCH | BGCHECKFLAG_WATER | BGCHECKFLAG_WATER_TOUCH);
    this->actor.gravity = -1.0f;
    this->actionFunc = func_808942B4;
}

RECOMP_HOOK("EnTite_Update") void TiteDmgRed(Actor* thisx, PlayState* play) {

    EnTite* this = (EnTite*)thisx;

    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        this->actor.colChkInfo.damage = (this->actor.colChkInfo.damage) / 2;
        break;

    case 1:
        this->actor.colChkInfo.damage = (this->actor.colChkInfo.damage + 2) / 4;
        break;

    default:
        break;
    }

}