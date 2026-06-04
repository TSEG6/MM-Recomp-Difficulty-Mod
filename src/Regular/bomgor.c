#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_en_baguo.h"
#include "overlays/actors/ovl_En_Clear_Tag/z_en_clear_tag.h"
#include "assets/objects/gameplay_keep/gameplay_keep.h"
#include "overlays/actors/ovl_En_Bom/z_en_bom.h"

void EnBaguo_UndergroundIdle(EnBaguo* this, PlayState* play);
void EnBaguo_Idle(EnBaguo* this, PlayState* play);
void EnBaguo_Roll(EnBaguo* this, PlayState* play);
void EnBaguo_SetupRetreatUnderground(EnBaguo* this);
void EnBaguo_DrawBody(Actor* thisx, PlayState* play);
void EnBaguo_InitializeEffect(EnBaguo* this, Vec3f* pos, Vec3f* velocity, Vec3f* accel, f32 scale, s16 timer);

typedef enum {
    /* 0 */ NEJIRON_ACTION_INACTIVE,   // The Nejiron is either underground or emerging from underground
    /* 1 */ NEJIRON_ACTION_ACTIVE,     // The Nejiron is above ground and actively chasing the player
    /* 2 */ NEJIRON_ACTION_RETREATING, // The Nejiron is burrowing back underground
    /* 3 */ NEJIRON_ACTION_EXPLODING   // The Nejiron has detonated
} NejironAction;

typedef enum {
    /* 0 */ NEJIRON_DIRECTION_RIGHT,
    /* 1 */ NEJIRON_DIRECTION_LEFT
} NejironRollDirection;

typedef enum {
    /* 0x0 */ NEJIRON_DMGEFF_NONE,      // Does not interact with the Nejiron at all
    /* 0xE */ NEJIRON_DMGEFF_KILL = 14, // Kills and detonates the Nejiron
    /* 0xF */ NEJIRON_DMGEFF_RECOIL     // Deals no damage, but displays the appropriate hit mark and recoil animation
} NejironDamageEffect;

extern void EnBaguo_PostDetonation(EnBaguo*, PlayState*);


// Allows it to turn faster and predict player movement
RECOMP_PATCH void EnBaguo_Idle(EnBaguo* this, PlayState* play) {
    s16 absoluteYaw;
    s16 yaw;
    s16 targetYaw;

    int Difficulty = (int)recomp_get_config_double("diff_option");

    targetYaw = this->actor.yawTowardsPlayer;

    if (Difficulty >= 1) {
        Player* player = GET_PLAYER(play);

        float framesToReach = this->actor.xzDistToPlayer / 8.0f;

        if (framesToReach > 40.0f) {
            framesToReach = 40.0f;
        }

        Vec3f predictedPos;
        predictedPos.x = player->actor.world.pos.x + (Math_SinS(player->actor.world.rot.y) * player->actor.speed * framesToReach);
        predictedPos.y = player->actor.world.pos.y;
        predictedPos.z = player->actor.world.pos.z + (Math_CosS(player->actor.world.rot.y) * player->actor.speed * framesToReach);
        targetYaw = Math_Vec3f_Yaw(&this->actor.world.pos, &predictedPos);
    }

    int timerDrain = 0;
    if (Difficulty == 0) {
        timerDrain = 1;
    }
    else if (Difficulty >= 1) {
        timerDrain = 2;
    }

    if (this->timer > timerDrain) {
        this->timer -= timerDrain;
    }

    if (this->timer != 0) {
        Math_SmoothStepToS(&this->actor.world.rot.x, 0, 10, 0x64, 0x3E8);
        Math_SmoothStepToS(&this->actor.world.rot.z, 0, 10, 0x64, 0x3E8);

        if ((this->timer & 8) != 0) {
            if (fabsf(this->actor.world.rot.y - targetYaw) > 200.0f) {

                s16 turnScale = 30;
                s16 turnStepMax = 0x12C;
                int sfx = 8;

                if (Difficulty == 0) {
                    turnScale = 3;
                    turnStepMax = 0x1000;
                    sfx = 4;
                }
                else if (Difficulty >= 1) {
                    turnScale = 4;
                    turnStepMax = 0x2500;
                    sfx = 2;
                }

                Math_SmoothStepToS(&this->actor.world.rot.y, targetYaw, turnScale, turnStepMax, 0x3E8);

                if ((play->gameplayFrames % sfx) == 0) {
                    Actor_SpawnFloorDustRing(play, &this->actor, &this->actor.world.pos,
                        this->actor.shape.shadowScale - 20.0f, 10, 8.0f, 500, 10, true);
                    Actor_PlaySfx(&this->actor, NA_SE_EN_BAKUO_VOICE);
                }
            }
        }
        this->actor.shape.rot.y = this->actor.world.rot.y;
    }
    else {
        yaw = targetYaw - this->actor.world.rot.y;
        absoluteYaw = ABS_ALT(yaw);
        Math_Vec3f_Copy(&this->targetRotation, &gZeroVec3f);
        Math_Vec3f_Copy(&this->currentRotation, &gZeroVec3f);

        if (absoluteYaw < 0x2000) {
            this->targetRotation.x = 2000.0f;
        }
        else {
            this->zRollDirection = NEJIRON_DIRECTION_RIGHT;
            this->targetRotation.z = 2000.0f;
            if ((s16)(targetYaw - this->actor.world.rot.y) > 0) {
                this->zRollDirection = NEJIRON_DIRECTION_LEFT;
            }
        }

        this->timer = 38;
        this->actor.world.rot.y = targetYaw;
        this->actor.shape.rot.y = targetYaw;
        this->bouncedFlag = 0;
        this->actionFunc = EnBaguo_Roll;
    }
}

// More prediction and actually rolling
RECOMP_PATCH void EnBaguo_Roll(EnBaguo* this, PlayState* play) {
    f32 xDistanceFromHome = this->actor.home.pos.x - this->actor.world.pos.x;
    f32 zDistanceFromHome = this->actor.home.pos.z - this->actor.world.pos.z;

    int Difficulty = (int)recomp_get_config_double("diff_option");

    if ((sqrtf(SQ(xDistanceFromHome) + SQ(zDistanceFromHome)) > this->maxDistanceFromHome) ||
        (Player_GetMask(play) == PLAYER_MASK_STONE)) {
        EnBaguo_SetupRetreatUnderground(this);
    }
    else if (this->timer == 0) {
        this->timer = 100;
        this->actor.world.rot.y = this->actor.shape.rot.y;
        this->actionFunc = EnBaguo_Idle;
        this->actor.speed = 0.0f;
    }
    else {
        if (!this->bouncedFlag && (this->collider.base.atFlags & AT_BOUNCED)) {
            this->zRollDirection ^= 1;
            this->bouncedFlag = 1;
            this->actor.speed = -7.0f;
        }

        Math_ApproachF(&this->currentRotation.x, this->targetRotation.x, 0.2f, 1000.0f);
        Math_ApproachF(&this->currentRotation.z, this->targetRotation.z, 0.2f, 1000.0f);

        switch (Difficulty) {
        case 0:
            Math_ApproachF(&this->actor.speed, 6.5f, 0.4f, 0.75f);
            break;

        case 1:
            Math_ApproachF(&this->actor.speed, 8.0f, 0.6f, 1.0f);
            break;

        default:
            break;
        }
        this->actor.world.rot.x += TRUNCF_BINANG(this->currentRotation.x);

        if (this->currentRotation.z != 0.0f) {
            if (this->zRollDirection == NEJIRON_DIRECTION_RIGHT) {
                this->actor.world.rot.z += TRUNCF_BINANG(this->currentRotation.z);
            }
            else {
                this->actor.world.rot.z -= TRUNCF_BINANG(this->currentRotation.z);
            }
        }

        Actor_PlaySfx(&this->actor, NA_SE_EN_BAKUO_ROLL - SFX_FLAG);
    }
}

// Fixes a bug where you could see the legs still after going back underground
RECOMP_PATCH void EnBaguo_RetreatUnderground(EnBaguo* this, PlayState* play) {

    this->actor.world.rot.y -= 0x1518;
    this->actor.shape.rot.y = this->actor.world.rot.y;
    if ((play->gameplayFrames % 8) == 0) {
        Actor_SpawnFloorDustRing(play, &this->actor, &this->actor.world.pos, this->actor.shape.shadowScale - 20.0f, 10,
            8.0f, 500, 10, true);
    }

    Math_ApproachF(&this->actor.shape.yOffset, -3000.0f, 100.0f, 500.0f);
    Math_ApproachZeroF(&this->actor.shape.shadowScale, 0.3f, 5.0f);

    if (this->actor.shape.yOffset < -2970.0f) {
        this->actor.shape.yOffset = -3000.0f;
        this->actor.draw = NULL;
        Math_Vec3f_Copy(&this->actor.world.pos, &this->actor.home.pos);
        Actor_PlaySfx(&this->actor, NA_SE_EN_BAKUO_APPEAR);
        this->actor.flags |= ACTOR_FLAG_LOCK_ON_DISABLED;
        this->actor.flags &= ~ACTOR_FLAG_ATTENTION_ENABLED;
        this->actionFunc = EnBaguo_UndergroundIdle;
    }
}

// Appear from the depths
RECOMP_PATCH void EnBaguo_EmergeFromUnderground(EnBaguo* this, PlayState* play) {
    this->actor.draw = EnBaguo_DrawBody;

    this->actor.world.rot.y += 0x1518;
    this->actor.shape.rot.y = this->actor.world.rot.y;
    if ((play->gameplayFrames % 8) == 0) {
        Actor_SpawnFloorDustRing(play, &this->actor, &this->actor.world.pos, this->actor.shape.shadowScale - 20.0f, 10,
            8.0f, 500, 10, true);
    }
    Math_ApproachF(&this->actor.shape.shadowScale, 50.0f, 0.3f, 5.0f);
    Math_ApproachF(&this->actor.shape.yOffset, 2700.0f, 100.0f, 500.0f);
    if (this->actor.shape.yOffset > 2650.0f) {
        this->action = NEJIRON_ACTION_ACTIVE;
        this->actor.shape.yOffset = 2700.0f;
        this->timer = 60;
        this->actionFunc = EnBaguo_Idle;
    }
}

// Actually spawn a bomb when exploding instead of using a fake explosion (useful for destorying the rocks nearby on the way to ikana)
RECOMP_PATCH void EnBaguo_CheckForDetonation(EnBaguo* this, PlayState* play) {
    EnBom* bomb;
    s32 i;

    // In order to match, this variable must act as both a boolean to check if
    // the Nejiron should forcibly explode and as a loop index.
    i = false;

    if (this->action != NEJIRON_ACTION_EXPLODING && this->action != NEJIRON_ACTION_RETREATING) {
        if (!(this->actor.bgCheckFlags & BGCHECKFLAG_GROUND) &&
            (this->actor.world.pos.y < (this->actor.home.pos.y - 100.0f))) {
            // Force a detonation if we're off the ground and have fallen
            // below our home position (e.g., we rolled off a ledge)
            i = true;
        }

        if ((this->actor.bgCheckFlags & (BGCHECKFLAG_WATER | BGCHECKFLAG_WATER_TOUCH)) &&
            (this->actor.depthInWater >= 40.0f)) {
            // Force a detonation if we're too far below the water's surface.
            i = true;
        }

        if ((this->collider.base.acFlags & AC_HIT) || i) {
            this->collider.base.acFlags &= ~AC_HIT;

            if (i || (this->actor.colChkInfo.damageEffect == NEJIRON_DMGEFF_KILL)) {
                Actor_SetColorFilter(&this->actor, COLORFILTER_COLORFLAG_RED, 255, COLORFILTER_BUFFLAG_OPA, 8);
                this->action = NEJIRON_ACTION_EXPLODING;
                this->actor.speed = 0.0f;
                this->actor.shape.shadowScale = 0.0f;

                bomb = (EnBom*)Actor_Spawn(
                    &play->actorCtx, play, ACTOR_EN_BOM,
                    this->actor.world.pos.x, this->actor.world.pos.y, this->actor.world.pos.z,
                    0, 0, 0, BOMB_TYPE_BODY
                );
                if (bomb != NULL) {
                    bomb->timer = 0;
                }

                Actor_Spawn(&play->actorCtx, play, ACTOR_EN_CLEAR_TAG,
                    this->actor.world.pos.x, this->actor.world.pos.y, this->actor.world.pos.z,
                    0, 0, 0, CLEAR_TAG_PARAMS(CLEAR_TAG_POP));
                Actor_PlaySfx(&this->actor, NA_SE_EN_BAKUO_DEAD);

                this->timer = 30;
                this->actor.flags |= ACTOR_FLAG_LOCK_ON_DISABLED;
                this->actor.flags &= ~ACTOR_FLAG_ATTENTION_ENABLED;
                Actor_SetScale(&this->actor, 0.0f);
                this->collider.elements[0].dim.scale = 3.0f;
                this->collider.elements[0].base.atDmgInfo.damage = 8;
                Item_DropCollectibleRandom(play, NULL, &this->actor.world.pos, 0xB0);
                this->actionFunc = EnBaguo_PostDetonation;
            }
        }
    }
}