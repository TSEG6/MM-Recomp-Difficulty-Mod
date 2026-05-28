#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "assets/objects/gameplay_keep/gameplay_keep.h"
#include "z_en_bbfall.h"
#include "overlays/actors/ovl_En_Clear_Tag/z_en_clear_tag.h"
#include "gameplay_keep.h"

// I do not feel like removing extra ones today
void EnBbfall_SetupWaitForPlayer(EnBbfall* this);
void EnBbfall_WaitForPlayer(EnBbfall* this, PlayState* play);
void EnBbfall_SetupEmerge(EnBbfall* this);
void EnBbfall_Emerge(EnBbfall* this, PlayState* play);
void EnBbfall_SetupFly(EnBbfall* this);
void EnBbfall_Fly(EnBbfall* this, PlayState* play);
void EnBbfall_SetupSinkIntoLava(EnBbfall* this);
void EnBbfall_SinkIntoLava(EnBbfall* this, PlayState* play);
void EnBbfall_Down(EnBbfall* this, PlayState* play);
void EnBbfall_Dead(EnBbfall* this, PlayState* play);
void EnBbfall_Damage(EnBbfall* this, PlayState* play);
void EnBbfall_Frozen(EnBbfall* this, PlayState* play);

s32 EnBbfall_IsTouchingLava(EnBbfall* this, PlayState* play) {
    if (!SurfaceType_IsWallDamage(&play->colCtx, this->actor.floorPoly, this->actor.floorBgId)) {
        FloorType floorType = SurfaceType_GetFloorType(&play->colCtx, this->actor.floorPoly, this->actor.floorBgId);

        if ((floorType == FLOOR_TYPE_2) || (floorType == FLOOR_TYPE_3) || (floorType == FLOOR_TYPE_9)) {
            return true;
        }
    }

    return false;
}

void EnBbfall_CheckForWall(EnBbfall* this) {
    s16 yawDiff;

    if (this->actor.bgCheckFlags & BGCHECKFLAG_WALL) {
        yawDiff = this->actor.shape.rot.y - this->actor.wallYaw;

        if (ABS_ALT(yawDiff) > 0x4000) {
            this->actor.shape.rot.y = ((this->actor.wallYaw * 2) - this->actor.shape.rot.y) - 0x8000;
        }

        this->actor.bgCheckFlags &= ~BGCHECKFLAG_WALL;
    }
}

void EnBbfall_PlaySfx(EnBbfall* this) {
    if (Animation_OnFrame(&this->skelAnime, 0.0f) || Animation_OnFrame(&this->skelAnime, 5.0f)) {
        Actor_PlaySfx(&this->actor, NA_SE_EN_BUBLE_MOUTH);
    }

    Actor_PlaySfx_Flagged(&this->actor, NA_SE_EN_BUBLEFALL_FIRE - SFX_FLAG);
}

RECOMP_PATCH void EnBbfall_Fly(EnBbfall* this, PlayState* play) {
    SkelAnime_Update(&this->skelAnime);
    Math_StepToF(&this->flameScaleY, 0.8f, 0.1f);
    Math_StepToF(&this->flameScaleX, 1.0f, 0.1f);
    EnBbfall_CheckForWall(this);

    int Difficulty = (int)recomp_get_config_double("diff_option");

    if (this->actor.bgCheckFlags & BGCHECKFLAG_GROUND) {
        if (EnBbfall_IsTouchingLava(this, play)) {
            EnBbfall_SetupSinkIntoLava(this);
        }
        else {
            this->actor.velocity.y *= -1.2f;
            this->actor.velocity.y = CLAMP(this->actor.velocity.y, 8.0f, 12.0f);

            if (Difficulty == 1) {
                s16 yawToPlayer = Actor_WorldYawTowardActor(&this->actor, &GET_PLAYER(play)->actor);

                Math_ApproachS(&this->actor.shape.rot.y, yawToPlayer, 3, 0x2000);

                this->actor.shape.rot.y += TRUNCF_BINANG(Rand_CenteredFloat(0x4000));
            }
            else {
                this->actor.shape.rot.y += TRUNCF_BINANG(Rand_CenteredFloat(0x12000));
            }
        }

        this->actor.bgCheckFlags &= ~BGCHECKFLAG_GROUND;
    }

    this->actor.world.rot.y = this->actor.shape.rot.y;
    EnBbfall_PlaySfx(this);
}

RECOMP_HOOK("EnBbfall_Update") void FallUpdate(Actor* thisx, PlayState* play) {

	EnBbfall* this = (EnBbfall*)thisx;

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