#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_en_bigpo.h"
#include "overlays/actors/ovl_En_Clear_Tag/z_en_clear_tag.h"
#include "assets/objects/object_bigpo/object_bigpo.h"
#include "assets/objects/gameplay_keep/gameplay_keep.h"

int TimesAttacked = 0;

void EnBigpo_SetupSpinAttack(EnBigpo* this);
void EnBigpo_SpinningDown(EnBigpo* this, PlayState* play);
void EnBigpo_SetupSpinUp(EnBigpo* this);

void EnBigpo_UpdateSpin(EnBigpo* this) {
    s16 oldYaw = this->actor.shape.rot.y;

    this->actor.shape.rot.y += this->angularVelocity;
    if ((oldYaw < 0) && (this->actor.shape.rot.y > 0)) {
        Actor_PlaySfx(&this->actor, NA_SE_EN_PO_ROLL); // spinning sfx during spin attack
    }
}

RECOMP_PATCH void EnBigpo_SetupSpinDown(EnBigpo* this) {

    int Difficulty = (int)recomp_get_config_double("diff_option");
    int AttackLimit = 1;

    switch (Difficulty) {
    case 0:
        AttackLimit = 2;
        break;

    case 1:
        AttackLimit = 3;
        break;

    default:
        break;
    }

    if (AttackLimit == TimesAttacked) {
        this->collider.base.atFlags &= ~AT_ON;
        this->actionFunc = EnBigpo_SpinningDown;
        TimesAttacked = 0;
    }
    else {
        EnBigpo_SetupSpinUp(this);
        TimesAttacked++;
    }
    recomp_printf("Attacked: 0x%08X\n", TimesAttacked);
    recomp_printf("AttackLimit: 0x%08X\n", AttackLimit);
}

RECOMP_HOOK("EnBigpo_SpinAttack") void BPSA(EnBigpo* this, PlayState* play) {


}

RECOMP_PATCH void EnBigpo_SpinAttack(EnBigpo* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    s16 yawDiff;

    int Difficulty = (int)recomp_get_config_double("diff_option");

    if (this->actor.speed <= 3.0f) {
        f32 attackSpeed = (Difficulty == 0) ? 15.0f : 20.0f;
        f32 leadScale = (Difficulty == 0) ? 0.6f : 1.2f;

        f32 frames = this->actor.xzDistToPlayer / attackSpeed;
        if (frames > 30.0f) {
            frames = 30.0f;
        }
        frames *= leadScale;

        f32 predX = player->actor.world.pos.x + (player->actor.velocity.x * frames);
        f32 predZ = player->actor.world.pos.z + (player->actor.velocity.z * frames);
        this->actor.world.rot.y = Math_Atan2S(predX - this->actor.world.pos.x, predZ - this->actor.world.pos.z);
    }

    SkelAnime_Update(&this->skelAnime);

    switch (Difficulty) {
    case 0:
        Math_StepToF(&this->actor.speed, 15.0f, 1.0f);
        break;

    case 1:
    default:
        Math_StepToF(&this->actor.speed, 20.0f, 1.0f);
        break;
    }

    Math_SmoothStepToF(&this->actor.world.pos.y, player->actor.world.pos.y, 0.3f, 7.5f, 1.0f);
    EnBigpo_UpdateSpin(this);
    yawDiff = this->actor.yawTowardsPlayer - this->actor.world.rot.y;

    // because acFlags AC_HARD and COL_MATERIAL_METAL, if we hit it means we contacted as attack
    if ((this->collider.base.atFlags & AT_HIT) ||
        ((ABS_ALT(yawDiff) > 0x4000) && (this->actor.xzDistToPlayer > 50.0f))) {
        // hit the player OR the poe has missed and flew past player
        EnBigpo_SetupSpinDown(this);
        this->actor.speed = 3.0;
    }
}

RECOMP_HOOK("EnBigpo_Update") void BPoeUpdate(Actor* thisx, PlayState* play) {

    EnBigpo* this = (EnBigpo*)thisx;

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