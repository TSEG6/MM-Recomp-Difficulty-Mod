#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_en_bigslime.h"
#include "z64quake.h"
#include "z64rumble.h"
#include "overlays/actors/ovl_En_Clear_Tag/z_en_clear_tag.h"
#include "overlays/effects/ovl_Effect_Ss_Hahen/z_eff_ss_hahen.h"
#include "assets/objects/object_bigslime/object_bigslime.h"
#include "assets/objects/gameplay_keep/gameplay_keep.h"
#include "z_en_minislime.h"
#include "overlays/actors/ovl_En_Bigslime/z_en_bigslime.h"

void EnBigslime_SetupDrop(EnBigslime* this);
void EnBigslime_Drop(EnBigslime* this, PlayState* play);
void EnBigslime_SetupThrowMinislime(EnBigslime* this);
void EnBigslime_JumpGekko(EnBigslime* this, PlayState* play);
void EnBigslime_SetupIdleLookAround(EnBigslime* this);
void EnBigslime_SetupCutscene(EnBigslime* this);

void EnBigslime_GekkoSfxOutsideBigslime(EnBigslime* this, u16 sfxId) {
    Audio_PlaySfx_AtPos(&this->gekkoProjectedPos, sfxId);
}

int RestartTimer = 20;
int callCount = 0;
bool HasCalledRecently = false;

RECOMP_HOOK("EnMinislime_Destroy") void ResetCalls(Actor* thisx, PlayState* play) {

    callCount = 0;

}

RECOMP_PATCH void EnBigslime_SetupDrop(EnBigslime* this) {

    int Difficulty = (int)recomp_get_config_double("diff_option");

    float DropDist = 100.0f;

    switch (Difficulty) {
    case 0:
        DropDist = 150.0f;
        break;
    case 1:
        DropDist = 75.0f;
        break;
    default:
        break;
    }

    if ((this->actor.xzDistToPlayer > DropDist) && (this->ceilingMoveTimer != 0)) {

        s16 pitch = this->ceilingMoveTimer * 0x800;
        switch (Difficulty) {
        case 0:
            this->actor.speed = (fabsf(Math_SinS(pitch)) * 6.0f) + 7.0f;
            break;
        case 1:
            this->actor.speed = (fabsf(Math_SinS(pitch)) * 7.0f) + 9.0f;
            break;
        default:
            break;
        }
        Math_SmoothStepToS(&this->actor.world.rot.y, this->actor.yawTowardsPlayer, 10, 0x800, 0x80);
        this->gekkoRot.y = this->actor.world.rot.y;
        return;
    }
    this->actor.velocity.y = 0.0f;
    this->ceilingDropTimer = 30;
    this->actor.speed = 0.0f;
    this->actionFunc = EnBigslime_Drop;
    HasCalledRecently = false;
}

RECOMP_HOOK("EnBigslime_DamageGekko") void DamageTimerReset(EnBigslime* this, PlayState* play) {

    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        RestartTimer = 160 - callCount;
        break;
    case 1:
        RestartTimer = 100 - callCount;
        break;
    default:
        break;
    }

    if (RestartTimer < 0) {
        RestartTimer = 0;
    }
    if (!HasCalledRecently) {
        if (callCount < 20) {
            callCount++;
            HasCalledRecently = true;
        }
    }
}

RECOMP_HOOK("EnBigslime_JumpGekko") void FrogJump(EnBigslime* this, PlayState* play) {

    int Difficulty = (int)recomp_get_config_double("diff_option");
    float newSpeed = 8.0f;

    switch (Difficulty) {
    case 0:
        newSpeed = 12.0f;
        this->jumpTimer--;
        break;
    case 1:
        newSpeed = 16.0f;
        this->jumpTimer = this->jumpTimer - 2;
        break;
    default:
        break;
    }

    if (!(this->actor.bgCheckFlags & BGCHECKFLAG_GROUND) ||
        ((this->skelAnime.curFrame > 1.0f) && (this->skelAnime.curFrame < 12.0f))) {
        this->actor.speed = newSpeed;
    }
    else {
        this->actor.speed = 0.0f;
    }
    if (this->jumpTimer < 0) {
        this->jumpTimer = 0;
    }

    if (RestartTimer <= 0) {
        EnBigslime_SetupCutscene(this);
        if (callCount < 20) callCount++;
        switch (Difficulty) {
        case 0:
            RestartTimer = 160 - callCount;
            break;
        case 1:
            RestartTimer = 100 - callCount;
            break;
        default:
            break;
        }
    }
    else {
        if (this->actionFunc == EnBigslime_JumpGekko) RestartTimer--;
    }
}

RECOMP_HOOK("EnBigslime_MoveOnCeiling") void FrogRoofMove(EnBigslime* this, PlayState* play) {

    s16 pitch; // polar (zenith) angle

    int Difficulty = (int)recomp_get_config_double("diff_option");

    pitch = this->ceilingMoveTimer * 0x800;

    if (this->subCamId != SUB_CAM_ID_DONE) {
        if (this->ceilingMoveTimer == 0) {
        }
    }
    else if ((this->actor.xzDistToPlayer < 250.0f) || (this->ceilingMoveTimer == 0)) {
        EnBigslime_SetupDrop(this);
    }
    else {
        switch (Difficulty) {
        case 0:
            this->actor.speed = (fabsf(Math_SinS(pitch)) * 6.0f) + 7.0f;
            break;
        case 1:
            this->actor.speed = (fabsf(Math_SinS(pitch)) * 7.0f) + 9.0f;
            break;
        default:
            break;
        }
    }
}

RECOMP_HOOK("EnBigslime_UpdateGekko") void UpdateFrogGBT(Actor* thisx, PlayState* play) {

    EnBigslime* this = (EnBigslime*)thisx;

	int Difficulty = (int)recomp_get_config_double("diff_option");

	#define HEALTH_SCALED_FLAG 0x8000

    if (!(this->actor.params & HEALTH_SCALED_FLAG)) {
        switch (Difficulty) {
        case 0:
            this->actor.colChkInfo.health = (this->actor.colChkInfo.health * 1.25f);
            RestartTimer = 160;
            break;
        case 1:
            this->actor.colChkInfo.health = (this->actor.colChkInfo.health * 1.5f);
            RestartTimer = 100;
            break;
        default:
            break;
        }
        this->actor.params |= HEALTH_SCALED_FLAG;
    }
}

void EnMinislime_SetupGrowAndShrink(EnMinislime* this);
void EnMinislime_SetupMoveToGekko(EnMinislime* this);

RECOMP_PATCH void EnMinislime_Bounce(EnMinislime* this, PlayState* play) {

    int Difficulty = (int)recomp_get_config_double("diff_option");

    if (this->actor.params == MINISLIME_SETUP_GEKKO_THROW) {
        EnMinislime_SetupMoveToGekko(this);
    }
    else {
        if (this->bounceTimer > 0) {
            this->bounceTimer--;
            if (this->bounceTimer == 0) {

                this->actor.gravity = -2.0f;
                this->actor.world.rot.y = this->actor.yawTowardsPlayer;

                if (Difficulty == 1) {
                    this->actor.speed = 3.0f;
                }
                else if (Difficulty == 0) {
                    this->actor.speed = 2.0f;
                }
                else {
                    this->actor.speed = 1.0f;
                }

                this->actor.velocity.y = 12.0f;
                this->actor.shape.rot.y = this->actor.world.rot.y;
            }
            Math_StepToF(&this->actor.scale.x, 0.17999999f, 0.010000001f);
            Math_StepToF(&this->actor.scale.y, 0.05f, 0.010000001f);
        }
        else if (this->actor.velocity.y > 0.0f) {
            Math_StepToF(&this->actor.scale.x, 0.095f, 0.020000001f);
            Math_StepToF(&this->actor.scale.y, 0.10700001f, 0.020000001f);
        }
        else {
            Math_StepToF(&this->actor.scale.x, 0.17999999f, 0.003f);
            Math_StepToF(&this->actor.scale.y, 0.05f, 0.003f);
            if (this->actor.bgCheckFlags & BGCHECKFLAG_GROUND) {
                EnMinislime_SetupGrowAndShrink(this);
                return;
            }
        }
        this->actor.scale.z = this->actor.scale.x;
    }
}