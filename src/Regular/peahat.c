#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "globalobjects_api.h"
#include "z_en_peehat.h"
#include "overlays/actors/ovl_En_Bom/z_en_bom.h"
#include "overlays/actors/ovl_En_Clear_Tag/z_en_clear_tag.h"
#include "overlays/effects/ovl_Effect_Ss_Hahen/z_eff_ss_hahen.h"

// doing funnies with home rotation so I can skip doing global variables in case you were curious

// Handles global objects
/*GLOBAL_OBJECTS_CALLBACK_ON_READY void onGlobalObjectsReady() {
    sPHEnemy = (uintptr_t)GlobalObjects_getGlobalObject(OBJECT_PH);
}


RECOMP_HOOK_RETURN("Actor_LoadOverlay") void on_return_Actor_LoadOverlay() {
    ActorProfile* profile = recomphook_get_return_ptr();
    if (profile != NULL && profile->id == ACTOR_EN_PEEHAT) {
        profile->objectId = GAMEPLAY_KEEP;
    }
}


RECOMP_HOOK("EnPeehat_Init")
void PH_Init(Actor* thisx, PlayState* play) {
    gSegments[0x06] = OS_K0_TO_PHYSICAL(sPHEnemy);
}


RECOMP_HOOK("EnPeehat_Update")
void PH_Update(Actor* thisx, PlayState* play) {
    gSegments[0x06] = OS_K0_TO_PHYSICAL(sPHEnemy);
}


RECOMP_HOOK("EnPeehat_Draw")
void PH_Draw(Actor* thisx, PlayState* play) {
    gSegments[0x06] = OS_K0_TO_PHYSICAL(sPHEnemy);

    OPEN_DISPS(play->state.gfxCtx);
    gSPSegment(POLY_OPA_DISP++, 0x06, sPHEnemy);
    gSPSegment(POLY_XLU_DISP++, 0x06, sPHEnemy);
    CLOSE_DISPS(play->state.gfxCtx);
}*/

// Increases health and animation speed
RECOMP_HOOK("EnPeehat_Update") void UpdateFun(Actor* thisx, PlayState* play2) {
    PlayState* play = play2;
    EnPeehat* this = (EnPeehat*)thisx;

    int Difficulty = (int)recomp_get_config_double("diff_option");
    u8 baseHealth = this->actor.colChkInfo.health;

    if (this->actor.home.rot.z == 0) {
        switch (Difficulty) {
        case 0:
            this->actor.colChkInfo.health = baseHealth * 1.25;
            break;

        case 1:
            this->actor.colChkInfo.health = baseHealth * 1.5;
            break;

        default:
            break;
        }
        this->actor.home.rot.z = 1;
    }

    switch (Difficulty) {
    case 0:
        this->skelAnime.playSpeed = 1.5f;
        break;

    case 1:
        this->skelAnime.playSpeed = 2.0f;
        break;

    default:
        break;
    }
}

// Increases actor speed
RECOMP_HOOK("func_80897F44") void IamSpeedPeahat(EnPeehat* this, PlayState* play) {
    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        this->actor.speed = Rand_ZeroFloat(0.75f) + 5.5f;
        this->actor.world.rot.y += this->unk_2B6;
        this->actor.shape.rot.y += 0x15E * 2;
        break;

    case 1:
        this->actor.speed = Rand_ZeroFloat(1.0f) + 7.5f;
        this->actor.world.rot.y += this->unk_2B6;
        this->actor.shape.rot.y += 0x15E * 6;
        break;

    default:
        this->actor.speed = Rand_ZeroFloat(0.5f) + 2.5f;
        break;
    }
}