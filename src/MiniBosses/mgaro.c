#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_en_jso2.h"

#include "z64olib.h"

#include "assets/objects/gameplay_keep/gameplay_keep.h"
#include "overlays/actors/ovl_En_Clear_Tag/z_en_clear_tag.h"
#include "overlays/actors/ovl_En_Col_Man/z_en_col_man.h"

void EnJso2_SetupSlash(EnJso2* this, PlayState* play);
void EnJso2_Stunned(EnJso2* this, PlayState* play);
void EnJso2_SetupJumpBack(EnJso2* this);
void EnJso2_SetupTeleport(EnJso2* this);
void EnJso2_FallFromTeleport(EnJso2* this, PlayState* play);
void EnJso2_SetupWaitAfterSlash(EnJso2* this);

int TimesTPd = 0;

typedef enum EnJso2Action {
    /*  0 */ EN_JSO2_ACTION_INTRO_CUTSCENE,
    /*  1 */ EN_JSO2_ACTION_UNK_1, // Checked in EnJso2_Update, but never actually used
    /*  2 */ EN_JSO2_ACTION_UNK_2, // Checked in EnJso2_UpdateDamage, but never actually used
    /*  3 */ EN_JSO2_ACTION_CIRCLE_PLAYER,
    /*  4 */ EN_JSO2_ACTION_GUARD,
    /*  5 */ EN_JSO2_ACTION_SPIN_BEFORE_ATTACK,
    /*  6 */ EN_JSO2_ACTION_DASH_ATTACK,
    /*  7 */ EN_JSO2_ACTION_SLASH,
    /*  8 */ EN_JSO2_ACTION_SPIN_ATTACK,
    /*  9 */ EN_JSO2_ACTION_WAIT_AFTER_SLASH,
    /* 10 */ EN_JSO2_ACTION_STUNNED,
    /* 11 */ EN_JSO2_ACTION_DAMAGED,
    /* 12 */ EN_JSO2_ACTION_JUMP_BACK,
    /* 13 */ EN_JSO2_ACTION_DEAD,
    /* 14 */ EN_JSO2_ACTION_BLOW_UP,
    /* 15 */ EN_JSO2_ACTION_TELEPORT,
    /* 16 */ EN_JSO2_ACTION_FALL_FROM_TELEPORT
} EnJso2Action;

typedef enum EnJso2Animation {
    /*  0 */ EN_JSO2_ANIM_DASH_ATTACK,
    /*  1 */ EN_JSO2_ANIM_SLASH_START,
    /*  2 */ EN_JSO2_ANIM_SLASH_LOOP,
    /*  3 */ EN_JSO2_ANIM_JUMP_BACK,
    /*  4 */ EN_JSO2_ANIM_DAMAGED,
    /*  5 */ EN_JSO2_ANIM_GUARD,
    /*  6 */ EN_JSO2_ANIM_APPEAR, // unused
    /*  7 */ EN_JSO2_ANIM_IDLE,   // unused
    /*  8 */ EN_JSO2_ANIM_BOUNCE,
    /*  9 */ EN_JSO2_ANIM_FALL_DOWN,    // unused
    /* 10 */ EN_JSO2_ANIM_KNOCKED_BACK, // unused
    /* 11 */ EN_JSO2_ANIM_COWER,        // unused
    /* 12 */ EN_JSO2_ANIM_LOOK_AROUND,
    /* 13 */ EN_JSO2_ANIM_APPEAR_AND_DRAW_SWORDS,
    /* 14 */ EN_JSO2_ANIM_SPIN_ATTACK,
    /* 15 */ EN_JSO2_ANIM_LAND,
    /* 16 */ EN_JSO2_ANIM_JUMP_DOWN,
    /* 17 */ EN_JSO2_ANIM_LAUGH,
    /* 18 */ EN_JSO2_DRAW_SWORDS,
    /* 19 */ EN_JSO2_COLLAPSE,
    /* 20 */ EN_JSO2_TREMBLE,
    /* 21 */ EN_JSO2_TAKE_OUT_BOMB,
    /* 22 */ EN_JSO2_ANIM_MAX
} EnJso2Animation;

extern void EnJso2_ChangeAnim(EnJso2*, s32);

RECOMP_PATCH void EnJso2_SetupStunned(EnJso2* this) {
    Vec3f knockbackVelocity;

    int Difficulty = (int)recomp_get_config_double("diff_option");
    int StunTimer = 40;

    this->actor.speed = 0.0f;

    switch (Difficulty) {
    case 0:
        StunTimer = 20;
        break;

    case 1:
        StunTimer = 10;
        break;

    default:
        break;
    }

    AudioSfx_SetChannelIO(&this->actor.projectedPos, NA_SE_EN_ANSATSUSYA_DASH_2, 0);
    EnJso2_ChangeAnim(this, EN_JSO2_ANIM_DAMAGED);
    this->timer = StunTimer;
    this->actor.colorFilterTimer = StunTimer;
    this->bodyCollider.base.acFlags &= ~AC_HARD;
    this->actor.speed = 0.0f;
    Matrix_RotateYS(this->actor.yawTowardsPlayer, MTXMODE_NEW);
    Matrix_MultVecZ(-10.0f, &knockbackVelocity);
    Math_Vec3f_Copy(&this->knockbackVelocity, &knockbackVelocity);

    if (((this->drawDmgEffType == ACTOR_DRAW_DMGEFF_FROZEN_SFX) ||
        (this->drawDmgEffType == ACTOR_DRAW_DMGEFF_FROZEN_NO_SFX)) &&
        (this->drawDmgEffAlpha == 0)) {
        this->drawDmgEffAlpha = 0;
        this->drawDmgEffType = ACTOR_DRAW_DMGEFF_FIRE;
        this->actor.colorFilterTimer = StunTimer;
    }

    if ((this->drawDmgEffType != ACTOR_DRAW_DMGEFF_FROZEN_SFX) &&
        (this->drawDmgEffType != ACTOR_DRAW_DMGEFF_FROZEN_NO_SFX)) {
        this->timer = StunTimer * 2;
        this->actor.colorFilterTimer = StunTimer;
    }

    this->action = EN_JSO2_ACTION_STUNNED;
    this->actionFunc = EnJso2_Stunned;
}

RECOMP_HOOK_RETURN("EnJso2_SetupDashAttack") void IncreaseSpeedForAttackGM(EnJso2* this) {

    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        this->actor.speed = 17.5f;
        break;

    case 1:
        this->actor.speed = 20.0f;
        break;

    default:
        break;
    }
}

RECOMP_HOOK_RETURN("EnJso2_SetupTeleport") void TeleportFaster(EnJso2* this) {

    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        this->attackMovementTimer = 15;
        break;

    case 1:
        this->attackMovementTimer = 7;
        break;

    default:
        break;
    }
}

RECOMP_PATCH void EnJso2_SetupFallFromTeleport(EnJso2* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    int Difficulty = (int)recomp_get_config_double("diff_option");
    float predictionFrames = 12.0f;

    switch (Difficulty) {
    case 0:
        predictionFrames = 6.0f;
        break;

    case 1:
        predictionFrames = 12.0f;
        break;

    default:
        break;
    }

    this->actor.world.pos.x = player->actor.world.pos.x + (player->actor.velocity.x * predictionFrames);
    this->actor.world.pos.z = player->actor.world.pos.z + (player->actor.velocity.z * predictionFrames);
    this->actor.world.pos.y = player->actor.world.pos.y + 300.0f + BREG(52);

    this->actor.velocity.y = 0.0f;
    this->actor.gravity = BREG(53) + -3.0f;

    s16 yawToPlayer = Math_Vec3f_Yaw(&this->actor.world.pos, &player->actor.world.pos);
    this->actor.world.rot.y = yawToPlayer;
    this->actor.shape.rot.y = yawToPlayer;

    Actor_PlaySfx(&this->actor, NA_SE_EN_ANSATSUSYA_FALL);
    this->action = EN_JSO2_ACTION_FALL_FROM_TELEPORT;
    this->actionFunc = EnJso2_FallFromTeleport;
}

RECOMP_HOOK("EnJso2_FallFromTeleport") void HelpLook(EnJso2* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    s16 yawToPlayer = Math_Vec3f_Yaw(&this->actor.world.pos, &player->actor.world.pos);
    this->actor.world.rot.y = yawToPlayer;
    this->actor.shape.rot.y = yawToPlayer;

}

RECOMP_HOOK("EnJso2_Slash") void TeleportFromSlash(EnJso2* this, PlayState* play) {
    f32 curFrame = this->skelAnime.curFrame;

    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        if (curFrame >= this->animEndFrame) {
            this->actor.speed = 0.0f;
            this->disableBlure = true;
            if (Rand_ZeroOne() < 0.05 && TimesTPd < 4) {
                EnJso2_SetupTeleport(this);
                TimesTPd++;
            }
            else EnJso2_SetupWaitAfterSlash(this);
        }
        break;

    case 1:
        if (curFrame >= this->animEndFrame) {
            this->actor.speed = 0.0f;
            this->disableBlure = true;
            if (Rand_ZeroOne() < 0.2 && TimesTPd < 4) {
                EnJso2_SetupTeleport(this);
                TimesTPd++;
            }
            else EnJso2_SetupWaitAfterSlash(this);
        }
        break;

    default:
        break;
    }
}

RECOMP_PATCH void EnJso2_DashAttack(EnJso2* this, PlayState* play) {
    f32 curFrame = this->skelAnime.curFrame;
    s16 yawDiff;
    s16 absYawDiff;
    Vec3f knockbackVelocity;
    Player* player = GET_PLAYER(play);

    int Difficulty = (int)recomp_get_config_double("diff_option");

    if ((this->rightSwordCollider.base.atFlags & AT_BOUNCED) || (this->leftSwordCollider.base.atFlags & AT_BOUNCED)) {
        this->rightSwordCollider.base.atFlags &= ~(AT_HIT | AT_BOUNCED);
        this->leftSwordCollider.base.atFlags &= ~(AT_HIT | AT_BOUNCED);
        Matrix_RotateYS(this->actor.yawTowardsPlayer, MTXMODE_NEW);
        Matrix_MultVecZ(-10.0f, &knockbackVelocity);
        Math_Vec3f_Copy(&this->knockbackVelocity, &knockbackVelocity);
        this->disableBlure = true;
        this->attackMovementTimer = 0;
        AudioSfx_SetChannelIO(&this->actor.projectedPos, NA_SE_EN_ANSATSUSYA_DASH_2, 0);
        EnJso2_SetupJumpBack(this);
        return;
    }

    if ((this->actor.velocity.y < 0.0f) && (this->actor.bgCheckFlags & BGCHECKFLAG_GROUND)) {
        if (Rand_ZeroOne() < ((BREG(22) * 0.1f) + 0.7f)) {
            this->actor.velocity.y = 13.0f;
        }
        else {
            AudioSfx_SetChannelIO(&this->actor.projectedPos, NA_SE_EN_ANSATSUSYA_DASH_2, 0);
            this->disableBlure = true;
            this->isTeleporting = true;
            this->actor.speed = 0.0f;
            EnJso2_SetupTeleport(this);
            return;
        }
    }

    if (Difficulty == 1) {
        f32 dist = this->actor.xzDistToPlayer;

        if (dist > 250.0f) {
            f32 framesToIntercept = dist / 15.0f;

            framesToIntercept = CLAMP_MAX(framesToIntercept, 15.0f);

            Vec3f predictedPos;
            predictedPos.x = player->actor.world.pos.x + (player->actor.velocity.x * framesToIntercept);
            predictedPos.y = player->actor.world.pos.y;
            predictedPos.z = player->actor.world.pos.z + (player->actor.velocity.z * framesToIntercept);

            s16 predictedYaw = Math_Vec3f_Yaw(&this->actor.world.pos, &predictedPos);

            Math_SmoothStepToS(&this->actor.world.rot.y, predictedYaw, 1, 0x1000, 0x100);
            this->actor.shape.rot.y = this->actor.world.rot.y;
        }
    }

    if (curFrame < this->animEndFrame) {
        return;
    }

    yawDiff = this->actor.yawTowardsPlayer - this->actor.shape.rot.y;
    absYawDiff = ABS_ALT(yawDiff);

    if ((this->attackMovementTimer == 0) || (this->actor.xzDistToPlayer < 120.0f) || (absYawDiff > 0x4300)) {
        AudioSfx_SetChannelIO(&this->actor.projectedPos, NA_SE_EN_ANSATSUSYA_DASH_2, 0);
        Math_ApproachZeroF(&this->actor.speed, 0.3f, 3.0f);
        EnJso2_SetupSlash(this, play);
    }
}

RECOMP_HOOK_RETURN("EnJso2_SetupWaitAfterSlash") void LessCooldownGM(EnJso2* this) {

    int Difficulty = (int)recomp_get_config_double("diff_option");
    int IdleTimer;
    int IdleTimerHit;

    switch (Difficulty) {
    case 0:
        IdleTimer = 20;
        IdleTimerHit = 10;
        break;

    case 1:
        IdleTimer = 5;
        IdleTimerHit = 1;
        break;

    default:
        break;
    }

    if (this->slashHitSomething) {
        EnJso2_ChangeAnim(this, EN_JSO2_ANIM_SLASH_LOOP);
        this->timer = IdleTimerHit;
    }
    else {
        EnJso2_ChangeAnim(this, EN_JSO2_ANIM_LOOK_AROUND);
        this->timer = IdleTimer;
    }
    TimesTPd = 0;
}

RECOMP_HOOK("EnJso2_Update") void GaroMUpdate(Actor* thisx, PlayState* play) {

	EnJso2* this = (EnJso2*)thisx;

    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        this->actor.colChkInfo.damage = (this->actor.colChkInfo.damage) / 2;
        this->attackTimer--;
        break;

    case 1:
        this->actor.colChkInfo.damage = (this->actor.colChkInfo.damage + 2) / 3;
        this->attackTimer = this->attackTimer - 2;
        break;

    default:
        break;
    }
    if (this->attackTimer < 0) this->attackTimer = 0;
}