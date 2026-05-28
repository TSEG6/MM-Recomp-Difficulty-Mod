#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_en_bigpamet.h"
#include "z64quake.h"
#include "z64rumble.h"

void func_80A27B58(EnBigpamet* this);
void func_80A28970(EnBigpamet* this);
void func_80A28D0C(EnBigpamet* this, PlayState* play);

void func_80A27FE8(EnBigpamet* this, PlayState* play) {
    Vec3f pos;
    s16 sp32;

    if (this->actor.depthInWater > 0.0f) {
        pos.x = this->actor.world.pos.x;
        pos.z = this->actor.world.pos.z;
        pos.y = this->actor.world.pos.y + this->actor.depthInWater;

        EffectSsGRipple_Spawn(play, &pos, 500, 900, 0);
        pos.y += 8.0f;

        if (this->actionFunc != func_80A28D0C) {
            sp32 = (s32)Rand_CenteredFloat(0x8000) + this->actor.world.rot.y;
            pos.x -= 55.0f * Math_SinS(sp32);
            pos.z -= 55.0f * Math_CosS(sp32);
        }
        EffectSsGSplash_Spawn(play, &pos, NULL, NULL, 0, Rand_S16Offset(1400, 200));
    }
}

void func_80A27DD8(EnBigpamet* this, PlayState* play) {
    s32 i;
    Vec3f pos;
    Vec3f sp8C;
    f32 temp_fs0;
    f32 temp_fs4;
    f32 temp_fs5;
    s16 temp_s0;

    temp_fs4 = Math_SinS(this->actor.wallYaw + 0x4000);
    temp_fs5 = Math_CosS(this->actor.wallYaw + 0x4000);

    sp8C.x = Math_SinS(this->actor.wallYaw + 0x8000) * 50.0f + this->actor.world.pos.x;
    sp8C.y = this->actor.world.pos.y;
    sp8C.z = (Math_CosS(this->actor.wallYaw + 0x8000) * 50.0f) + this->actor.world.pos.z;

    sp8C.x += (this->actor.world.pos.x - sp8C.x) * 0.3f;
    sp8C.z += (this->actor.world.pos.z - sp8C.z) * 0.3f;

    for (i = 0; i < 4; i++) {
        temp_fs0 = Rand_ZeroFloat(60.0f) + 50.0f;
        temp_s0 = Rand_Next() >> 0x11;

        pos.x = (Math_CosS(temp_s0) * temp_fs0 * temp_fs4) + sp8C.x;
        pos.y = (Math_SinS(temp_s0) * temp_fs0) + sp8C.y;
        pos.z = (Math_CosS(temp_s0) * temp_fs0 * temp_fs5) + sp8C.z;

        func_800B12F0(play, &pos, &gZeroVec3f, &gZeroVec3f, Rand_S16Offset(950, 100), Rand_S16Offset(20, 10), 20);
    }
}

// This doesn't seem to do much

RECOMP_HOOK("EnBigpamet_Update") void FSpdate(Actor* thisx, PlayState* play) {
    EnBigpamet* this = (EnBigpamet*)thisx;

    int Difficulty = (int)recomp_get_config_double("diff_option");
    float speedMultiplier = 1.0f;

    switch (Difficulty) {
    case 0:
        speedMultiplier = 1.25f;
        this->actor.colChkInfo.damage = (this->actor.colChkInfo.damage) / 2;
        break;

    case 1:
        speedMultiplier = 1.5f;
        this->actor.colChkInfo.damage = (this->actor.colChkInfo.damage + 2) / 3;
        break;

    default:
        break;
    }

    if (this->actor.colChkInfo.health != 0) {
        this->snapperSkelAnime.playSpeed = speedMultiplier;
    }

    if (this->actor.colChkInfo.health == 0) this->actor.speed = 0;

    if (this->actor.speed <= 6.0f) {
        this->actor.speed *= speedMultiplier;
    }
}

// Charge speed based on difficulty and a bit of prediction

RECOMP_PATCH void func_80A287E8(EnBigpamet* this, PlayState* play) {
    s16 quakeIndex;

    int Difficulty = (int)recomp_get_config_double("diff_option");

    f32 speedMult = (Difficulty == 0) ? 1.2f : 1.6f;
    s16 turnAmount = (Difficulty == 0) ? 0x400 : 0x600;
    s16 maxMomentum = (Difficulty == 0) ? 24 : 32;
    f32 forwardSpeedMult = (Difficulty == 0) ? 1.0f : 1.5f;
    f32 maxSpeed = (Difficulty == 0) ? 24.0f : 36.0f;

    if (this->unk_29E == 0) {
        Player* player = GET_PLAYER(play);
        Vec3f predictedPos;
        f32 dx = player->actor.world.pos.x - this->actor.world.pos.x;
        f32 dz = player->actor.world.pos.z - this->actor.world.pos.z;
        f32 distance = sqrtf(SQ(dx) + SQ(dz));
        f32 travelTime = distance / 20.0f;
        f32 predictFactor = (Difficulty == 1) ? 0.75f : 0.25f;

        travelTime = CLAMP(travelTime, 0.0f, 12.0f);

        predictedPos.x = player->actor.world.pos.x + (player->actor.velocity.x * travelTime * predictFactor);
        predictedPos.z = player->actor.world.pos.z + (player->actor.velocity.z * travelTime * predictFactor);

        this->actor.world.rot.y = Math_Vec3f_Yaw(&this->actor.world.pos, &predictedPos);
    }

    this->actor.shape.rot.y += (s16)(0x3B00 * speedMult);

    Actor_PlaySfx_Flagged(&this->actor,
        NA_SE_EN_B_PAMET_ROLL - SFX_FLAG);

    this->unk_29E += (Difficulty == 0) ? 2 : 3;
    this->unk_29E = CLAMP_MAX(this->unk_29E, maxMomentum);

    if (this->collider.base.atFlags & AT_HIT) {
        this->collider.base.atFlags &= ~AT_HIT;
    }

    if (play->gameplayFrames & 1) {
        func_80A27FE8(this, play);
    }

    this->actor.speed *= forwardSpeedMult;
    if (this->actor.speed > maxSpeed) {
        this->actor.speed = maxSpeed;
    }

    if (this->actor.bgCheckFlags & BGCHECKFLAG_WALL) {
        quakeIndex =
            Quake_Request(GET_ACTIVE_CAM(play), QUAKE_TYPE_3);

        this->actor.velocity.y =
            this->unk_29E * (Difficulty == 0 ? 0.45f : 0.6f);

        Quake_SetSpeed(quakeIndex, 20000);
        Quake_SetPerturbations(quakeIndex, 15, 0, 0, 0);
        Quake_SetDuration(quakeIndex, 10);

        Rumble_Request(
            this->actor.xyzDistToPlayerSq, 180, 20, 100);

        func_80A27B58(this);
        func_80A27DD8(this, play);
        func_80A28970(this);
    }
}