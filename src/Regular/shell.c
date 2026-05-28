
// A real shame I had to use a lot of patches...

#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_en_sb.h"
#include "overlays/actors/ovl_En_Part/z_en_part.h"

void EnSb_SetupWaitClosed(EnSb* this);
void EnSb_Open(EnSb* this, PlayState* play);
void EnSb_WaitOpen(EnSb* this, PlayState* play);
void EnSb_Lunge(EnSb* this, PlayState* play);

RECOMP_PATCH void EnSb_SetupOpen(EnSb* this) {
    Animation_Change(&this->skelAnime, &object_sb_Anim_000194, 1.0f, 0, Animation_GetLastFrame(&object_sb_Anim_000194),
        ANIMMODE_ONCE, 0.0f);
    this->state = SHELLBLADE_OPEN;
    this->actionFunc = EnSb_Open;
    Actor_PlaySfx(&this->actor, NA_SE_EN_MIZUBABA1_MOUTH);
}

void EnSb_SetupLunge(EnSb* this) {
    f32 endFrame = Animation_GetLastFrame(&object_sb_Anim_000124);
    f32 playbackSpeed = this->actor.depthInWater > 0.0f ? 1.0f : 0.0f;

    Animation_Change(&this->skelAnime, &object_sb_Anim_000124, playbackSpeed, 0.0f, endFrame, ANIMMODE_ONCE, 0);
    this->state = SHELLBLADE_LUNGE;
    this->actionFunc = EnSb_Lunge;
    Actor_PlaySfx(&this->actor, NA_SE_EN_DEKU_MOUTH);
}

void EnSb_SpawnBubbles(PlayState* play, EnSb* this) {
    s32 bubbleCount;

    if (this->actor.depthInWater > 0.0f) {
        for (bubbleCount = 0; bubbleCount < 10; bubbleCount++) {
            EffectSsBubble_Spawn(play, &this->actor.world.pos, 10.0f, 10.0f, 30.0f, 0.25f);
        }
    }
}

void EnSb_SetupWaitOpen(EnSb* this) {
    Animation_Change(&this->skelAnime, &object_sb_Anim_002C8C, 1.0f, 0, Animation_GetLastFrame(&object_sb_Anim_002C8C),
        ANIMMODE_LOOP, 0.0f);
    this->state = SHELLBLADE_WAIT_OPEN;
    this->actionFunc = EnSb_WaitOpen;
}

RECOMP_PATCH void EnSb_Open(EnSb* this, PlayState* play) {
    f32 curFrame = this->skelAnime.curFrame;
    f32 endFrame = Animation_GetLastFrame(&object_sb_Anim_000194);

    int Difficulty = (int)recomp_get_config_double("diff_option");
    int openTimer = 20;

    switch (Difficulty) {
    case 0:
        openTimer = 15;
        break;

    case 1:
        openTimer = 10;
        break;

    default:
        break;
    }

    if (curFrame >= endFrame) {
        this->vulnerableTimer = openTimer;
        EnSb_SetupWaitOpen(this);
    }
    else {
        Math_SmoothStepToS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 0xA, 0x7D0, 0);
        if ((this->actor.xzDistToPlayer > 240.0f) || (this->actor.xzDistToPlayer <= 40.0f)) {
            this->vulnerableTimer = 0;
            EnSb_SetupWaitClosed(this);
        }
    }
}

RECOMP_PATCH void EnSb_TurnAround(EnSb* this, PlayState* play) {
    s16 invertedYaw = BINANG_ROT180(this->yawAngle);
    float waterSpeed;
    float airSpeed;

    int Difficulty = (int)recomp_get_config_double("diff_option");

    Math_SmoothStepToS(&this->actor.shape.rot.y, invertedYaw, 1, 0x1F40, 0xA);
    if (this->actor.shape.rot.y == invertedYaw) {
        this->actor.world.rot.y = this->yawAngle;

        if (Difficulty == 1) {
            waterSpeed = 9.0f;
            airSpeed = 12.0f;
        }
        else {
            waterSpeed = 6.5f;
            airSpeed = 8.0f;
        }
        if (this->actor.depthInWater > 0.0f) {
            this->actor.velocity.y = 3.0f;
            this->actor.speed = waterSpeed;
            this->actor.gravity = -0.35f;
        }
        else {
            this->actor.velocity.y = 2.0f;
            this->actor.speed = airSpeed;
            this->actor.gravity = -2.0f;
        }
        EnSb_SpawnBubbles(play, this);
        this->bounceCounter = 3;
        EnSb_SetupLunge(this);
    }
}

RECOMP_PATCH void EnSb_Bounce(EnSb* this, PlayState* play) {
    s32 pad;
    f32 curFrame = this->skelAnime.curFrame;
    f32 endFrame = Animation_GetLastFrame(&object_sb_Anim_0000B4);
    float waterSpeed;
    float airSpeed;
    s16 turnStep;
    s16 targetVisualYaw;

    int Difficulty = (int)recomp_get_config_double("diff_option");

    if (Difficulty == 1) {
        turnStep = 0x0A00;
    }
    else {
        turnStep = 0x0400;
    }

    targetVisualYaw = BINANG_ROT180(this->actor.yawTowardsPlayer);
    Math_SmoothStepToS(&this->actor.shape.rot.y, targetVisualYaw, 3, turnStep, 0);

    Math_StepToF(&this->actor.speed, 0.0f, 0.2f);

    if (curFrame == endFrame) {
        if (this->bounceCounter != 0) {
            this->bounceCounter--;
            this->attackTimer = 1;

            this->actor.world.rot.y = BINANG_ROT180(this->actor.shape.rot.y);

            if (Difficulty == 1) {
                waterSpeed = 9.0f;
                airSpeed = 12.0f;
            }
            else {
                waterSpeed = 6.5f;
                airSpeed = 8.0f;
            }

            if (this->actor.depthInWater > 0.0f) {
                this->actor.velocity.y = 3.0f;
                this->actor.speed = waterSpeed;
                this->actor.gravity = -0.35f;
            }
            else {
                this->actor.velocity.y = 2.0f;
                this->actor.speed = airSpeed;
                this->actor.gravity = -2.0f;
            }

            EnSb_SpawnBubbles(play, this);
            EnSb_SetupLunge(this);
        }
        else if (this->actor.bgCheckFlags & BGCHECKFLAG_GROUND) {
            this->actor.bgCheckFlags &= ~BGCHECKFLAG_GROUND_TOUCH;
            this->actor.speed = 0.0f;
            this->attackTimer = 1;
            EnSb_SetupWaitClosed(this);
        }
    }
}