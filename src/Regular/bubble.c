#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_en_bb.h"
#include "overlays/actors/ovl_En_Clear_Tag/z_en_clear_tag.h"
#include "assets/objects/gameplay_keep/gameplay_keep.h"
#include "eztr_api.h"

// doing funnies with home rotation so I can skip doing global variables in case you were curious

void EnBb_SetupFlyIdle(EnBb* this);
void EnBb_Attack(EnBb* this, PlayState* play);
void EnBb_Down(EnBb* this, PlayState* play);

extern s32 EnBb_OverrideLimbDraw(PlayState*, s32, Gfx**, Vec3f*, Vec3s*, Actor*);
extern void EnBb_PostLimbDraw(PlayState*, s32, Gfx**, Vec3s*, Actor*);
extern void EnBb_SetupDown(EnBb*);
extern void EnBb_SetupDamage(EnBb*);
extern void EnBb_SetupDead(EnBb*, PlayState*);
extern void EnBb_SetupFrozen(EnBb*);
extern void EnBb_Freeze(EnBb*);
extern void EnBb_Thaw(EnBb*, PlayState*);

int CurrentVarient = 0;

typedef enum {
    /* 0x0 */ EN_BB_DMGEFF_NONE,
    /* 0x1 */ EN_BB_DMGEFF_STUN,
    /* 0x3 */ EN_BB_DMGEFF_ICE_ARROW = 0x3,
    /* 0x4 */ EN_BB_DMGEFF_LIGHT_ARROW,
    /* 0x5 */ EN_BB_DMGEFF_ZORA_MAGIC,
    /* 0xE */ EN_BB_DMGEFF_HOOKSHOT = 0xE
} EnBbDamageEffect;

static DamageTable sDamageTable = {
    /* Deku Nut       */ DMG_ENTRY(0, EN_BB_DMGEFF_STUN),
    /* Deku Stick     */ DMG_ENTRY(1, EN_BB_DMGEFF_NONE),
    /* Horse trample  */ DMG_ENTRY(1, EN_BB_DMGEFF_NONE),
    /* Explosives     */ DMG_ENTRY(1, EN_BB_DMGEFF_NONE),
    /* Zora boomerang */ DMG_ENTRY(1, EN_BB_DMGEFF_NONE),
    /* Normal arrow   */ DMG_ENTRY(1, EN_BB_DMGEFF_NONE),
    /* UNK_DMG_0x06   */ DMG_ENTRY(0, EN_BB_DMGEFF_NONE),
    /* Hookshot       */ DMG_ENTRY(0, EN_BB_DMGEFF_HOOKSHOT),
    /* Goron punch    */ DMG_ENTRY(1, EN_BB_DMGEFF_NONE),
    /* Sword          */ DMG_ENTRY(1, EN_BB_DMGEFF_NONE),
    /* Goron pound    */ DMG_ENTRY(1, EN_BB_DMGEFF_NONE),
    /* Fire arrow     */ DMG_ENTRY(1, EN_BB_DMGEFF_NONE),
    /* Ice arrow      */ DMG_ENTRY(2, EN_BB_DMGEFF_ICE_ARROW),
    /* Light arrow    */ DMG_ENTRY(2, EN_BB_DMGEFF_LIGHT_ARROW),
    /* Goron spikes   */ DMG_ENTRY(1, EN_BB_DMGEFF_NONE),
    /* Deku spin      */ DMG_ENTRY(1, EN_BB_DMGEFF_NONE),
    /* Deku bubble    */ DMG_ENTRY(1, EN_BB_DMGEFF_NONE),
    /* Deku launch    */ DMG_ENTRY(2, EN_BB_DMGEFF_NONE),
    /* UNK_DMG_0x12   */ DMG_ENTRY(0, EN_BB_DMGEFF_STUN),
    /* Zora barrier   */ DMG_ENTRY(0, EN_BB_DMGEFF_ZORA_MAGIC),
    /* Normal shield  */ DMG_ENTRY(0, EN_BB_DMGEFF_NONE),
    /* Light ray      */ DMG_ENTRY(0, EN_BB_DMGEFF_NONE),
    /* Thrown object  */ DMG_ENTRY(1, EN_BB_DMGEFF_NONE),
    /* Zora punch     */ DMG_ENTRY(1, EN_BB_DMGEFF_NONE),
    /* Spin attack    */ DMG_ENTRY(1, EN_BB_DMGEFF_NONE),
    /* Sword beam     */ DMG_ENTRY(0, EN_BB_DMGEFF_NONE),
    /* Normal Roll    */ DMG_ENTRY(0, EN_BB_DMGEFF_NONE),
    /* UNK_DMG_0x1B   */ DMG_ENTRY(0, EN_BB_DMGEFF_NONE),
    /* UNK_DMG_0x1C   */ DMG_ENTRY(0, EN_BB_DMGEFF_NONE),
    /* Unblockable    */ DMG_ENTRY(0, EN_BB_DMGEFF_NONE),
    /* UNK_DMG_0x1E   */ DMG_ENTRY(0, EN_BB_DMGEFF_NONE),
    /* Powder Keg     */ DMG_ENTRY(1, EN_BB_DMGEFF_NONE),
};

void func_80833B18(PlayState* play, Player* this, s32 arg2, f32 speed, f32 velocityY, s16 arg5, s32 invincibilityTimer);

void EnBb_CheckForWall(EnBb* this) {
    s16 yawDiff;

    if (this->actor.bgCheckFlags & BGCHECKFLAG_WALL) {
        yawDiff = this->actor.shape.rot.y - this->actor.wallYaw;
        if (ABS_ALT(yawDiff) > 0x4000) {
            this->actor.shape.rot.y = ((this->actor.wallYaw * 2) - this->actor.shape.rot.y) - 0x8000;
        }

        this->targetYRotation = this->actor.shape.rot.y;
        this->actor.bgCheckFlags &= ~BGCHECKFLAG_WALL;
    }
}

void EnBb_UpdateStateForFlying(EnBb* this) {
    SkelAnime_Update(&this->skelAnime);
    if (this->actor.floorHeight > BGCHECK_Y_MIN) {
        Math_StepToF(&this->actor.world.pos.y, this->actor.floorHeight + this->flyHeightMod, 0.5f);
    }

    this->actor.world.pos.y += Math_CosS(this->bobPhase);
    this->bobPhase += 0x826;
    Math_StepToF(&this->flameScaleY, 0.8f, 0.1f);
    Math_StepToF(&this->flameScaleX, 1.0f, 0.1f);
    EnBb_CheckForWall(this);
    Math_StepToF(&this->actor.speed, this->maxSpeed, 0.5f);
    Math_ApproachS(&this->actor.shape.rot.y, this->targetYRotation, 5, 0x3E8);
    this->actor.world.rot.y = this->actor.shape.rot.y;
}

RECOMP_HOOK_RETURN("EnBb_Init") void BBBuff(Actor* thisx, PlayState* play) {
    EnBb* this = (EnBb*)thisx;

    int Difficulty = (int)recomp_get_config_double("diff_option");
    s16 baseHealth = this->actor.colChkInfo.health;

    switch (Difficulty) {
    case 0:
        this->attackRange = 400.0f;
        break;

    case 1:
        this->attackRange = 800.0f;
        break;

    default:
        break;
    }
}

RECOMP_HOOK("EnBb_Init") void SetType(Actor* thisx, PlayState* play) {

    EnBb* this = (EnBb*)thisx;
    int Difficulty = (int)recomp_get_config_double("diff_option");

    if (Difficulty == 1) {

        if (this->actor.home.rot.x == 0) {
            if (Difficulty == 1) {
                this->actor.home.rot.z = (Rand_ZeroOne() * 3.0f);
            }
            else {
                this->actor.home.rot.z = 0;
            }

            this->variant = this->actor.home.rot.z;

            this->actor.home.rot.x = 1;
        }
    }
}

RECOMP_PATCH void EnBb_UpdateDamage(EnBb* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    if (this->collider.base.acFlags & AC_HIT) {
        this->collider.base.acFlags &= ~AC_HIT;
        this->collider.base.atFlags &= ~(AT_HIT | AT_BOUNCED);
        this->collider.base.atFlags &= ~AT_ON;
        if ((this->drawDmgEffType != ACTOR_DRAW_DMGEFF_FROZEN_NO_SFX) ||
            !(this->collider.elem.acHitElem->atDmgInfo.dmgFlags & 0xDB0B3)) {
            Actor_SetDropFlag(&this->actor, &this->collider.elem);
            this->flameScaleY = 0.0f;
            this->flameScaleX = 0.0f;
            EnBb_Thaw(this, play);

            if (Actor_ApplyDamage(&this->actor) == 0) {
                Enemy_StartFinishingBlow(play, &this->actor);
            }

            if (this->actor.colChkInfo.damageEffect == EN_BB_DMGEFF_ICE_ARROW) {
                EnBb_Freeze(this);
                if (this->actor.colChkInfo.health == 0) {
                    this->timer = 3;
                    this->collider.base.acFlags &= ~AC_ON;
                }

                EnBb_SetupFrozen(this);
            }
            else if (this->actor.colChkInfo.health == 0) {
                EnBb_SetupDead(this, play);
            }
            else {
                EnBb_SetupDamage(this);
            }

            if (this->actor.colChkInfo.damageEffect == EN_BB_DMGEFF_LIGHT_ARROW) {
                this->drawDmgEffAlpha = 4.0f;
                this->drawDmgEffScale = 0.4f;
                this->drawDmgEffType = ACTOR_DRAW_DMGEFF_LIGHT_ORBS;
                Actor_Spawn(&play->actorCtx, play, ACTOR_EN_CLEAR_TAG, this->collider.elem.acDmgInfo.hitPos.x,
                    this->collider.elem.acDmgInfo.hitPos.y, this->collider.elem.acDmgInfo.hitPos.z, 0, 0, 0,
                    CLEAR_TAG_PARAMS(CLEAR_TAG_SMALL_LIGHT_RAYS));
            }
        }
    }
    else if (this->collider.base.atFlags & AT_BOUNCED) {
        this->collider.base.atFlags &= ~(AT_HIT | AT_BOUNCED);
        if (this->actionFunc != EnBb_Down) {
            this->actor.world.rot.y = this->actor.yawTowardsPlayer + 0x8000;
            this->actor.shape.rot.y = this->actor.world.rot.y;
            EnBb_SetupDown(this);
        }
    }
    else if (this->collider.base.atFlags & AT_HIT) {
        this->collider.base.atFlags &= ~AT_HIT;
        this->actor.world.rot.y = this->actor.yawTowardsPlayer + 0x8000;
        this->actor.shape.rot.y = this->actor.world.rot.y;
        Actor_PlaySfx(&this->actor, NA_SE_EN_BUBLE_BITE);

        if (this->flameScaleX > 0.0f) {
            Player* player = GET_PLAYER(play);
            switch (this->variant) {
            case 1:
                if (player->stateFlags1 != PLAYER_STATE1_800000) func_80833B18(play, player, 3, 0.0f, 0.0f, 0, 20);
                break;
            case 2:
                if (player->stateFlags1 != PLAYER_STATE1_800000) func_80833B18(play, player, 4, 0.0f, 0.0f, 0, 20);
                break;
            case 0:
            default:
                gSaveContext.jinxTimer = 1200;
                break;
            }
        }

        if (this->actionFunc == EnBb_Attack) {
            EnBb_SetupFlyIdle(this);
        }
    }
}

RECOMP_HOOK("EnBb_Update") void Updating(Actor* thisx, PlayState* play) {
    EnBb* this = (EnBb*)thisx;
    int Difficulty = (int)recomp_get_config_double("diff_option");
    Player* player = GET_PLAYER(play);

    if (player->focusActor == &this->actor) {
        CurrentVarient = this->variant;
    }

    if (Difficulty == 1) {
        if (Rand_ZeroOne() < 0.05f) {
            this->actor.home.pos.x = this->actor.world.pos.x + Rand_CenteredFloat(300.0f);
            this->actor.home.pos.z = this->actor.world.pos.z + Rand_CenteredFloat(300.0f);
            this->actor.home.pos.y = this->actor.world.pos.y + Rand_CenteredFloat(50.0f);
        }
    }

    if (this->actor.home.rot.y & 1) {
        this->actor.home.rot.x++;

        if (this->actor.home.rot.x >= 5) {
            switch (Difficulty) {
            case 0:
                this->attackRange = 350.0f;
                this->actor.colChkInfo.health = this->actor.home.rot.z * 2;
                break;

            case 1:
                this->attackRange = 500.0f;
                this->actor.colChkInfo.health = this->actor.home.rot.z * 3;
                break;

            default:
                break;
            }

            this->actor.home.rot.x = 0;
            this->actor.home.rot.y &= ~1;
        }
    }
    switch (Difficulty) {
    case 0:
        this->actor.colChkInfo.damage = (this->actor.colChkInfo.damage) / 2;
        break;

    case 1:
        this->actor.colChkInfo.damage = (this->actor.colChkInfo.damage + 2) / 3;
        break;

    default:
        break;
    }
}

EZTR_MSG_CALLBACK(bb_text_callback) {
    int Difficulty = (int)recomp_get_config_double("diff_option");

    if (Difficulty == 0) {
        EZTR_MsgSContent_Sprintf(buf->data.content, "|05That's a |00Blue Bubble|05!|11Quick! Run! Don't let it curse|11you! If it comes after you, defend|11yourself to block it!|BF");
        return;
    }
    if (CurrentVarient == 0) {
        EZTR_MsgSContent_Sprintf(buf->data.content, "|05That's a |00Purple Bubble|05!|11Quick! Run! Don't let it curse|11you!|BF");
    }
    if (CurrentVarient == 1) {
        EZTR_MsgSContent_Sprintf(buf->data.content, "|05That's a |00Blue Bubble|05!|11Quick! Run! Don't let it freeze|11you!|BF");
    }
    if (CurrentVarient == 2) {
        EZTR_MsgSContent_Sprintf(buf->data.content, "|05That's a |00Yellow Bubble|05!|11Quick! Run! Don't let it shock|11you!|BF");
    }
}

EZTR_ON_INIT void init_text_bubble() {
    EZTR_Basic_ReplaceText(
        0x191C,
        EZTR_BLUE_TEXT_BOX,
        0,
        EZTR_ICON_NO_ICON,
        EZTR_NO_VALUE,
        EZTR_NO_VALUE,
        EZTR_NO_VALUE,
        true,
        "",
        bb_text_callback
    );
}

RECOMP_HOOK("EnBb_SetupWaitForRevive") void Vive(EnBb* this) {
    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        this->timer = 100;
        break;

    case 1:
        this->timer = 50;
        break;

    default:
        this->timer = 200;
        break;
    }
}

RECOMP_PATCH void EnBb_SetupAttack(EnBb* this) {
    int Difficulty = (int)recomp_get_config_double("diff_option");
    float diffbasedMaxSpeed = 1.0f;

    switch (Difficulty) {
    case 0:
        diffbasedMaxSpeed = 1.25f;
        break;

    case 1:
        diffbasedMaxSpeed = 1.5f;
        break;

    default:
        diffbasedMaxSpeed = 1.0f;
        break;
    }

    Animation_PlayLoop(&this->skelAnime, &gBubbleAttackAnim);
    this->timer = (s32)Rand_ZeroFloat(20.0f) + 60 * diffbasedMaxSpeed;
    this->flyHeightMod = (Math_CosS(this->bobPhase) * 10.0f) + 30.0f;
    this->targetYRotation = this->actor.yawTowardsPlayer;
    this->maxSpeed = Rand_ZeroFloat(1.5f) + 4.0f * diffbasedMaxSpeed;
    this->actionFunc = EnBb_Attack;
}

RECOMP_PATCH void EnBb_Attack(EnBb* this, PlayState* play) {
    int Difficulty = (int)recomp_get_config_double("diff_option");
    float sightDist = 120.f;
    float chaseDist = 400.0f;

    switch (Difficulty) {
    case 0:
        sightDist = 240.0f;
        chaseDist = 600.0f;
        break;

    case 1:
        sightDist = 360.0f;
        chaseDist = 800.0f;
        break;

    default:
        sightDist = 120.0f;
        chaseDist = 400.0f;
        break;
    }

    this->targetYRotation = this->actor.yawTowardsPlayer;
    EnBb_UpdateStateForFlying(this);

    if (Animation_OnFrame(&this->skelAnime, 0.0f) || Animation_OnFrame(&this->skelAnime, 5.0f)) {
        Actor_PlaySfx(&this->actor, NA_SE_EN_BUBLE_MOUTH);
    }
    else if (Animation_OnFrame(&this->skelAnime, 2.0f) || Animation_OnFrame(&this->skelAnime, 7.0f)) {
        Actor_PlaySfx(&this->actor, NA_SE_EN_BUBLE_WING);
    }
    else if (Animation_OnFrame(&this->skelAnime, 0.0f) && (Rand_ZeroOne() < 0.1f)) {
        Actor_PlaySfx(&this->actor, NA_SE_EN_BUBLE_LAUGH);
    }

    this->timer--;

    if (((this->attackRange + sightDist) < this->actor.xzDistToPlayer) || (this->timer == 0) ||
        (Player_GetMask(play) == PLAYER_MASK_STONE) ||
        (Actor_WorldDistXZToPoint(&this->actor, &this->actor.home.pos) > (chaseDist * 2))) {
        EnBb_SetupFlyIdle(this);
    }
}

RECOMP_PATCH void EnBb_FlyIdle(EnBb* this, PlayState* play) {
    int Difficulty = (int)recomp_get_config_double("diff_option");
    float sightDist = 120.f;

    switch (Difficulty) {
    case 0:
        sightDist = 240.0f;
        break;

    case 1:
        sightDist = 360.0f;
        break;

    default:
        sightDist = 120.0f;
        break;
    }

    EnBb_UpdateStateForFlying(this);

    if (Animation_OnFrame(&this->skelAnime, 5.0f)) {
        Actor_PlaySfx(&this->actor, NA_SE_EN_BUBLE_WING);
    }
    else if (Animation_OnFrame(&this->skelAnime, 0.0f) && (Rand_ZeroOne() < 0.1f)) {
        Actor_PlaySfx(&this->actor, NA_SE_EN_BUBLE_LAUGH);
    }

    DECR(this->attackWaitTimer);
    this->timer--;

    if ((this->attackWaitTimer == 0) && (this->actor.xzDistToPlayer < this->attackRange + sightDist) &&
        (Player_GetMask(play) != PLAYER_MASK_STONE)) {
        EnBb_SetupAttack(this);
    }
    else if (this->timer == 0) {
        if (Difficulty == 0) EnBb_SetupFlyIdle(this);
    }
}

extern Gfx gEffFire1DL[];

RECOMP_PATCH void EnBb_Draw(Actor* thisx, PlayState* play) {
    s32 pad;
    EnBb* this = (EnBb*)thisx;
    MtxF* currentMatrixState;
    Gfx* gfx;

    OPEN_DISPS(play->state.gfxCtx);

    int Difficulty = (int)recomp_get_config_double("diff_option");

    gfx = POLY_OPA_DISP;
    gSPDisplayList(&gfx[0], gSetupDLs[SETUPDL_25]);
    POLY_OPA_DISP = &gfx[1];
    SkelAnime_DrawOpa(play, this->skelAnime.skeleton, this->skelAnime.jointTable, EnBb_OverrideLimbDraw,
        EnBb_PostLimbDraw, &this->actor);

    if (this->flameScaleX > 0.0f) {
        currentMatrixState = Matrix_GetCurrent();
        Gfx_SetupDL25_Xlu(play->state.gfxCtx);
        Matrix_RotateYS(((Camera_GetCamDirYaw(GET_ACTIVE_CAM(play)) - this->actor.shape.rot.y) + 0x8000),
            MTXMODE_APPLY);
        Matrix_Scale(this->flameScaleX, this->flameScaleY, 1.0f, MTXMODE_APPLY);
        gDPSetPrimColor(POLY_XLU_DISP++, 0x80, 0x80, 255, 255, 255, 255);
        if (Difficulty == 1) {
            switch (this->variant) {
            case 1:
                gDPSetEnvColor(POLY_XLU_DISP++, 0, 100, 255, 255);
                break;
            case 2:
                gDPSetEnvColor(POLY_XLU_DISP++, 255, 255, 0, 255);
                break;
            case 0:
            default:
                gDPSetPrimColor(POLY_XLU_DISP++, 0x80, 0x80, 255, 0, 255, 255);
                gDPSetEnvColor(POLY_XLU_DISP++, 64, 0, 255, 255);
                break;
            }
        }
        else {
            gDPSetPrimColor(POLY_XLU_DISP++, 0x80, 0x80, 255, 255, 255, 255);
            gDPSetEnvColor(POLY_XLU_DISP++, 0, 0, 255, 0);
        }
        gSPSegment(
            POLY_XLU_DISP++, 0x08,
            Gfx_TwoTexScroll(play->state.gfxCtx, 0, 0, 0, 32, 64, 1, 0, (play->gameplayFrames * -20) & 0x1FF, 32, 128));
        currentMatrixState->mf[3][1] -= 47.0f * this->flameScaleY;
        MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx);
        gSPDisplayList(POLY_XLU_DISP++, gEffFire1DL);
    }

    Actor_DrawDamageEffects(play, &this->actor, this->bodyPartsPos, BUBBLE_BODYPART_MAX, this->drawDmgEffScale,
        this->drawDmgEffFrozenSteamScale, this->drawDmgEffAlpha, this->drawDmgEffType);

    CLOSE_DISPS(play->state.gfxCtx);
}