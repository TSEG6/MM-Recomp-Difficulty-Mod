#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_en_neo_reeba.h"
#include "attributes.h"
#include "assets/objects/object_rb/object_rb.h"

void EnNeoReeba_Draw(Actor* thisx, PlayState* play);
void EnNeoReeba_Move(EnNeoReeba* this, PlayState* play);

void EnNeoReeba_DrawFrozenEffects(EnNeoReeba* this, PlayState* play) {
    s32 i;
    f32 bodyPartPosScaleXZ = 10.0f;
    f32 phi_f2 = 20.0f;
    f32 drawEffectScale = 0.8f;

    if (EN_NEO_REEBA_IS_LARGE(&this->actor)) {
        bodyPartPosScaleXZ *= 1.5f;
        phi_f2 *= 1.5f;
        drawEffectScale *= 1.5f;
    }

    for (i = 0; i <= EN_NEO_REEBA_BODYPART_2; i++) {
        this->bodyPartsPos[i] = this->actor.world.pos;

        this->bodyPartsPos[i].x += bodyPartPosScaleXZ * Math_SinS(BINANG_ADD(this->actor.shape.rot.y, i * 0x5555));
        this->bodyPartsPos[i].z += bodyPartPosScaleXZ * Math_CosS(BINANG_ADD(this->actor.shape.rot.y, i * 0x5555));
        this->bodyPartsPos[i].y += 5.0f;
    }

    this->bodyPartsPos[EN_NEO_REEBA_BODYPART_3] = this->actor.world.pos;
    this->bodyPartsPos[EN_NEO_REEBA_BODYPART_3].y += phi_f2;

    this->drawEffectScale = drawEffectScale;
    Actor_DrawDamageEffects(play, &this->actor, this->bodyPartsPos, EN_NEO_REEBA_BODYPART_MAX, drawEffectScale, 0.5f,
        this->drawEffectAlpha, this->drawEffectType);
}

s32 EnNeoReeba_OverrideLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot, Actor* thisx) {
    EnNeoReeba* this = (EnNeoReeba*)thisx;

    if ((limbIndex == OBJECT_RB_LIMB_03) && (this->rotationSpeed != 0.0f)) {
        rot->y += TRUNCF_BINANG(this->rotationSpeed * Math_SinS(this->rotationAngle));
        rot->z += TRUNCF_BINANG(this->rotationSpeed * Math_CosS(this->rotationAngle));
    }

    return false;
}

void EnNeoReeba_DrawEffects(EnNeoReeba* this, PlayState* play) {
    s32 i;
    f32 scale = 15.0f;

    if (EN_NEO_REEBA_IS_LARGE(&this->actor)) {
        scale *= 1.5f;
    }

    if ((this->drawEffectType == ACTOR_DRAW_DMGEFF_FIRE) || (this->drawEffectType == ACTOR_DRAW_DMGEFF_LIGHT_ORBS) ||
        (this->drawEffectType == ACTOR_DRAW_DMGEFF_ELECTRIC_SPARKS_SMALL)) {
        for (i = 0; i <= EN_NEO_REEBA_BODYPART_2; i++) {
            this->bodyPartsPos[i] = this->actor.world.pos;
            this->bodyPartsPos[i].x += scale * Math_SinS(BINANG_ADD(this->actor.shape.rot.y, i * 0x5555));
            this->bodyPartsPos[i].z += scale * Math_CosS(BINANG_ADD(this->actor.shape.rot.y, i * 0x5555));
            this->bodyPartsPos[i].y += -20.0f;
        }

        this->bodyPartsPos[EN_NEO_REEBA_BODYPART_3] = this->actor.world.pos;
        Actor_DrawDamageEffects(play, NULL, this->bodyPartsPos, EN_NEO_REEBA_BODYPART_MAX, this->drawEffectScale, 0.5f,
            this->drawEffectAlpha, this->drawEffectType);
    }
}

RECOMP_PATCH void EnNeoReeba_Draw(Actor* thisx, PlayState* play) {
    EnNeoReeba* this = (EnNeoReeba*)thisx;

    Gfx_SetupDL25_Opa(play->state.gfxCtx);

    OPEN_DISPS(play->state.gfxCtx);

    int Difficulty = (int)recomp_get_config_double("diff_option");

    if (this->actor.home.rot.x == 1) {

        gDPSetPrimColor(POLY_OPA_DISP++, 0, 0x01, 155, 55, 255, 255);

    }
    else {

        gDPSetPrimColor(POLY_OPA_DISP++, 0, 0x01, 255, 255, 255, 255);

    }


    SkelAnime_DrawOpa(play, this->skelAnime.skeleton, this->skelAnime.jointTable, EnNeoReeba_OverrideLimbDraw, NULL,
        &this->actor);

    CLOSE_DISPS(play->state.gfxCtx);

    if (this->stunTimer > 0) {
        if (this->drawEffectType == ACTOR_DRAW_DMGEFF_FROZEN_SFX) {
            EnNeoReeba_DrawFrozenEffects(this, play);
        }
        else {
            EnNeoReeba_DrawEffects(this, play);
        }
    }
}

RECOMP_HOOK_RETURN("EnNeoReeba_Init") void InitStuffRETURN(Actor* thisx, PlayState* play) {

    EnNeoReeba* this = (EnNeoReeba*)thisx;
    int Difficulty = (int)recomp_get_config_double("diff_option");
    u8 baseHealth = this->actor.colChkInfo.health;


    switch (Difficulty) {
    case 0:
        this->actor.colChkInfo.health = baseHealth * 5;
        break;

    case 1: {
        this->actor.colChkInfo.health = baseHealth * 10;
        if (Rand_ZeroOne() < 0.1f) {

            this->actor.colChkInfo.health = baseHealth * 30;
            this->actor.home.rot.x = 1;

        }
        thisx->params |= 0x8000;
        Actor_SetScale(&this->actor, 0.05f);
        this->collider.dim.radius = 27;
        this->collider.dim.height = 45;
        break;
    }
    default:
        break;
    }
}

void EnNeoReeba_SetupMove(EnNeoReeba* this) {
    Actor_PlaySfx(&this->actor, NA_SE_EN_RIVA_MOVE);
    this->sfxTimer = 10;
    this->actionTimer = 60;
    this->actionFunc = EnNeoReeba_Move;
    this->skelAnime.playSpeed = 2.0f;
    this->actor.speed = 14.0f;
}

RECOMP_HOOK("EnNeoReeba_ChooseAction") void IncreasedRange(EnNeoReeba* this, PlayState* play) {

    Player* player = GET_PLAYER(play);
    f32 distToPlayer = Actor_WorldDistXZToPoint(&player->actor, &this->actor.home.pos);
    int Difficulty = (int)recomp_get_config_double("diff_option");
    float distanceCheck = 0.0f;

    switch (Difficulty) {
    case 0:
        distanceCheck = 140.0f;
        break;

    case 1: {

        if (this->actor.home.rot.x == 1) {

            distanceCheck = 240.0f;
        }
        else {

            distanceCheck = 180.0f;
        }
        break;
    }
    default:
        break;
    }

    if ((distToPlayer < distanceCheck) && (fabsf(this->actor.playerHeightRel) < 100.0f)) {
        this->targetPos = player->actor.world.pos;
        this->targetPos.x += 10.0f * player->actor.speed * Math_SinS(player->actor.world.rot.y);
        this->targetPos.z += 10.0f * player->actor.speed * Math_CosS(player->actor.world.rot.y);
        EnNeoReeba_SetupMove(this);
    }

}

RECOMP_HOOK("EnNeoReeba_UpdatePosition") void leeverupdatething(EnNeoReeba* this, PlayState* play) {

    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:

        if (this->actionTimer != 0) this->skelAnime.playSpeed = 1.1f;
        else this->actor.speed = 15.0f;
        break;

    case 1: {
        if (this->actionTimer != 0) {
            this->skelAnime.playSpeed = 1.25f;
        }
        else {

            if (this->actor.home.rot.x == 1) {

                this->actor.speed = 22.0f;

            }
            else {

                this->actor.speed = 18.0f;
            }
        }
        break;
    }
    default:
        break;
    }

}