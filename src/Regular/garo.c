#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_en_jso.h"
#include "overlays/actors/ovl_En_Clear_Tag/z_en_clear_tag.h"
#include "overlays/actors/ovl_En_Encount3/z_en_encount3.h"
#include "overlays/actors/ovl_En_Part/z_en_part.h"
#include "eztr_api.h"

#include "z_en_encount3.h"
#include "assets/objects/object_big_fwall/object_big_fwall.h"

extern s32 D_809AD810;

void func_809AD194(EnEncount3* this, PlayState* play);

void EnJso_SetupCower(EnJso* this);
void EnJso_CirclePlayer(EnJso* this, PlayState* play);
void EnJso_Stunned(EnJso* this, PlayState* play);

bool PlayerUsingGaroMask = false;
int TimesEncountered = 0;

typedef enum EnJsoAnimation {
    /*  0 */ EN_JSO_ANIM_APPEAR,
    /*  1 */ EN_JSO_ANIM_IDLE,
    /*  2 */ EN_JSO_ANIM_BOUNCE,
    /*  3 */ EN_JSO_ANIM_GUARD,
    /*  4 */ EN_JSO_ANIM_DASH_ATTACK,
    /*  5 */ EN_JSO_ANIM_SLASH_START,
    /*  6 */ EN_JSO_ANIM_SLASH_LOOP,
    /*  7 */ EN_JSO_ANIM_KNOCKED_BACK,
    /*  8 */ EN_JSO_ANIM_COWER,
    /*  9 */ EN_JSO_ANIM_DAMAGED,
    /* 10 */ EN_JSO_ANIM_JUMP_BACK,
    /* 11 */ EN_JSO_ANIM_FALL_DOWN,
    /* 12 */ EN_JSO_ANIM_MAX
} EnJsoAnimation;

static AnimationHeader* sAnimations[EN_JSO_ANIM_MAX] = {
    &gGaroAppearAnim,      // EN_JSO_ANIM_APPEAR
    &gGaroIdleAnim,        // EN_JSO_ANIM_IDLE
    &gGaroBounceAnim,      // EN_JSO_ANIM_BOUNCE
    &gGaroGuardAnim,       // EN_JSO_ANIM_GUARD
    &gGaroDashAttackAnim,  // EN_JSO_ANIM_DASH_ATTACK
    &gGaroSlashStartAnim,  // EN_JSO_ANIM_SLASH_START
    &gGaroSlashLoopAnim,   // EN_JSO_ANIM_SLASH_LOOP
    &gGaroKnockedBackAnim, // EN_JSO_ANIM_KNOCKED_BACK
    &gGaroCowerAnim,       // EN_JSO_ANIM_COWER
    &gGaroDamagedAnim,     // EN_JSO_ANIM_DAMAGED
    &gGaroJumpBackAnim,    // EN_JSO_ANIM_JUMP_BACK
    &gGaroFallDownAnim,    // EN_JSO_ANIM_FALL_DOWN
};

static u8 sAnimationModes[EN_JSO_ANIM_MAX] = {
    ANIMMODE_ONCE, // EN_JSO_ANIM_APPEAR
    ANIMMODE_LOOP, // EN_JSO_ANIM_IDLE
    ANIMMODE_LOOP, // EN_JSO_ANIM_BOUNCE
    ANIMMODE_ONCE, // EN_JSO_ANIM_GUARD
    ANIMMODE_ONCE, // EN_JSO_ANIM_DASH_ATTACK
    ANIMMODE_ONCE, // EN_JSO_ANIM_SLASH_START
    ANIMMODE_LOOP, // EN_JSO_ANIM_SLASH_LOOP
    ANIMMODE_ONCE, // EN_JSO_ANIM_KNOCKED_BACK
    ANIMMODE_LOOP, // EN_JSO_ANIM_COWER
    ANIMMODE_LOOP, // EN_JSO_ANIM_DAMAGED
    ANIMMODE_ONCE, // EN_JSO_ANIM_JUMP_BACK
    ANIMMODE_ONCE, // EN_JSO_ANIM_FALL_DOWN
};

typedef enum EnJsoAction {
    /*  0 */ EN_JSO_ACTION_INTRO_CUTSCENE,
    /*  1 */ EN_JSO_ACTION_REAPPEAR,
    /*  2 */ EN_JSO_ACTION_CIRCLE_PLAYER,
    /*  3 */ EN_JSO_ACTION_GUARD,
    /*  4 */ EN_JSO_ACTION_SPIN_BEFORE_ATTACK,
    /*  5 */ EN_JSO_ACTION_DASH_ATTACK,
    /*  6 */ EN_JSO_ACTION_SLASH,
    /*  7 */ EN_JSO_ACTION_WAIT_AFTER_SLASH,
    /*  8 */ EN_JSO_ACTION_STUNNED,
    /*  9 */ EN_JSO_ACTION_KNOCKED_BACK,
    /* 10 */ EN_JSO_ACTION_COWER,
    /* 11 */ EN_JSO_ACTION_DAMAGED,
    /* 12 */ EN_JSO_ACTION_JUMP_BACK,
    /* 13 */ EN_JSO_ACTION_DEAD,
    /* 14 */ EN_JSO_ACTION_FALL_DOWN_AND_TALK,
    /* 15 */ EN_JSO_ACTION_UNK_15 // Checked in EnJso_Update, but never actually used
} EnJsoAction;

extern void EnJso_ChangeAnim(EnJso*, s32);

EZTR_MSG_CALLBACK(garo_text_callback_1) {

    int Difficulty = (int)recomp_get_config_double("diff_option");

    if (Difficulty == 1) {
        EZTR_MsgSContent_Sprintf(buf->data.content, "Invader located!|11Commencing execution.|19|BF");
    }
    else {
        EZTR_MsgSContent_Sprintf(buf->data.content, "Master! You called!|19|BF");
    }
}

EZTR_MSG_CALLBACK(garo_text_callback_2) {

    int Difficulty = (int)recomp_get_config_double("diff_option");

    if (Difficulty == 1) {
        EZTR_MsgSContent_Sprintf(buf->data.content, "Surrender...|11|13|13|12|17Shall never be accepted.|18|E0|BF");
    }
    else {
        EZTR_MsgSContent_Sprintf(buf->data.content, "...!!!|11|13|13|12|17What are you???|18|E0|BF");
    }
}

EZTR_ON_INIT void init_text_garo() {
    EZTR_Basic_ReplaceText(
        0x1396,
        EZTR_STANDARD_TEXT_BOX_I,
        0,
        EZTR_ICON_NO_ICON,
        EZTR_NO_VALUE,
        EZTR_NO_VALUE,
        EZTR_NO_VALUE,
        true,
        "",
        garo_text_callback_1
    );

    EZTR_Basic_ReplaceText(
        0x1397,
        EZTR_STANDARD_TEXT_BOX_I,
        0,
        EZTR_ICON_NO_ICON,
        EZTR_NO_VALUE,
        EZTR_NO_VALUE,
        EZTR_NO_VALUE,
        true,
        "...!!!|11|13|13|12|17What are you???|18|E0|BF",
        garo_text_callback_2
    );
}

RECOMP_HOOK_RETURN("EnJso_SetupDashAttack") void IncreaseSpeedForAttack(EnJso* this) {

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

RECOMP_HOOK_RETURN("EnJso_SetupCower") void WhatABaby(EnJso* this) {

    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        this->timer = 15;
        break;

    case 1:
        this->timer = 5;
        break;

    default:
        break;
    }
}

RECOMP_HOOK_RETURN("EnJso_SetupWaitAfterSlash") void LessCooldown(EnJso* this) {

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
        EnJso_ChangeAnim(this, EN_JSO_ANIM_SLASH_LOOP);
        this->timer = IdleTimerHit;
    }
    else {
        EnJso_ChangeAnim(this, EN_JSO_ANIM_SLASH_LOOP);
        this->timer = IdleTimer;
    }
}

RECOMP_PATCH void EnJso_SetupStunned(EnJso* this) {
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
    EnJso_ChangeAnim(this, EN_JSO_ANIM_DAMAGED);

    if (((this->drawDmgEffType == ACTOR_DRAW_DMGEFF_FROZEN_SFX) ||
        (this->drawDmgEffType == ACTOR_DRAW_DMGEFF_FROZEN_NO_SFX)) &&
        (this->drawDmgEffAlpha == 0)) {
        this->drawDmgEffAlpha = 0;
        this->drawDmgEffType = ACTOR_DRAW_DMGEFF_FIRE;
    }

    this->timer = StunTimer;
    this->actor.colorFilterTimer = StunTimer;

    Matrix_RotateYS(this->actor.yawTowardsPlayer, MTXMODE_NEW);
    Matrix_MultVecZ(-10.0f, &knockbackVelocity);
    Math_Vec3f_Copy(&this->knockbackVelocity, &knockbackVelocity);
    this->action = EN_JSO_ACTION_STUNNED;
    this->bodyCollider.base.acFlags &= ~AC_HARD;
    this->actionFunc = EnJso_Stunned;
}

RECOMP_HOOK("EnJso_Update") void GaroUpdate(Actor* thisx, PlayState* play) {

	EnJso* this = (EnJso*)thisx;

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

    if (Player_GetMask(play) == PLAYER_MASK_GARO) PlayerUsingGaroMask = true;
    else PlayerUsingGaroMask = false;
}

RECOMP_HOOK("EnEncount3_Init") void CanAttackNoMask(Actor* thisx, PlayState* play) {

    EnEncount3* this = (EnEncount3*)thisx;
    #define CAN_ATTACK 0x8000

    if (Rand_ZeroOne() < 0.2) {
        this->actor.params |= CAN_ATTACK;
    }
}

RECOMP_HOOK_RETURN("func_809AD058") void Attacking(EnEncount3* this) {
    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        this->timer = 30;
        break;

    case 1:
        if (this->actor.params & CAN_ATTACK) this->timer = 30 + (TimesEncountered * 30);
        else this->timer = 20;
        break;

    default:
        break;
    }
}

RECOMP_PATCH void func_809AD084(EnEncount3* this, PlayState* play) {
    int Difficulty = (int)recomp_get_config_double("diff_option");

    if ((this->switchFlag > SWITCH_FLAG_NONE) && Flags_GetSwitch(play, this->switchFlag)) {
        Actor_Kill(&this->actor);
        return;
    }

    if (!(this->unk16C < this->actor.xzDistToPlayer) &&
        (Player_GetMask(play) == PLAYER_MASK_GARO || (Difficulty == 1 && (this->actor.params & CAN_ATTACK))) &&
        !D_809AD810) {

        if (this->timer > 0) {
            if (Difficulty == 1) this->timer = this->timer - 2;
            else this->timer--;
            if (this->timer < 0) this->timer = 0;
        }
        else {
            this->child =
                Actor_SpawnAsChild(&play->actorCtx, &this->actor, play, this->childActorId, this->actor.world.pos.x,
                    this->actor.world.pos.y, this->actor.world.pos.z, 0, 0, 0, this->childParams);
            if (this->child != NULL) {
                this->unk14E++;
                D_809AD810 = true;
                this->actionFunc = func_809AD194;
                TimesEncountered++;

                switch (Difficulty) {
                case 0:
                    this->timer = 30;
                    break;

                case 1:
                    if (this->actor.params & CAN_ATTACK) this->timer = 30 + (TimesEncountered * 10);
                    else this->timer = 20;
                    break;
                default:
                    break;
                }
            }
        }
    }
}