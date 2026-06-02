#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_en_knight.h"
#include "z64shrink_window.h"
#include "attributes.h"
#include "overlays/actors/ovl_Mir_Ray3/z_mir_ray3.h"
#include "overlays/effects/ovl_Effect_Ss_Hitmark/z_eff_ss_hitmark.h"
#include "assets/objects/gameplay_keep/gameplay_keep.h"
#include "assets/objects/object_knight/object_knight.h"
#include "z64player.h"

void EnKnight_SetupLowSwing(EnKnight* this, PlayState* play);
void EnKnight_HeavyAttack(EnKnight* this, PlayState* play);
void EnKnight_SetupLowSwingEnd(EnKnight* this, PlayState* play);
void EnKnight_SetupBasicSwing(EnKnight* this, PlayState* play);
void EnKnight_SetupWait(EnKnight* this, PlayState* play);
void EnKnight_SetupTurnToPlayer(EnKnight* this, PlayState* play);
void EnKnight_SetupJumpBackwardsIgos(EnKnight* this, PlayState* play);
void EnKnight_March(EnKnight* this, PlayState* play);
void EnKnight_IgosSitting(EnKnight* this, PlayState* play);
void EnKnight_LookAtOther(EnKnight* this, PlayState* play);
void EnKnight_JumpAttack(EnKnight* this, PlayState* play);
void EnKnight_CaptainsHatCS(EnKnight* this, PlayState* play);

void EnKnight_SetupTelegraphHeavyAttack(EnKnight* this, PlayState* play, s32 noTelegraph);
void EnKnight_SetupStrafe(EnKnight* this, PlayState* play, s16 strafeAngle);
void EnKnight_SetupRetreat(EnKnight* this, PlayState* play, u8 shielding);

extern void EnKnight_SetupJumpAttack(EnKnight*, PlayState*);
extern void EnKnight_SetupBlocking(EnKnight*, PlayState*);
extern void EnKnight_SetupKnightCaptainsHatCS(EnKnight*, PlayState*);

void func_80833B18(PlayState* play, Player* this, s32 arg2, f32 speed, f32 velocityY, s16 arg5, s32 invincibilityTimer);

extern AnimationHeader gKnightIdleAnim;
extern AnimationHeader gKnightMarchAnim;
extern AnimationHeader gKnightJumpAttackEndAnim;

typedef enum {
    /*  0 */ KNIGHT_DMGEFF_NONE,
    /*  1 */ KNIGHT_DMGEFF_STUN,
    /*  2 */ KNIGHT_DMGEFF_FIRE,
    /*  3 */ KNIGHT_DMGEFF_ICE,
    /*  4 */ KNIGHT_DMGEFF_LIGHT,
    /* 10 */ KNIGHT_DMGEFF_ZORA_BARRIER = 10,
    /* 13 */ KNIGHT_DMGEFF_GORON_POUND = 13,
    /* 14 */ KNIGHT_DMGEFF_LIGHT_RAY,
    /* 15 */ KNIGHT_DMGEFF_OTHER
} EnKnightDamageEffect;

static DamageTable sDamageTableStanding = {
    /* Deku Nut       */ DMG_ENTRY(0, KNIGHT_DMGEFF_STUN),
    /* Deku Stick     */ DMG_ENTRY(1, KNIGHT_DMGEFF_OTHER),
    /* Horse trample  */ DMG_ENTRY(0, KNIGHT_DMGEFF_NONE),
    /* Explosives     */ DMG_ENTRY(1, KNIGHT_DMGEFF_OTHER),
    /* Zora boomerang */ DMG_ENTRY(0, KNIGHT_DMGEFF_STUN),
    /* Normal arrow   */ DMG_ENTRY(3, KNIGHT_DMGEFF_OTHER),
    /* UNK_DMG_0x06   */ DMG_ENTRY(0, KNIGHT_DMGEFF_NONE),
    /* Hookshot       */ DMG_ENTRY(0, KNIGHT_DMGEFF_OTHER),
    /* Goron punch    */ DMG_ENTRY(1, KNIGHT_DMGEFF_OTHER),
    /* Sword          */ DMG_ENTRY(1, KNIGHT_DMGEFF_OTHER),
    /* Goron pound    */ DMG_ENTRY(0, KNIGHT_DMGEFF_GORON_POUND),
    /* Fire arrow     */ DMG_ENTRY(1, KNIGHT_DMGEFF_FIRE),
    /* Ice arrow      */ DMG_ENTRY(1, KNIGHT_DMGEFF_ICE),
    /* Light arrow    */ DMG_ENTRY(2, KNIGHT_DMGEFF_LIGHT),
    /* Goron spikes   */ DMG_ENTRY(1, KNIGHT_DMGEFF_OTHER),
    /* Deku spin      */ DMG_ENTRY(0, KNIGHT_DMGEFF_STUN),
    /* Deku bubble    */ DMG_ENTRY(2, KNIGHT_DMGEFF_OTHER),
    /* Deku launch    */ DMG_ENTRY(1, KNIGHT_DMGEFF_OTHER),
    /* UNK_DMG_0x12   */ DMG_ENTRY(0, KNIGHT_DMGEFF_NONE),
    /* Zora barrier   */ DMG_ENTRY(0, KNIGHT_DMGEFF_ZORA_BARRIER),
    /* Normal shield  */ DMG_ENTRY(0, KNIGHT_DMGEFF_NONE),
    /* Light ray      */ DMG_ENTRY(0, KNIGHT_DMGEFF_LIGHT_RAY),
    /* Thrown object  */ DMG_ENTRY(1, KNIGHT_DMGEFF_OTHER),
    /* Zora punch     */ DMG_ENTRY(1, KNIGHT_DMGEFF_OTHER),
    /* Spin attack    */ DMG_ENTRY(1, KNIGHT_DMGEFF_OTHER),
    /* Sword beam     */ DMG_ENTRY(0, KNIGHT_DMGEFF_NONE),
    /* Normal Roll    */ DMG_ENTRY(0, KNIGHT_DMGEFF_NONE),
    /* UNK_DMG_0x1B   */ DMG_ENTRY(0, KNIGHT_DMGEFF_NONE),
    /* UNK_DMG_0x1C   */ DMG_ENTRY(0, KNIGHT_DMGEFF_NONE),
    /* Unblockable    */ DMG_ENTRY(0, KNIGHT_DMGEFF_NONE),
    /* UNK_DMG_0x1E   */ DMG_ENTRY(0, KNIGHT_DMGEFF_NONE),
    /* Powder Keg     */ DMG_ENTRY(1, KNIGHT_DMGEFF_OTHER),
};

typedef struct {
    /* 0x00 */ Vec3f pos;
    /* 0x0C */ Vec3f velocity;
    /* 0x18 */ Vec3f accel;
    /* 0x24 */ UNK_TYPE1 unk_24[0x6];
    /* 0x2A */ u8 active;
    /* 0x2C */ s16 scrollTimer;
    /* 0x2E */ s16 alpha;
    /* 0x30 */ s16 unk_30; // set to 0 but unused
    /* 0x34 */ f32 scale;
    /* 0x38 */ f32 scaleTarget;
} EnKnightEffect; // size = 0x3C

EnKnight* sIgosInstance;
EnKnight* sThinKnightInstance;
EnKnight* sWideKnightInstance;
EnKnight* sIgosHeadInstance;
EnKnight* sTargetKnight; // During some actions, the other knight will use this knight as a target
MirRay3* sMirRayInstance;
EnKnightEffect sEnKnightEffects[100];

typedef enum {
    /* 0 */ KNIGHT_SUB_ACTION_MARCH_0,
    /* 1 */ KNIGHT_SUB_ACTION_MARCH_1,
    /* 2 */ KNIGHT_SUB_ACTION_MARCH_2
} EnKnightSubActionMarch;

typedef enum {
    /* 0 */ KNIGHT_CS_2_STATE_0,
    /* 1 */ KNIGHT_CS_2_STATE_1,
    /* 2 */ KNIGHT_CS_2_STATE_2,
    /* 3 */ KNIGHT_CS_2_STATE_3,
    /* 4 */ KNIGHT_CS_2_STATE_4
} EnKnightCaptainsHatCsState;

typedef enum {
    /* 0 */ KNIGHT_SUB_ACTION_CAPTAINS_HAT_CS_IGOS_SITTING,
    /* 1 */ KNIGHT_SUB_ACTION_CAPTAINS_HAT_CS_IGOS_STANDING
} EnKnightSubActionCaptainsHatCS;

RECOMP_PATCH void EnKnight_SetupMarch(EnKnight* this, PlayState* play) {

    int Difficulty = (int)recomp_get_config_double("diff_option");

    if (Difficulty == 1) {

        this->hasFinishedInteraction = true;
        return;
    }
    else {
        this->actionFunc = EnKnight_March;
        Animation_MorphToLoop(&this->skelAnime, &gKnightIdleAnim, -5.0f);
        this->subAction = KNIGHT_SUB_ACTION_MARCH_0;
        this->timers[0] = Rand_ZeroFloat(10.0f) + 65.0f;
    }
}

RECOMP_PATCH void EnKnight_JumpAttack(EnKnight* this, PlayState* play) {
    int Difficulty = (int)recomp_get_config_double("diff_option");
    Player* player = GET_PLAYER(play);

    if (this->timers[0] > 0) {
        s16 targetYaw = Math_Vec3f_Yaw(&this->actor.world.pos, &player->actor.world.pos);
        s16 rotStep = (Difficulty >= 1) ? 0x800 : 0x300;

        Math_SmoothStepToS(&this->actor.world.rot.y, targetYaw, 1, rotStep, 0);
        this->actor.shape.rot.y = this->actor.world.rot.y;
    }

    if ((this->timers[0] > 1) && (this->timers[0] < 6)) {
        this->blureAlpha = 255.0f;
        this->blureTranslation.x = -19.0f;
        this->blureTranslation.y = 46.0f;
        this->blureTranslation.z = 17.0f;
        this->blureRotation.x = -0.5497787f;
        this->blureRotation.y = 2.9059734f;
        this->blureRotation.z = -1.1780972f;
    }

    SkelAnime_Update(&this->skelAnime);
    if (Animation_OnFrame(&this->skelAnime, this->animEndFrame)) {
        Animation_MorphToPlayOnce(&this->skelAnime, &gKnightJumpAttackEndAnim, 0.0f);
        Actor_PlaySfx(&this->actor, this->attackSfx);
        this->animEndFrame = 1000.0f;
    }

    this->actor.speed = 0.0f;
    if ((this->actor.velocity.y <= 0.0f) && (this->actor.bgCheckFlags & BGCHECKFLAG_GROUND)) {
        this->animMovementZ = 0.0f;
        this->animMovementX = 0.0f;
        Actor_PlaySfx(&this->actor, NA_SE_EN_EYEGOLE_ATTACK);
    }

    if (this->timers[0] == 0) {
        EnKnight_SetupWait(this, play);
    }

    this->swordColliderActive = true;
}

RECOMP_PATCH void EnKnight_BasicSwing(EnKnight* this, PlayState* play) {

    int Difficulty = (int)recomp_get_config_double("diff_option");

    SkelAnime_Update(&this->skelAnime);

    if (Animation_OnFrame(&this->skelAnime, this->animEndFrame)) {
        if ((Rand_ZeroOne() < 0.6f) && (this->actor.xzDistToPlayer <= 100.0f)) {
            if (Rand_ZeroOne() < 0.5f) {
                EnKnight_SetupBasicSwing(this, play);
            }
            else {
                EnKnight_SetupTelegraphHeavyAttack(this, play, false);
            }
        }
        else {
            EnKnight_SetupWait(this, play);
        }
    }

    s16 scale = (Difficulty >= 1) ? 2 : 5;
    s16 step = (Difficulty >= 1) ? 0x800 : 0x500;

    Math_ApproachS(&this->actor.world.rot.y, this->yawToPlayer, scale, step);

    this->actor.shape.rot.y = this->actor.world.rot.y;

    Math_ApproachZeroF(&this->actor.speed, 1.0f, 1.0f);

    if (this->timers[0] == 16) {
        this->swordColliderActive = true;
    }
}

RECOMP_HOOK("EnKnight_SetupTelegraphHeavyAttack") void FasterTeleHeavy(EnKnight* this, PlayState* play, s32 noTelegraph) {

    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        if (this == sIgosInstance) {
            if (noTelegraph) {
                this->timers[0] = 0;
            }
            else {
                this->timers[0] = 3;
            }
        }
        else {
            this->timers[0] = 7;
        }
        break;

    case 1:
        this->timers[0] = 5;
        break;

    default:
        break;
    }
}

RECOMP_PATCH void EnKnight_LowSwing(EnKnight* this, PlayState* play) {
    Vec3f translation;
    int Difficulty = (int)recomp_get_config_double("diff_option");

    this->neckYawTarget = 0;

    s16 turnStep = (Difficulty >= 1) ? 0x1500 : 0x800;
    Math_ApproachS(&this->actor.world.rot.y, this->yawToPlayer, 2, turnStep);
    this->actor.shape.rot.y = this->actor.world.rot.y;

    if ((this->skelAnime.curFrame >= 12.0f) && (this->skelAnime.curFrame <= 15.0f)) {
        this->blureAlpha = 255.0f;
        this->blureTranslation.x = -20.0f;
        this->blureTranslation.y = 47.0f;
        this->blureTranslation.z = 0.0f;
        this->blureRotation.x = M_PIf;
        this->blureRotation.y = 0.f;
        this->blureRotation.z = 0.47123894f;
    }

    SkelAnime_Update(&this->skelAnime);
    Math_ApproachZeroF(&this->actor.speed, 1.0f, 1.0f);

    if ((this->skelAnime.curFrame >= 12.0f) && (this->skelAnime.curFrame <= 15.0f)) {
        Matrix_RotateYS(this->actor.world.rot.y, MTXMODE_NEW);

        float lungeDist = (Difficulty >= 1) ? 9.0f : 5.0f;

        Matrix_MultVecZ(lungeDist, &translation);
        this->animMovementX = translation.x;
        this->animMovementZ = translation.z;
    }

    if (Animation_OnFrame(&this->skelAnime, 12.0f)) {
        Actor_PlaySfx(&this->actor, this->attackSfx);
    }

    if (Animation_OnFrame(&this->skelAnime, this->animEndFrame)) {

        float chainChance = (Difficulty >= 1) ? 0.6f : 0.3f;

        if ((Rand_ZeroOne() < chainChance) && (this->actor.xzDistToPlayer <= 150.0f)) {

            if (Rand_ZeroOne() < 0.5f) {
                EnKnight_HeavyAttack(this, play);
            }
            else {
                EnKnight_SetupBasicSwing(this, play);
            }
        }
        else {
            EnKnight_SetupLowSwingEnd(this, play);
        }
    }

    if (this->skelAnime.curFrame >= 12.0f) {
        this->swordColliderActive = true;
    }
}

RECOMP_PATCH void EnKnight_HeavyAttack(EnKnight* this, PlayState* play) {
    Vec3f translation;
    int Difficulty = (int)recomp_get_config_double("diff_option");

    this->neckYawTarget = 0;

    if (this->skelAnime.curFrame < 2.0f) {
        if (Difficulty >= 1) {
            this->actor.world.rot.y = this->yawToPlayer;
            this->actor.shape.rot.y = this->yawToPlayer;
        }
        else {
            Math_ApproachS(&this->actor.world.rot.y, this->yawToPlayer, 1, 0x2000);
            this->actor.shape.rot.y = this->actor.world.rot.y;
        }
    }
    else if (this->skelAnime.curFrame >= 2.0f && this->skelAnime.curFrame <= 5.0f) {
        s16 turnStep = (Difficulty >= 1) ? 0x3000 : 0x1000;
        Math_ApproachS(&this->actor.world.rot.y, this->yawToPlayer, 2, turnStep);
        this->actor.shape.rot.y = this->actor.world.rot.y;
    }

    if (this->skelAnime.curFrame >= 3.0f && this->skelAnime.curFrame <= 6.0f) {
        this->blureAlpha = 255.0f;
        this->blureTranslation.x = -19.0f;
        this->blureTranslation.y = 46.0f;
        this->blureTranslation.z = 17.0f;
        this->blureRotation.x = -0.5497787f;
        this->blureRotation.y = 2.9059734f;
        this->blureRotation.z = -1.1780972f;
    }

    SkelAnime_Update(&this->skelAnime);
    Math_ApproachZeroF(&this->actor.speed, 1.0f, 1.0f);

    if ((this->skelAnime.curFrame >= 2.0f) && (this->skelAnime.curFrame <= 5.0f)) {
        Matrix_RotateYS(this->actor.world.rot.y, MTXMODE_NEW);

        float lungeDist = (Difficulty >= 1) ? 15.0f : 8.0f;

        Matrix_MultVecZ(lungeDist, &translation);
        this->animMovementX = translation.x;
        this->animMovementZ = translation.z;
    }

    if (Animation_OnFrame(&this->skelAnime, 1.0f)) {
        Actor_PlaySfx(&this->actor, this->attackSfx);
    }

    if (Animation_OnFrame(&this->skelAnime, this->animEndFrame)) {
        if ((Rand_ZeroOne() < 0.5f) && (this->actor.xzDistToPlayer <= 100.0f)) {
            EnKnight_SetupLowSwing(this, play);
        }
        else {
            EnKnight_SetupWait(this, play);
        }
    }

    this->swordColliderActive = true;
}

RECOMP_PATCH void EnKnight_Wait(EnKnight* this, PlayState* play) {
    SkelAnime_Update(&this->skelAnime);

    int Difficulty = (int)recomp_get_config_double("diff_option");
    const s16 yawDiff = this->yawToPlayer - this->actor.shape.rot.y;
    const s32 yawCap = (this == sIgosInstance) ? 0x7FFF : (Difficulty >= 1 ? 0x7500 : 0x6000);

    if (ABS_ALT(yawDiff) < yawCap) {
        EnKnight_SetupTurnToPlayer(this, play);
    }
    else {
        const int fidgetFreq = (Difficulty >= 1) ? 4 : 8;

        if ((this->randTimer % fidgetFreq == 0) && (Rand_ZeroOne() < 0.75f)) {
            this->neckYawTarget = Rand_CenteredFloat(28672.0f);
        }

        if (this->timers[0] == 0) {
            EnKnight_SetupStrafe(this, play, 0x4000);
        }
    }

    Math_ApproachZeroF(&this->actor.speed, 1.0f, 1.0f);
    this->canRetreat = true;
}

RECOMP_HOOK("EnKnight_RecoilFromDamage") void FasterRecoverKN(EnKnight* this, PlayState* play) {
    f32 rand = Rand_ZeroOne();

    int Difficulty = (int)recomp_get_config_double("diff_option");

    if (this != sIgosInstance) {

        switch (Difficulty) {
        case 0:
            this->skelAnime.playSpeed = 2.5f;
            break;
        case 1:
            this->skelAnime.playSpeed = 5.0f;
            break;
        default:
            break;
        }
    }
    else {

        if (rand < 0.25f) {
            EnKnight_SetupLowSwing(this, play);
        }
        else if (rand < 0.5f) {
            EnKnight_SetupTelegraphHeavyAttack(this, play, false);
        }
        else if (rand < 0.75f) {
            EnKnight_SetupJumpBackwardsIgos(this, play);
        }
        else {
            EnKnight_SetupBlocking(this, play);
        }
    }
}

RECOMP_HOOK("EnKnight_ApproachPlayer") void speedupknight(EnKnight* this, PlayState* play) {

    int Difficulty = (int)recomp_get_config_double("diff_option");
    float speedMultiplier = 1.0f;

    if (this != sIgosInstance) {

        switch (Difficulty) {
        case 0:
            speedMultiplier = 1.25f;
            break;

        case 1:
            speedMultiplier = 1.5f;
            break;

        default:
            break;
        }

        if (this->actor.colChkInfo.health != 0) {
            this->skelAnime.playSpeed = speedMultiplier * 1.5;
        }

        if (this->actor.speed <= 4.0f) {
            this->actor.speed *= speedMultiplier;
        }
    }
}

RECOMP_PATCH void EnKnight_SetupCaptainsHatCS(EnKnight* this, PlayState* play) {
    int Difficulty = (int)recomp_get_config_double("diff_option");

    if (Difficulty == 1) {

        return;
    }

    this->csTimer = 0;
    this->csState = KNIGHT_CS_2_STATE_0;
    this->csStepValue = 0.0f;

    SEQCMD_STOP_SEQUENCE(SEQ_PLAYER_BGM_MAIN, 10);

    if (this->actionFunc != EnKnight_IgosSitting) {
        this->subAction = KNIGHT_SUB_ACTION_CAPTAINS_HAT_CS_IGOS_STANDING;
        this->actor.shape.rot.y = this->actor.yawTowardsPlayer;
        this->actor.world.rot.y = this->actor.yawTowardsPlayer;
    }
    else {
        this->subAction = KNIGHT_SUB_ACTION_CAPTAINS_HAT_CS_IGOS_SITTING;
    }

    EnKnight_SetupKnightCaptainsHatCS(sThinKnightInstance, play);
    EnKnight_SetupKnightCaptainsHatCS(sWideKnightInstance, play);
    this->prevActionFunc = this->actionFunc;
    this->actionFunc = EnKnight_CaptainsHatCS;
}


RECOMP_HOOK("EnKnight_FallOver") void wakeywakey(EnKnight* this, PlayState* play) {

    s16 timerTarget;
    int Difficulty = (int)recomp_get_config_double("diff_option");

    if (Difficulty == 1) {
        this->timers[1] = this->timers[1] - 3;
        if (this->timers[1] < 0) this->timers[1] = 0;
    }
    else {
        this->timers[1]--;
        if (this->timers[1] < 0) this->timers[1] = 0;
    }
}

RECOMP_PATCH void EnKnight_SetupLookAtOther(EnKnight* this, PlayState* play) {
    int Difficulty = (int)recomp_get_config_double("diff_option");
    if (Difficulty == 1) return;

    this->actionFunc = EnKnight_LookAtOther;
    Animation_MorphToLoop(&this->skelAnime, &gKnightIdleAnim, -5.0f);
}

RECOMP_PATCH void EnKnight_CheckRetreat(EnKnight* this, PlayState* play) {
    Player* player;
    Actor* explosive;
    f32 px;
    f32 pz;
    f32 dx;
    f32 dz;

    int Difficulty = (int)recomp_get_config_double("diff_option");

    if (this->isHeadless) {
        return;
    }

    player = GET_PLAYER(play);

    for (explosive = play->actorCtx.actorLists[ACTORCAT_EXPLOSIVES].first; explosive != NULL;
        explosive = explosive->next) {
        px = this->actor.world.pos.x;
        pz = this->actor.world.pos.z;
        dx = explosive->world.pos.x - px;
        dz = explosive->world.pos.z - pz;
        if (sqrtf(SQ(dx) + SQ(dz)) < 100.0f) {
            this->retreatTowards.x = px - dx * 100.0f;
            this->retreatTowards.z = pz - dz * 100.0f;
            EnKnight_SetupRetreat(this, play, false);
        }
    }
    if (Difficulty == 0) {
        if ((sMirRayInstance != NULL) && (sMirRayInstance->unk_214 > 0.1f) &&
            (this->actor.xzDistToPlayer <= (BREG(70) + 300.0f))) {
            px = this->actor.world.pos.x;
            pz = this->actor.world.pos.z;
            dx = player->actor.world.pos.x - px;
            dz = player->actor.world.pos.z - pz;
            this->retreatTowards.x = px - dx * 100.0f;
            this->retreatTowards.z = pz - dz * 100.0f;
            EnKnight_SetupRetreat(this, play, true);
        }
    }
}

RECOMP_HOOK("EnKnight_Update") void KniUpd(Actor* thisx, PlayState* play) {

    Player* player = GET_PLAYER(play);
    EnKnight* this = (EnKnight*)thisx;
    s32 limbIndex;
    int Difficulty = (int)recomp_get_config_double("diff_option");

    #define HEALTH_SCALED_FLAG 0x8000

    switch (Difficulty) {
    case 0:
        this->actor.colChkInfo.damage = (this->actor.colChkInfo.damage) / 2;
        this->timers[0]--;
        if (this->timers[0] < 0) this->timers[0] = 0;
        break;

    case 1:
        this->actor.colChkInfo.damage = (this->actor.colChkInfo.damage + 2) / 3;

        if ((this != sIgosInstance) && (player->stateFlags3 & PLAYER_STATE3_20000000)) {
            if (this->hasFinishedInteraction == true) {
                if (sTargetKnight != NULL) EnKnight_SetupWait(this, play);
            }
            else {
                if (sTargetKnight != NULL) EnKnight_SetupMarch(this, play);
            }
            this->swordColliderActive = true;
            if (Rand_ZeroOne() < 0.5f) this->canRetreat = true;
            this->timers[0] = this->timers[0] - 2;
            if (this->timers[0] < 0) this->timers[0] = 0;
            break;

    default:
        break;
        }
    }

    if (!(this->actor.params & HEALTH_SCALED_FLAG)) {
        switch (Difficulty) {
        case 0:
            this->actor.colChkInfo.health = (this->actor.colChkInfo.health * 1.25f);
            break;
        case 1:
            this->actor.colChkInfo.health = (this->actor.colChkInfo.health * 1.5f);
            break;
        default:
            break;
        }
        this->actor.params |= HEALTH_SCALED_FLAG;
    }
}