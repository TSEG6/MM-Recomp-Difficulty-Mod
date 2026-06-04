#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_en_vm.h"
#include "overlays/actors/ovl_En_Bom/z_en_bom.h"
#include "assets/objects/gameplay_keep/gameplay_keep.h"

void func_808CC420(EnVm* this);
void func_808CC788(EnVm* this);

void func_80833B18(PlayState* play, Player* this, s32 arg2, f32 speed, f32 velocityY, s16 arg5, s32 invincibilityTimer);

// Player prediction based on difficulty
RECOMP_PATCH void func_808CC610(EnVm* this, PlayState* play) {
    static Color_RGBA8 sPrimColor = { 0, 0, 255, 0 };
    static Color_RGBA8 sEnvColor = { 255, 255, 255, 255 };
    Player* player = GET_PLAYER(play);
    s16 sp3A;
    s16 sp38;

    int Difficulty = (int)recomp_get_config_double("diff_option");
    float distMult = 1.0f;
    float predictFactor = 0.0f;

    switch (Difficulty) {
    case 0:
        distMult = 2.5f;
        predictFactor = 5.0f;
        break;

    case 1:
        distMult = 4.0f;
        predictFactor = 15.0f;;
        break;

    default:
        break;
    }

    Vec3f targetPos = player->actor.world.pos;
    targetPos.x += player->actor.velocity.x * predictFactor;
    targetPos.z += player->actor.velocity.z * predictFactor;

    SkelAnime_Update(&this->skelAnime);

    sp38 = Math_Vec3f_Pitch(&this->actor.focus.pos, &targetPos);
    sp38 = CLAMP_MAX(sp38, 0x1B91);
    s16 targetYaw = Math_Vec3f_Yaw(&this->actor.world.pos, &targetPos);

    sp3A = BINANG_ADD((s32)(((this->unk_21C * distMult) - this->actor.xzDistToPlayer) * 60.0f), 0xFA0);
    sp3A = CLAMP_MAX(sp3A, 0x1000);

    Math_SmoothStepToS(&this->unk_216, sp38, 10, 0xFA0, 0);

    if ((sp38 < 0xAAA) || (sp3A <= 0)) {
        func_808CC420(this);
    }
    else if (Math_ScaledStepToS(&this->unk_218, BINANG_SUB(targetYaw, this->actor.shape.rot.y),
        sp3A)) {
        this->unk_214--;
        if (this->unk_214 == 0) {
            EffectSsDeadDd_Spawn(play, &this->unk_228, &gZeroVec3f, &gZeroVec3f, &sPrimColor, &sEnvColor, 150, -25, 16,
                20);
            func_808CC788(this);
        }
    }
}

// Applies a shock to the player on hard if attacked
RECOMP_HOOK("EnVm_Update") void electricgrave(Actor* thisx, PlayState* play) {

    EnVm* this = (EnVm*)thisx;
    Player* player = GET_PLAYER(play);

    int Difficulty = (int)recomp_get_config_double("diff_option");

    if (this->colliderTris.base.atFlags & AT_HIT) {
    
        switch (Difficulty) {
        case 0:
            break;

        case 1:
            func_80833B18(play, player, 4, 0.0f, 0.0f, 0, 10);
            break;

        default:
            break;
        }
    }
}