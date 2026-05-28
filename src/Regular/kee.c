#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_en_firefly.h"
#include "overlays/actors/ovl_En_Clear_Tag/z_en_clear_tag.h"
#include "overlays/actors/ovl_Obj_Syokudai/z_obj_syokudai.h"

void EnFirefly_Die(EnFirefly* this, PlayState* play);
void EnFirefly_SetupDiveAttack(EnFirefly* this);

typedef enum {
    /* 0 */ KEESE_FIRE,
    /* 3 */ KEESE_NORMAL = 3,
    /* 4 */ KEESE_ICE,
} KeeseCurrentType;

typedef enum {
    /* 0 */ KEESE_AURA_NONE,
    /* 1 */ KEESE_AURA_FIRE,
    /* 2 */ KEESE_AURA_ICE,
} KeeseAuraType;

void EnFirefly_Extinguish(EnFirefly* this) {
    this->currentType = KEESE_NORMAL;
    this->collider.elem.atDmgInfo.effect = 0; // Nothing
    this->auraType = KEESE_AURA_NONE;
    this->actor.hintId = TATL_HINT_ID_KEESE;
}

RECOMP_HOOK("EnFirefly_DiveAttack") void mcqueenbat(Actor* thisx, PlayState* play2) {

	EnFirefly* this = (EnFirefly*)thisx;

    int Difficulty = (int)recomp_get_config_double("diff_option");
    float speedMultiplier = 1.0f;

    switch (Difficulty) {
    case 0:
        speedMultiplier = 1.75f;
        this->actor.colChkInfo.damage = (this->actor.colChkInfo.damage) / 2;
        break;

    case 1:
        speedMultiplier = 2.5f;
        this->actor.colChkInfo.damage = (this->actor.colChkInfo.damage + 2) / 3;
        break;

    default:
        break;
    }

    if (this->actor.speed <= 6.0f) {
        this->actor.speed *= speedMultiplier;
    }

    if (this->actor.colChkInfo.health == 0) this->actor.speed = 0;
}

RECOMP_HOOK("EnFirefly_FlyIdle") void greaterattackkee(EnFirefly* this, PlayState* play) {

    int Difficulty = (int)recomp_get_config_double("diff_option");
    float AtkDistance = 200.0f;

    switch (Difficulty) {
    case 0:
        AtkDistance = 400.0f;
        break;

    case 1:
        AtkDistance = 600.0f;
        break;

    default:
        break;
    }

    if ((this->timer == 0) && (this->actor.xzDistToPlayer < AtkDistance) && (Player_GetMask(play) != PLAYER_MASK_STONE)) {
        EnFirefly_SetupDiveAttack(this);
    }
}

RECOMP_HOOK("EnFirefly_Update") void lean(Actor* thisx, PlayState* play2) {

    EnFirefly* this = (EnFirefly*)thisx;

    int Difficulty = (int)recomp_get_config_double("diff_option");
    int JinxTimer = 300;

    switch (Difficulty) {
    case 0:
        JinxTimer = 160;
        break;

    case 1:
        JinxTimer = 320;
        break;

    default:
        break;
    }

    if (this->collider.base.atFlags & AT_HIT) {
        this->collider.base.atFlags &= ~AT_HIT;
        Actor_PlaySfx(&this->actor, NA_SE_EN_FFLY_ATTACK);
        gSaveContext.jinxTimer = JinxTimer;
    }
}

RECOMP_PATCH void EnFirefly_PostLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3s* rot, Actor* thisx, Gfx** gfx) {

    static Color_RGBA8 sFireAuraPrimColor = { 230, 100, 255, 255 };
    static Color_RGBA8 sFireAuraEnvColor = { 150, 0, 200, 0 };

    static Color_RGBA8 sIceAuraPrimColor = { 160, 150, 255, 255 };
    static Color_RGBA8 sIceAuraEnvColor = { 60, 0, 255, 0 };

    static Color_RGBA8 sPurpleAuraPrimColor = { 180, 50, 255, 255 };
    static Color_RGBA8 sPurpleAuraEnvColor = { 50, 0, 100, 0 };

    static Vec3f sAuraVelocity = { 0.0f, 0.5f, 0.0f };
    static Vec3f sAuraAccel = { 0.0f, 0.5f, 0.0f };
    Vec3f auraPos;
    Color_RGBA8* auraPrimColor;
    Color_RGBA8* auraEnvColor;
    s16 auraScaleStep;
    s16 auraLife;
    s32 pad;
    EnFirefly* this = (EnFirefly*)thisx;

    if ((this->currentType != KEESE_FIRE) && (limbIndex == FIRE_KEESE_LIMB_HEAD)) {
        gSPDisplayList((*gfx)++, gKeeseRedEyesDL);
    }
    else if ((this->lastDrawnFrame != play->gameplayFrames) &&
        ((limbIndex == FIRE_KEESE_LIMB_LEFT_WING_END) || (limbIndex == FIRE_KEESE_LIMB_RIGHT_WING_END_ROOT))) {
        if (this->actionFunc != EnFirefly_Die) {
            Matrix_MultZero(&auraPos);
            auraPos.x += Rand_ZeroFloat(5.0f);
            auraPos.y += Rand_ZeroFloat(5.0f);
            auraPos.z += Rand_ZeroFloat(5.0f);
            auraScaleStep = -40;
            auraLife = 3;
        }
        else {
            if (limbIndex == FIRE_KEESE_LIMB_LEFT_WING_END) {
                auraPos.x = Math_SinS(9100 * this->timer) * this->timer + this->actor.world.pos.x;
                auraPos.z = Math_CosS(9100 * this->timer) * this->timer + this->actor.world.pos.z;
            }
            else {
                auraPos.x = this->actor.world.pos.x - Math_SinS(9100 * this->timer) * this->timer;
                auraPos.z = this->actor.world.pos.z - Math_CosS(9100 * this->timer) * this->timer;
            }

            auraPos.y = this->actor.world.pos.y + (15 - this->timer) * 1.5f;
            auraScaleStep = -5;
            auraLife = 10;
        }

        if (this->auraType == KEESE_AURA_FIRE) {
            auraPrimColor = &sFireAuraPrimColor;
            auraEnvColor = &sFireAuraEnvColor;
        }
        else if (this->auraType == KEESE_AURA_ICE) {
            auraPrimColor = &sIceAuraPrimColor;
            auraEnvColor = &sIceAuraEnvColor;
        }
        else {
            auraPrimColor = &sPurpleAuraPrimColor;
            auraEnvColor = &sPurpleAuraEnvColor;
        }

        func_800B0F80(play, &auraPos, &sAuraVelocity, &sAuraAccel, auraPrimColor, auraEnvColor, 250, auraScaleStep,
            auraLife);
    }

    if (limbIndex == FIRE_KEESE_LIMB_LEFT_WING_END) {
        Matrix_MultZero(&this->bodyPartsPos[KEESE_BODYPART_LEFT_WING_END]);
    }
    else if (limbIndex == FIRE_KEESE_LIMB_RIGHT_WING_END_ROOT) {
        Matrix_MultZero(&this->bodyPartsPos[KEESE_BODYPART_RIGHT_WING_END_ROOT]);
    }
    else if (limbIndex == FIRE_KEESE_LIMB_BODY) {
        Matrix_MultZero(&this->bodyPartsPos[KEESE_BODYPART_BODY]);
    }
}