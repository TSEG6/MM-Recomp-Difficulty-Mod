#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_en_skb.h"
#include "attributes.h"
#include "overlays/actors/ovl_En_Encount4/z_en_encount4.h"
#include "overlays/actors/ovl_En_Part/z_en_part.h"
#include "overlays/effects/ovl_Effect_Ss_Hahen/z_eff_ss_hahen.h"

void func_80995068(EnSkb* this, PlayState* play);
void func_8099599C(EnSkb* this, PlayState* play);
void func_80995C24(EnSkb* this);
void func_80995A30(EnSkb* this);
void func_80995D3C(EnSkb* this);
s32 func_80996594(EnSkb* this, PlayState* play);

typedef enum StalchildAnimation {
    /*  0 */ STALCHILD_ANIM_0,
    /*  1 */ STALCHILD_ANIM_1,
    /*  2 */ STALCHILD_ANIM_2,
    /*  3 */ STALCHILD_ANIM_3,
    /*  4 */ STALCHILD_ANIM_4,
    /*  5 */ STALCHILD_ANIM_5,
    /*  6 */ STALCHILD_ANIM_6,
    /*  7 */ STALCHILD_ANIM_7,
    /*  8 */ STALCHILD_ANIM_8,
    /*  9 */ STALCHILD_ANIM_9,
    /* 10 */ STALCHILD_ANIM_10,
    /* 11 */ STALCHILD_ANIM_11,
    /* 12 */ STALCHILD_ANIM_12,
    /* 13 */ STALCHILD_ANIM_MAX
} StalchildAnimation;

static AnimationInfo sAnimationInfo[STALCHILD_ANIM_MAX] = {
    { &gStalchildWalkAnim, 1.0f, 0.0f, 0.0f, ANIMMODE_LOOP, -4.0 },          // STALCHILD_ANIM_0
    { &gStalchildStandUpAnim, 1.0f, 0.0f, 0.0f, ANIMMODE_ONCE, -1.0f },      // STALCHILD_ANIM_1
    { &gStalchildAttackAnim, 0.6f, 0.0f, 0.0f, ANIMMODE_ONCE_INTERP, 4.0f }, // STALCHILD_ANIM_2
    { &gStalchildStaggerAnim, 1.0f, 0.0f, 0.0f, ANIMMODE_ONCE, -4.0 },       // STALCHILD_ANIM_3
    { &gStalchildCollapseAnim, 1.0f, 0.0f, 0.0f, ANIMMODE_ONCE, -4.0 },      // STALCHILD_ANIM_4
    { &gStalchildSitLaughAnim, 1.0f, 0.0f, 0.0f, ANIMMODE_LOOP, -4.0 },      // STALCHILD_ANIM_5
    { &gStalchildSitTapToesAnim, 1.0f, 0.0f, 0.0f, ANIMMODE_LOOP, -4.0 },    // STALCHILD_ANIM_6
    { &gStalchildSwingOnBranchAnim, 1.0f, 0.0f, 0.0f, ANIMMODE_LOOP, -4.0 }, // STALCHILD_ANIM_7
    { &gStalchildStandUpAnim, 1.0f, 0.0f, 0.0f, ANIMMODE_ONCE, -8.0f },      // STALCHILD_ANIM_8
    { &gStalchildStandUpAnim, 1.0f, 0.0f, 0.0f, ANIMMODE_ONCE, -16.0f },     // STALCHILD_ANIM_9
    { &gStalchildStaggerAnim, 1.0f, 0.0f, 0.0f, ANIMMODE_ONCE, -8.0f },      // STALCHILD_ANIM_10
    { &gStalchildSaluteAnim, 1.0f, 0.0f, 0.0f, ANIMMODE_ONCE, -4.0 },        // STALCHILD_ANIM_11
    { &gStalchildIdleAnim, 1.0f, 0.0f, 0.0f, ANIMMODE_LOOP, -4.0 },          // STALCHILD_ANIM_12
};

void func_8099504C(EnSkb* this) {
    this->actionFunc = func_80995068;
    this->actor.speed = 0.0f;
}

void func_80994DA8(EnSkb* this, PlayState* play) {
    if (Actor_IsFacingPlayer(&this->actor, 0x11C7) && (this->actor.xzDistToPlayer < 60.0f) &&
        (Player_GetMask(play) != PLAYER_MASK_CAPTAIN)) {
        func_80995C24(this);
    }
    else {
        func_80995A30(this);
    }
}

void func_809958F4(EnSkb* this) {
    Animation_Change(&this->skelAnime, &gStalchildStandUpAnim, -1.0f, Animation_GetLastFrame(&gStalchildStandUpAnim),
        0.0f, ANIMMODE_ONCE, -4.0f);
    this->unk_3E4 = 0;
    this->actor.flags &= ~ACTOR_FLAG_ATTENTION_ENABLED;
    this->actor.speed = 0.0f;
    Actor_PlaySfx(&this->actor, NA_SE_EN_AKINDONUTS_HIDE);
    this->unk_3DE = 1;
    this->actionFunc = func_8099599C;
}

RECOMP_HOOK("func_80995C24") void PrepAtk(EnSkb* this) {

    int Difficulty = (int)recomp_get_config_double("diff_option");

    float speed = (Difficulty >= 1) ? 3.0f : 0.0f;
    this->actor.speed = speed;

}

RECOMP_PATCH void func_80995C84(EnSkb* this, PlayState* play) {

    int Difficulty = (int)recomp_get_config_double("diff_option");
 
    Math_SmoothStepToS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 1, 0x500, 0);
    this->actor.world.rot.y = this->actor.shape.rot.y;

    if (Animation_OnFrame(&this->skelAnime, 3.0f) && (this->unk_3E4 == 0)) {
        Actor_PlaySfx(&this->actor, NA_SE_EN_STALKID_ATTACK);
        this->unk_3E4 = 1;

        if (Difficulty == 1 && this->actor.xzDistToPlayer > 80.0f) {
            this->actor.speed = 12.0f;
            this->actor.velocity.y = 4.0f;
        }
    }
    else if (Animation_OnFrame(&this->skelAnime, 6.0f)) {
        this->unk_3E4 = 0;
    }

    if (Difficulty == 1 && this->actor.speed > 0.0f) {
        Math_StepToF(&this->actor.speed, 0.0f, 1.0f);
    }

    if (this->collider.base.atFlags & AT_BOUNCED) {
        this->collider.base.atFlags &= ~(AT_BOUNCED | AT_HIT);
        func_80995D3C(this);
    }
    else if (Animation_OnFrame(&this->skelAnime, this->skelAnime.endFrame)) {
        func_80994DA8(this, play);
    }
}

RECOMP_PATCH void func_80995A8C(EnSkb* this, PlayState* play) {

    int Difficulty = (int)recomp_get_config_double("diff_option");

    if (Player_GetMask(play) == PLAYER_MASK_CAPTAIN) {
        this->actor.flags &= ~(ACTOR_FLAG_ATTENTION_ENABLED | ACTOR_FLAG_HOSTILE);
        this->actor.flags |= (ACTOR_FLAG_ATTENTION_ENABLED | ACTOR_FLAG_FRIENDLY);
        this->actor.hintId = TATL_HINT_ID_NONE;
        this->actor.colChkInfo.mass = MASS_HEAVY;
        Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, STALCHILD_ANIM_12);
        func_8099504C(this);
        return;
    }

    f32 speedMult = (Difficulty >= 1) ? 2.0f : 1.33f;

    this->skelAnime.playSpeed = speedMult;
    s16 updateRate = (Difficulty >= 1) ? 8 : 16;
    if ((this->unk_3D8 != 0) && ((play->gameplayFrames % updateRate) == 0)) {
        f32 jitterRange = (Difficulty >= 1) ? 75000.0f : 50000.0f;
        this->unk_3DA = Rand_CenteredFloat(jitterRange);
    }

    s16 rotStep = (Difficulty >= 1) ? 0x4AA : 0x2EE;
    Math_SmoothStepToS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer + this->unk_3DA, 1, rotStep, 0);
    this->actor.world.rot.y = this->actor.shape.rot.y;
    this->actor.speed = 2.0f * speedMult;

    if (Animation_OnFrame(&this->skelAnime, 8.0f / speedMult) || Animation_OnFrame(&this->skelAnime, 15.0f / speedMult)) {
        Actor_PlaySfx(&this->actor, NA_SE_EN_STALKID_WALK);
    }

    float attackDist = (Difficulty >= 1) ? 90.0f : 60.0f;
    s16 attackAngle = (Difficulty >= 1) ? 0x2000 : 0x11C7;

    if ((this->actor.xzDistToPlayer > 800.0f) || func_80996594(this, play)) {
        func_809958F4(this);
    }
    else if (Actor_IsFacingPlayer(&this->actor, attackAngle) && (this->actor.xzDistToPlayer < attackDist)) {
        func_80995C24(this);
    }
}

RECOMP_HOOK("EnSkb_Update") void SkullKidsUpdate(Actor* thisx, PlayState* play) {

	EnSkb* this = (EnSkb*)thisx;

    int Difficulty = (int)recomp_get_config_double("diff_option");

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