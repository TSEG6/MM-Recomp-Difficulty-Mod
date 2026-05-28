#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_en_dekunuts.h"
#include "overlays/actors/ovl_En_Clear_Tag/z_en_clear_tag.h"
#include "overlays/effects/ovl_Effect_Ss_Hahen/z_eff_ss_hahen.h"
#include "assets/objects/gameplay_keep/gameplay_keep.h"

void func_808BD870(EnDekunuts* this);

RECOMP_HOOK("func_808BD49C") void AtkDistanceD(EnDekunuts* this, PlayState* play) {

    s32 phi_v1 = false;

    int Difficulty = (int)recomp_get_config_double("diff_option");
    float AtkDistD = 480.0f;

    switch (Difficulty) {
    case 0:
        AtkDistD = 600.0f;
        break;

    case 1: {
        AtkDistD = 750.0f;
        break;
    }
    default:
        break;
    }

    if (phi_v1 && ((this->actor.xzDistToPlayer > 160.0f) || (this->actor.params != ENDEKUNUTS_GET_FF00_0)) &&
        (((this->actor.params == ENDEKUNUTS_GET_FF00_0) && (fabsf(this->actor.playerHeightRel) < 120.0f)) ||
            ((this->actor.params == ENDEKUNUTS_GET_FF00_2) && (this->actor.playerHeightRel > -60.0f)) ||
            (this->actor.params == ENDEKUNUTS_GET_FF00_1)) &&
        ((this->unk_190 == 0) || (this->actor.xzDistToPlayer < AtkDistD))) {
        this->skelAnime.playSpeed = 1.0f;
    }

}

RECOMP_PATCH void func_808BDA4C(EnDekunuts* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    Vec3f targetPos;
    s16 pitch;
    Vec3f pos;
    f32 val;
    s16 params;
    f32 projectileSpeed;
    f32 predictFactor;
    int Difficulty = (int)recomp_get_config_double("diff_option");

    if (Difficulty == 1) {
        projectileSpeed = 20.0f;
        predictFactor = 0.9f;
    }
    else {
        projectileSpeed = 13.0f;
        predictFactor = 0.6f;
    }

    f32 dx = player->actor.world.pos.x - this->actor.world.pos.x;
    f32 dz = player->actor.world.pos.z - this->actor.world.pos.z;
    f32 dist = sqrtf(SQ(dx) + SQ(dz));
    f32 travelTime = CLAMP(dist / projectileSpeed, 0.0f, 20.0f);

    targetPos.x = player->actor.world.pos.x + (player->actor.velocity.x * travelTime * predictFactor);
    targetPos.z = player->actor.world.pos.z + (player->actor.velocity.z * travelTime * predictFactor);
    targetPos.y = player->actor.world.pos.y + 40.0f;

    s16 targetYaw = Math_Vec3f_Yaw(&this->actor.world.pos, &targetPos);

    Math_ApproachS(&this->actor.shape.rot.y, targetYaw, 2, 0xE38);

    if (this->actor.params == ENDEKUNUTS_GET_FF00_2) {

        pitch = Actor_WorldPitchTowardPoint(&this->actor, &targetPos);
        pitch = CLAMP(pitch, -0x3800, -0x2000);

        if (this->skelAnime.curFrame < 7.0f) {
            Math_ScaledStepToS(&this->actor.world.rot.x, pitch, 0x800);
        }
        else {
            Math_ScaledStepToS(&this->actor.world.rot.x, 0, 0x800);
        }
    }

    if (SkelAnime_Update(&this->skelAnime)) {
        this->actor.world.rot.x = 0;
        func_808BD870(this);
    }
    else if (Animation_OnFrame(&this->skelAnime, 7.0f)) {
        val = Math_CosS(this->actor.world.rot.x) * 15.0f;
        pos.x = (Math_SinS(this->actor.shape.rot.y) * val) + this->actor.world.pos.x;
        pos.y = (this->actor.world.pos.y + 12.0f) - (Math_SinS(this->actor.world.rot.x) * 15.0f);
        pos.z = (Math_CosS(this->actor.shape.rot.y) * val) + this->actor.world.pos.z;

        params = (this->actor.params == ENDEKUNUTS_GET_FF00_2) ? ENDEKUNUTS_GET_FF00_2 : ENDEKUNUTS_GET_FF00_0;

        Actor* nut = Actor_Spawn(&play->actorCtx, play, ACTOR_EN_NUTSBALL, pos.x, pos.y, pos.z,
            this->actor.world.rot.x, this->actor.shape.rot.y, 0, params);

        if (nut != NULL) {
            nut->speed = projectileSpeed;
            nut->velocity.x = projectileSpeed * Math_SinS(nut->world.rot.y) * Math_CosS(nut->world.rot.x);
            nut->velocity.y = projectileSpeed * -Math_SinS(nut->world.rot.x);
            nut->velocity.z = projectileSpeed * Math_CosS(nut->world.rot.y) * Math_CosS(nut->world.rot.x);

            Actor_PlaySfx(&this->actor, NA_SE_EN_NUTS_THROW);
        }
    }
    else if ((this->unk_190 >= 2) && Animation_OnFrame(&this->skelAnime, 12.0f)) {
        Animation_MorphToPlayOnce(&this->skelAnime, &gDekuScrubSpitAnim, -3.0f);
        if (this->unk_190 != 0) {
            this->unk_190--;
        }
    }
}