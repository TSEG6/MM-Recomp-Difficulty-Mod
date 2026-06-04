#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_en_am.h"
#include "overlays/actors/ovl_En_Bom/z_en_bom.h"
#include "overlays/actors/ovl_En_Bombf/z_en_bombf.h"
#include "overlays/actors/ovl_En_Clear_Tag/z_en_clear_tag.h"

void EnAm_WakeUp(EnAm* this);
void func_808B0460(EnAm* this);
void func_808B0640(EnAm* this);
void func_808B03C0(EnAm* this, PlayState* play);

extern void func_808B0208(EnAm*, PlayState*);

// If on Hard it'll wake up by proximity
RECOMP_HOOK("EnAm_Update") void ArmosUpdate(Actor* thisx, PlayState* play) {
    EnAm* this = (EnAm*)thisx;
    Player* player = GET_PLAYER(play);

    int Difficulty = (int)recomp_get_config_double("diff_option");
    float DistancePlayer = this->actor.xzDistToPlayer;

    if (Difficulty == 1 && DistancePlayer < 200 && this->textureBlend == 0){

        this->enemyCollider.base.acFlags |= AC_ON;
        EnAm_WakeUp(this);
    }
}

// Speed increases
RECOMP_PATCH void func_808B0358(EnAm* this) {
    Animation_PlayLoopSetSpeed(&this->skelAnime, &gArmosHopAnim, 4.0f);
    this->explodeTimer = 3;
    this->actor.speed = 0.0f;
    this->actor.world.rot.y = this->actor.shape.rot.y;
    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        this->speed = 8.0f;
        break;

    case 1:
        this->speed = 12.0f;
        break;

    default:
        break;
    }
    this->actionFunc = func_808B03C0;
}

// Changes how far away it can go before going back to home
RECOMP_PATCH void func_808B03C0(EnAm* this, PlayState* play) {

    int Difficulty = (int)recomp_get_config_double("diff_option");
    float awayhomedist = 240.0f;

    switch (Difficulty) {
    case 0:
        awayhomedist = 500.0f;
        break;

    case 1:
        awayhomedist = 750.0f;
        break;

    default:
        break;
    }

    this->armosYaw = this->actor.yawTowardsPlayer;
    func_808B0208(this, play);
    if (this->actor.bgCheckFlags & BGCHECKFLAG_GROUND_TOUCH) {
        this->explodeTimer--;
    }
    if (this->explodeTimer == 0) {
        func_808B0640(this);
    }
    else if ((this->returnHomeTimer == 0) || Actor_WorldDistXZToPoint(&this->actor, &this->actor.home.pos) > awayhomedist) {
        func_808B0460(this);
    }
}