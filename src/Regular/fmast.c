#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_en_floormas.h"
#include "overlays/actors/ovl_En_Clear_Tag/z_en_clear_tag.h"

void func_808D0C14(EnFloormas* this);
void func_808D0CE4(EnFloormas* this);
void func_808D161C(EnFloormas* this);
void func_808D1740(EnFloormas* this);
void func_808D2700(EnFloormas* this);
void func_80833B18(PlayState* play, Player* this, s32 arg2, f32 speed, f32 velocityY, s16 arg5, s32 invincibilityTimer);

extern void func_808D14DC(EnFloormas*, PlayState*);
extern void func_808D0908(EnFloormas*);

s32 EnFloormas_OverrideLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot, Actor* thisx,
    Gfx** gfx) {
    EnFloormas* this = (EnFloormas*)thisx;

    if (limbIndex == WALLMASTER_LIMB_ROOT) {
        pos->z += this->unk_192;
    }

    return false;
}

static s8 sLimbToBodyParts[WALLMASTER_LIMB_MAX] = {
    BODYPART_NONE,         // WALLMASTER_LIMB_NONE
    BODYPART_NONE,         // WALLMASTER_LIMB_ROOT
    BODYPART_NONE,         // WALLMASTER_LIMB_HAND
    BODYPART_NONE,         // WALLMASTER_LIMB_INDEX_FINGER_ROOT
    ENFLOORMAS_BODYPART_0, // WALLMASTER_LIMB_INDEX_FINGER_PROXIMAL
    BODYPART_NONE,         // WALLMASTER_LIMB_INDEX_FINGER_DISTAL_ROOT
    BODYPART_NONE,         // WALLMASTER_LIMB_INDEX_FINGER_MIDDLE
    ENFLOORMAS_BODYPART_1, // WALLMASTER_LIMB_INDEX_FINGER_DISTAL
    BODYPART_NONE,         // WALLMASTER_LIMB_RING_FINGER_ROOT
    ENFLOORMAS_BODYPART_2, // WALLMASTER_LIMB_RING_FINGER_PROXIMAL
    BODYPART_NONE,         // WALLMASTER_LIMB_RING_FINGER_DISTAL_ROOT
    BODYPART_NONE,         // WALLMASTER_LIMB_RING_FINGER_MIDDLE
    ENFLOORMAS_BODYPART_3, // WALLMASTER_LIMB_RING_FINGER_DISTAL
    BODYPART_NONE,         // WALLMASTER_LIMB_MIDDLE_FINGER_ROOT
    ENFLOORMAS_BODYPART_4, // WALLMASTER_LIMB_MIDDLE_FINGER_PROXIMAL
    BODYPART_NONE,         // WALLMASTER_LIMB_MIDDLE_FINGER_DISTAL_ROOT
    BODYPART_NONE,         // WALLMASTER_LIMB_MIDDLE_FINGER_MIDDLE
    ENFLOORMAS_BODYPART_5, // WALLMASTER_LIMB_MIDDLE_FINGER_DISTAL
    BODYPART_NONE,         // WALLMASTER_LIMB_WRIST_ROOT
    BODYPART_NONE,         // WALLMASTER_LIMB_WRIST
    BODYPART_NONE,         // WALLMASTER_LIMB_THUMB_ROOT
    ENFLOORMAS_BODYPART_6, // WALLMASTER_LIMB_THUMB_PROXIMAL
    ENFLOORMAS_BODYPART_7, // WALLMASTER_LIMB_THUMB_DISTAL_ROOT
    BODYPART_NONE,         // WALLMASTER_LIMB_THUMB_MIDDLE
    ENFLOORMAS_BODYPART_8, // WALLMASTER_LIMB_THUMB_DISTAL
};

void EnFloormas_PostLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3s* rot, Actor* thisx, Gfx** gfx) {
    EnFloormas* this = (EnFloormas*)thisx;

    if (sLimbToBodyParts[limbIndex] != BODYPART_NONE) {
        Matrix_MultZero(&this->bodyPartsPos[sLimbToBodyParts[limbIndex]]);
    }

    if (limbIndex == WALLMASTER_LIMB_WRIST) {
        Matrix_MultVecX(1000.0f, &this->bodyPartsPos[ENFLOORMAS_BODYPART_9]);
        Matrix_MultVecX(-1000.0f, &this->bodyPartsPos[ENFLOORMAS_BODYPART_10]);
    }
    else if (limbIndex == WALLMASTER_LIMB_HAND) {
        Matrix_Push();
        Matrix_Translate(1600.0f, -700.0f, -1700.0f, MTXMODE_APPLY);
        Matrix_RotateYF(M_PIf / 3, MTXMODE_APPLY);
        Matrix_RotateZF(M_PIf / 12, MTXMODE_APPLY);
        Matrix_Scale(2.0f, 2.0f, 2.0f, MTXMODE_APPLY);

        MATRIX_FINALIZE_AND_LOAD((*gfx)++, play->state.gfxCtx);
        gSPDisplayList((*gfx)++, gWallmasterLittleFingerDL);

        Matrix_Pop();
    }
}

// This makes it so there's no cooldown animation on Hard
RECOMP_HOOK("func_808D17EC") void InstaReset(EnFloormas* this, PlayState* play) {

    int Difficulty = (int)recomp_get_config_double("diff_option");

    if (Difficulty == 1) {

        if (this->actor.bgCheckFlags & BGCHECKFLAG_GROUND_TOUCH) {
            if (this->actor.params != ENFLOORMAS_GET_7FFF_40) {
                func_808D0CE4(this);
            }
        }
    }
}

// This makes attacking faster 
RECOMP_PATCH void func_808D1650(EnFloormas* this, PlayState* play) {
    f32 temp_f0_2;

    int Difficulty = (int)recomp_get_config_double("diff_option");
    float speedMult = (Difficulty >= 1) ? 2.0f : 1.5f;
    float minStep = (Difficulty >= 1) ? 5.0f : 1.0f;

    if (this->unk_18E != 0) {
        this->unk_18E--;
    }

    f32 targetSpeed = 15.0f * speedMult;
    f32 baseSpeedEquiv = this->actor.speed / speedMult;
    f32 stepAmount = SQ(baseSpeedEquiv) * (1.0f / 3.0f) * speedMult;

    if (stepAmount < minStep) {
        stepAmount = minStep;
    }

    Math_StepToF(&this->actor.speed, targetSpeed, stepAmount);
    Math_ScaledStepToS(&this->actor.shape.rot.x, -0x1680, 0x140);

    temp_f0_2 = this->actor.world.pos.y - this->actor.floorHeight;

    if (temp_f0_2 < 10.0f) {
        this->actor.world.pos.y = this->actor.floorHeight + 10.0f;
        this->actor.gravity = 0.0f;
        this->actor.velocity.y = 0.0f;
    }
    else if (temp_f0_2 > 10.0f && temp_f0_2 < 60.0f) {
        Math_StepToF(&this->actor.world.pos.y, this->actor.floorHeight + 10.0f, 15.0f);
        this->actor.velocity.y = 0.0f;
    }

    if (temp_f0_2 < 12.0f) {
        func_808D14DC(this, play);
    }

    if ((this->actor.bgCheckFlags & BGCHECKFLAG_WALL) || (this->unk_18E == 0)) {
        func_808D1740(this);
    }
}

// And this makes the attack prep faster iirc (it looks like it)
RECOMP_PATCH void func_808D1458(EnFloormas* this, PlayState* play) {
    if (SkelAnime_Update(&this->skelAnime)) {
        func_808D161C(this);
    }

    int Difficulty = (int)recomp_get_config_double("diff_option");

    f32 speedMult = 1.0f + (0.5f * Difficulty);
    s16 targetYaw = this->actor.yawTowardsPlayer;

    if (Difficulty >= 1) {
        Player* player = GET_PLAYER(play);

        if (fabsf(player->actor.velocity.x) > 0.1f || fabsf(player->actor.velocity.z) > 0.1f) {

            f32 distXZ = Math_Vec3f_DistXZ(&this->actor.world.pos, &player->actor.world.pos);

            f32 leadFrames = distXZ / 4.0f;

            if (leadFrames > 20.0f) {
                leadFrames = 20.0f;
            }

            Vec3f predictedPos;
            predictedPos.x = player->actor.world.pos.x + (player->actor.velocity.x * leadFrames);
            predictedPos.y = player->actor.world.pos.y;
            predictedPos.z = player->actor.world.pos.z + (player->actor.velocity.z * leadFrames);

            targetYaw = Math_Vec3f_Yaw(&this->actor.world.pos, &predictedPos);
        }
    }

    this->actor.shape.rot.x += (s16)(0x140 * speedMult);
    this->actor.world.pos.y += 10.0f * speedMult;

    Math_ApproachS(&this->actor.shape.rot.y, targetYaw, 3, (s16)(0xAAA * speedMult));
    Math_StepToS(&this->unk_192, 1200, (s16)(100 * speedMult));
}

// Handling attacking types and their effects as well as defense boosts
RECOMP_HOOK("EnFloormas_Update") void FMUpdate(Actor* thisx, PlayState* play) {
    EnFloormas* this = (EnFloormas*)thisx;
    int Difficulty = (int)recomp_get_config_double("diff_option");

    if (this->actor.home.rot.z == 0) {
        this->actor.home.rot.z = (s16)(Rand_ZeroFloat(4.0f)) + 1;
    }

    switch (Difficulty) {
    case 0:
        this->actor.colChkInfo.damage = (this->actor.colChkInfo.damage) / 2;
        break;

    case 1:
        this->actor.colChkInfo.damage = (this->actor.colChkInfo.damage + 2) / 3;

        if (this->collider.base.atFlags & AT_HIT) {
            Player* player = GET_PLAYER(play);
            int attackType = this->actor.home.rot.z;

            if (attackType == 2) {
                func_80833B18(play, player, 3, 0.0f, 0.0f, 0, 20);
            }
            else if (attackType == 3) {
                func_80833B18(play, player, 4, 0.0f, 0.0f, 0, 20);
            }
            else if (attackType == 4) {
                gSaveContext.jinxTimer = 200;
            }

            this->collider.base.atFlags &= ~AT_HIT;
        }
        break;

    default:
        break;
    }
}

static Color_RGBA8 D_808D3958_Green = { 0, 255, 0, 0 };
static Color_RGBA8 D_808D3958_Blue = { 100, 200, 255, 0 };
static Color_RGBA8 D_808D3958_Yellow = { 255, 255, 0, 0 };
static Color_RGBA8 D_808D3958_Purple = { 200, 0, 255, 0 };

// Handling the color of the attack types when attacking
RECOMP_PATCH void EnFloormas_Draw(Actor* thisx, PlayState* play) {
    EnFloormas* this = (EnFloormas*)thisx;
    int Difficulty = (int)recomp_get_config_double("diff_option");

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Opa(play->state.gfxCtx);

    if (this->collider.base.colMaterial == COL_MATERIAL_HARD) {
        Color_RGBA8* activeColor = &D_808D3958_Green;

        if (Difficulty == 1) {
            if (this->actor.home.rot.z == 2) {
                activeColor = &D_808D3958_Blue;
            }
            else if (this->actor.home.rot.z == 3) {
                activeColor = &D_808D3958_Yellow;
            }
            else if (this->actor.home.rot.z == 4) {
                activeColor = &D_808D3958_Purple;
            }
        }

        func_800AE2A0(play, activeColor, this->unk_190 % 40, 40);
    }

    POLY_OPA_DISP =
        SkelAnime_DrawFlex(play, this->skelAnime.skeleton, this->skelAnime.jointTable, this->skelAnime.dListCount,
            EnFloormas_OverrideLimbDraw, EnFloormas_PostLimbDraw, &this->actor, POLY_OPA_DISP);

    if (this->collider.base.colMaterial == COL_MATERIAL_HARD) {
        func_800AE5A0(play);
    }

    CLOSE_DISPS(play->state.gfxCtx);

    Actor_DrawDamageEffects(play, &this->actor, this->bodyPartsPos, ENFLOORMAS_BODYPART_MAX,
        100.0f * (this->drawDmgEffScale * this->actor.scale.x),
        100.0f * (this->drawDmgEffFrozenSteamScale * this->actor.scale.x), this->drawDmgEffAlpha,
        this->drawDmgEffType);
}