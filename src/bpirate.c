#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_en_kaizoku.h"
#include "attributes.h"

// this is awful please help

void func_80B85F48(EnKaizoku* this);
void func_80B85FA8(EnKaizoku* this, PlayState* play);
void func_80B868B8(EnKaizoku* this, PlayState* play);
void func_80B86B58(EnKaizoku* this);
void func_80B86B74(EnKaizoku* this, PlayState* play);
void func_80B872A4(EnKaizoku* this);
void func_80B872F4(EnKaizoku* this, PlayState* play);
void func_80B874D8(EnKaizoku* this, PlayState* play);
void func_80B8760C(EnKaizoku* this, PlayState* play);
void func_80B87900(EnKaizoku* this);
void func_80B8798C(EnKaizoku* this, PlayState* play);
void func_80B87C7C(EnKaizoku* this);
void func_80B87CF8(PlayState* play, Vec3f* pos);
void func_80B87D3C(EnKaizoku* this, PlayState* play);
void func_80B87E28(EnKaizoku* this);
void func_80B87E9C(EnKaizoku* this, PlayState* play);
void func_80B87F70(EnKaizoku* this);
void func_80B87FDC(EnKaizoku* this, PlayState* play2);
void func_80B88214(EnKaizoku* this);
void func_80B88278(EnKaizoku* this, PlayState* play);
void func_80B8833C(EnKaizoku* this);
void func_80B88378(EnKaizoku* this, PlayState* play);
void func_80B88770(EnKaizoku* this);
void func_80B887AC(EnKaizoku* this, PlayState* play);
void func_80B88910(EnKaizoku* this);
void func_80B88964(EnKaizoku* this, PlayState* play);
void func_80B88CD8(EnKaizoku* this);
void func_80B88D6C(EnKaizoku* this, PlayState* play);
void func_80B89280(EnKaizoku* this, PlayState* play);
void func_80B894C0(EnKaizoku* this, PlayState* play);
void func_80B8971C(EnKaizoku* this, PlayState* play);

#define EnKaizoku_SetupWaitForApproach   func_80B85F48
#define EnKaizoku_WaitForApproach        func_80B85FA8
#define EnKaizoku_PlayerLoss             func_80B868B8
#define EnKaizoku_SetupPlayerWinCutscene func_80B86B58
#define EnKaizoku_PlayerWinCutscene      func_80B86B74
#define EnKaizoku_SetupReady             func_80B872A4
#define EnKaizoku_Ready                  func_80B872F4
#define EnKaizoku_SetupSpinDodge         func_80B874D8
#define EnKaizoku_SpinDodge              func_80B8760C
#define EnKaizoku_SetupBlock             func_80B87900
#define EnKaizoku_Block                  func_80B8798C
#define EnKaizoku_SetupJump              func_80B87C7C
#define EnKaizoku_SpawnVerticalFootDust  func_80B87CF8
#define EnKaizoku_Jump                   func_80B87D3C
#define EnKaizoku_SetupRollBack          func_80B87E28
#define EnKaizoku_RollBack               func_80B87E9C
#define EnKaizoku_SetupSlash             func_80B87F70
#define EnKaizoku_Slash                  func_80B87FDC
#define EnKaizoku_SetupRollForward       func_80B88214
#define EnKaizoku_RollForward            func_80B88278
#define EnKaizoku_SetupAdvance           func_80B8833C
#define EnKaizoku_Advance                func_80B88378
#define EnKaizoku_SetupPivot             func_80B88770
#define EnKaizoku_Pivot                  func_80B887AC
#define EnKaizoku_SetupSpinAttack        func_80B88910
#define EnKaizoku_SpinAttack             func_80B88964
#define EnKaizoku_SetupCircle            func_80B88CD8
#define EnKaizoku_Circle                 func_80B88D6C
#define EnKaizoku_Stunned                func_80B89280
#define EnKaizoku_Damaged                func_80B894C0
#define EnKaizoku_DefeatKnockdown        func_80B8971C

typedef enum KaizokuAction {
    /*  0 */ KAIZOKU_ACTION_HIDDEN,
    /*  1 */ KAIZOKU_ACTION_READY,
    /*  2 */ KAIZOKU_ACTION_SPIN_DODGE,
    /*  3 */ KAIZOKU_ACTION_CIRCLE,
    /*  4 */ KAIZOKU_ACTION_ADVANCE,
    /*  5 */ KAIZOKU_ACTION_ROLL_FORWARD,
    /*  6 */ KAIZOKU_ACTION_JUMP,
    /*  7 */ KAIZOKU_ACTION_ROLL_BACK,
    /*  8 */ KAIZOKU_ACTION_UNUSED_8,
    /*  9 */ KAIZOKU_ACTION_SLASH,
    /* 10 */ KAIZOKU_ACTION_PIVOT,
    /* 11 */ KAIZOKU_ACTION_SPIN_ATTACK,
    /* 12 */ KAIZOKU_ACTION_BLOCK,
    /* 13 */ KAIZOKU_ACTION_STUNNED,
    /* 14 */ KAIZOKU_ACTION_DAMAGED,
    /* 15 */ KAIZOKU_ACTION_KNOCK_DOWN,
    /* 16 */ KAIZOKU_ACTION_SCENE_FADE
} KaizokuAction;

s32 EnKaizoku_ReactToPlayer(EnKaizoku* this, PlayState* play, s16 arg2) {
    Player* player = GET_PLAYER(play);
    s16 angleToWall = ABS_ALT(this->picto.actor.wallYaw - this->picto.actor.shape.rot.y);
    s16 angleToPlayer = ABS_ALT(this->picto.actor.yawTowardsPlayer - this->picto.actor.shape.rot.y);
    Actor* explosiveActor;

    if (func_800BE184(play, &this->picto.actor, 100.0f, 0x2710, 0x4000, this->picto.actor.shape.rot.y)) {
        if (player->meleeWeaponAnimation == PLAYER_MWA_JUMPSLASH_START) {
            if (this->action != KAIZOKU_ACTION_SPIN_DODGE) {
                EnKaizoku_SetupSpinDodge(this, play);
            }
            return true;
        }
        else {
            EnKaizoku_SetupBlock(this);
            return true;
        }
    }
    else if (func_800BE184(play, &this->picto.actor, 100.0f, 0x5DC0, 0x2AA8, this->picto.actor.shape.rot.y)) {
        this->picto.actor.shape.rot.y = this->picto.actor.world.rot.y = this->picto.actor.yawTowardsPlayer;
        if ((this->picto.actor.bgCheckFlags & BGCHECKFLAG_WALL) && (ABS_ALT(angleToWall) < 0x2EE0) &&
            (this->picto.actor.xzDistToPlayer < 90.0f)) {
            if (this->action != KAIZOKU_ACTION_JUMP) {
                EnKaizoku_SetupJump(this);
            }
            return true;
        }
        else if (player->meleeWeaponAnimation == PLAYER_MWA_JUMPSLASH_START) {
            if (this->action != KAIZOKU_ACTION_SPIN_DODGE) {
                EnKaizoku_SetupSpinDodge(this, play);
            }
        }
        else if (this->picto.actor.xzDistToPlayer < BREG(11) + 180.0f) {
            EnKaizoku_SetupBlock(this);
        }
        else if (this->action != KAIZOKU_ACTION_ROLL_BACK) {
            EnKaizoku_SetupRollBack(this);
        }
        return true;
    }

    explosiveActor = Actor_FindNearby(play, &this->picto.actor, -1, ACTORCAT_EXPLOSIVES, 80.0f);
    if (explosiveActor != NULL) {
        this->picto.actor.shape.rot.y = this->picto.actor.world.rot.y = this->picto.actor.yawTowardsPlayer;

        if (((this->picto.actor.bgCheckFlags & BGCHECKFLAG_WALL) && (angleToWall < 0x2EE0)) ||
            (explosiveActor->id == ACTOR_EN_BOM_CHU)) {
            if ((explosiveActor->id == ACTOR_EN_BOM_CHU) &&
                (Actor_WorldDistXYZToActor(&this->picto.actor, explosiveActor) < 80.0f) &&
                (BINANG_ADD(this->picto.actor.shape.rot.y - explosiveActor->world.rot.y, 0x8000) < 0x4000)) {
                if (this->action != KAIZOKU_ACTION_JUMP) {
                    EnKaizoku_SetupJump(this);
                }
            }
            else {
                EnKaizoku_SetupBlock(this);
            }

            return true;
        }

        if (this->action != KAIZOKU_ACTION_ROLL_BACK) {
            EnKaizoku_SetupRollBack(this);
        }
        return true;
    }

    if (arg2) {
        s16 yawDiff;

        if (angleToPlayer >= 0x2710) {
            // in OOT this was sidestep instead of block
            EnKaizoku_SetupBlock(this);
        }
        else {
            yawDiff = player->actor.shape.rot.y - this->picto.actor.shape.rot.y;
            if ((this->picto.actor.xzDistToPlayer <= 65.0f) && !Actor_OtherIsTargeted(play, &this->picto.actor) &&
                (ABS_ALT(yawDiff) < 0x5000)) {
                if (this->action != KAIZOKU_ACTION_SLASH) {
                    EnKaizoku_SetupSlash(this);
                    return true;
                }
            }
            else if (this->action != KAIZOKU_ACTION_CIRCLE) {
                EnKaizoku_SetupCircle(this);
            }
        }
        return true;
    }

    return false;
}


RECOMP_HOOK("func_80B89280") void NoStun(EnKaizoku* this, PlayState* play) {

    int Difficulty = (int)recomp_get_config_double("diff_option");

    if (Difficulty == 1) {

        this->unk_2D8 = false;
        EnKaizoku_ReactToPlayer(this, play, true);
    }

}

RECOMP_HOOK("EnKaizoku_Update") void PirateBossUpdate(Actor* thisx, PlayState* play2) {

    EnKaizoku* this = (EnKaizoku*)thisx;

    int Difficulty = (int)recomp_get_config_double("diff_option");
    float speedMultiplier = 1.0f;

    switch (Difficulty) {
    case 0:
        speedMultiplier = 1.1f;
        this->picto.actor.colChkInfo.damage = (this->picto.actor.colChkInfo.damage) / 2;
        break;

    case 1:
        speedMultiplier = 1.5f;
        this->picto.actor.colChkInfo.damage = (this->picto.actor.colChkInfo.damage + 2) / 3;
        break;

    default:
        break;
    }

    if (this->picto.actor.colChkInfo.health != 0 && this->actionFunc != EnKaizoku_Slash && this->actionFunc != (EnKaizokuActionFunc)EnKaizoku_SetupRollBack) {
        this->skelAnime.playSpeed = speedMultiplier;
    }

    if (this->action == KAIZOKU_ACTION_SCENE_FADE) gSaveContext.save.saveInfo.playerData.health = 0;
}