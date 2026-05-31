#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"

#include "z_en_syateki_crow.h"
#include "overlays/actors/ovl_En_Syateki_Man/z_en_syateki_man.h"

#include "z_en_syateki_dekunuts.h"
#include "overlays/actors/ovl_En_Syateki_Man/z_en_syateki_man.h"
#include "overlays/effects/ovl_Effect_Ss_Hahen/z_eff_ss_hahen.h"

#include "z_en_syateki_wf.h"
#include "overlays/actors/ovl_En_Syateki_Man/z_en_syateki_man.h"

void EnSyatekiWf_Draw(Actor* thisx, PlayState* play);
void EnSyatekiWf_Run(EnSyatekiWf* this, PlayState* play);
void EnSyatekiWf_Howl(EnSyatekiWf* this, PlayState* play);

typedef enum ShootingGalleryWolfosAnimation {
    /* 0 */ SG_WOLFOS_ANIM_WAIT, // unused
    /* 1 */ SG_WOLFOS_ANIM_RUN,
    /* 2 */ SG_WOLFOS_ANIM_JUMP,
    /* 3 */ SG_WOLFOS_ANIM_LAND,
    /* 4 */ SG_WOLFOS_ANIM_BACKFLIP, // unused
    /* 5 */ SG_WOLFOS_ANIM_DAMAGED,
    /* 6 */ SG_WOLFOS_ANIM_REAR_UP_FALL_OVER,
    /* 7 */ SG_WOLFOS_ANIM_MAX
} ShootingGalleryWolfosAnimation;

static AnimationInfo sAnimationInfo[SG_WOLFOS_ANIM_MAX] = {
    { &gWolfosWaitAnim, 2.0f, 0.0f, 0.0f, ANIMMODE_LOOP, -1.0f },           // SG_WOLFOS_ANIM_WAIT
    { &gWolfosRunAnim, 1.0f, 0.0f, 0.0f, ANIMMODE_LOOP, -8.0f },            // SG_WOLFOS_ANIM_RUN
    { &gWolfosRunAnim, 1.0f, 0.0f, 4.0f, ANIMMODE_ONCE, 1.0f },             // SG_WOLFOS_ANIM_JUMP
    { &gWolfosRunAnim, 1.0f, 4.0f, 8.0f, ANIMMODE_ONCE, 1.0f },             // SG_WOLFOS_ANIM_LAND
    { &gWolfosBackflipAnim, 1.0f, 0.0f, 0.0f, ANIMMODE_ONCE, -1.0f },       // SG_WOLFOS_ANIM_BACKFLIP
    { &gWolfosDamagedAnim, 1.0f, 0.0f, 0.0f, ANIMMODE_ONCE, 8.0f },         // SG_WOLFOS_ANIM_DAMAGED
    { &gWolfosRearUpFallOverAnim, 1.0f, 0.0f, 0.0f, ANIMMODE_ONCE, -1.0f }, // SG_WOLFOS_ANIM_REAR_UP_FALL_OVER
};


void EnSyatekiCrow_Fly(EnSyatekiCrow* this, PlayState* play);

static Vec3f sVelocity = { 0.0f, 20.0f, 0.0f };

static Vec3f sAccel = { 0.0f, 0.0f, 0.0f };


void EnSyatekiDekunuts_WaitToStart(EnSyatekiDekunuts* this, PlayState* play);

typedef enum {
    /* 0 */ SG_DEKU_HEADDRESS_TYPE_NORMAL,
    /* 1 */ SG_DEKU_HEADDRESS_TYPE_FLIPPED_UP
} ShootingGalleryDekuScrubHeaddressType;


RECOMP_PATCH void EnSyatekiWf_SetupHowl(EnSyatekiWf* this) {

    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        this->timer = 30;
        break;

    case 1:
        this->timer = 20;
        break;

    default:
        break;
    }

    this->actor.speed = 0.0f;
    Actor_PlaySfx(&this->actor, NA_SE_EN_WOLFOS_APPEAR);
    Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, SG_WOLFOS_ANIM_DAMAGED);
    this->actionFunc = EnSyatekiWf_Howl;
}


RECOMP_PATCH void EnSyatekiWf_SetupRun(EnSyatekiWf* this) {
    Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, SG_WOLFOS_ANIM_RUN);

    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        this->actor.speed = 12.5f;
        this->skelAnime.playSpeed = 1.25;
        break;

    case 1:
        this->actor.speed = 14.0f;
        this->skelAnime.playSpeed = 1.45;
        break;

    default:
        break;
    }

    this->actor.world.rot.y = this->actor.shape.rot.y;
    this->actor.draw = EnSyatekiWf_Draw;
    this->actionFunc = EnSyatekiWf_Run;
}





RECOMP_PATCH void EnSyatekiCrow_WaitToMove(EnSyatekiCrow* this, PlayState* play) {
    if (((SG_GUAY_GET_WAIT_MOD(&this->actor) * 20) + 20) < this->waitTimer) {
        Actor_PlaySfx(this->actor.parent, NA_SE_EN_KAICHO_CRY);
        this->waitTimer = 0;

        int Difficulty = (int)recomp_get_config_double("diff_option");

        switch (Difficulty) {
        case 0:
            this->actor.speed = SG_GUAY_GET_SPEED_MOD(&this->actor) + 9.0f;
            this->skelAnime.playSpeed = 1.5;
            break;

        case 1:
            this->actor.speed = SG_GUAY_GET_SPEED_MOD(&this->actor) + 12.0f;
            this->skelAnime.playSpeed = 2;
            break;

        default:
            break;
        }

        this->actor.gravity = -0.5f;
        this->actionFunc = EnSyatekiCrow_Fly;
    }
    else {
        this->waitTimer++;
    }
}





RECOMP_PATCH void EnSyatekiDekunuts_SetupWaitToStart(EnSyatekiDekunuts* this) {
    Vec3f pos;
    EnSyatekiMan* syatekiMan = (EnSyatekiMan*)this->actor.parent;

    this->timer = 0;
    pos.x = this->flowerPos[this->index].x;
    pos.y = this->flowerPos[this->index].y;
    pos.z = this->flowerPos[this->index].z;
    this->actor.world.pos = this->actor.prevPos = pos;
    this->actor.world.rot.y = this->actor.yawTowardsPlayer;
    this->actor.shape.rot.y = this->actor.yawTowardsPlayer;

    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        this->timeToBurrow = 93 - (syatekiMan->currentWave * 3);
        if (this->timeToBurrow < 40) this->timeToBurrow = 40;
        break;

    case 1:
        this->timeToBurrow = 70 - (syatekiMan->currentWave * 5);
        if (this->timeToBurrow < 40) this->timeToBurrow = 40;
        break;

    default:
        break;
    }

    if ((syatekiMan->currentWave % 2) != 0) {
        this->headdressType = SG_DEKU_HEADDRESS_TYPE_FLIPPED_UP;
        this->headdressRotZ = 0;
    }
    else {
        this->headdressType = SG_DEKU_HEADDRESS_TYPE_NORMAL;
    }

    this->actionFunc = EnSyatekiDekunuts_WaitToStart;
}