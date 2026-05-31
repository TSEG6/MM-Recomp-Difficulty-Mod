#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_en_po_sisters.h"
#include "overlays/actors/ovl_En_Clear_Tag/z_en_clear_tag.h"
#include "assets/objects/gameplay_keep/gameplay_keep.h"

void EnPoSisters_SetupAimlessIdleFlying(EnPoSisters* this);
void EnPoSisters_SetupSpinAttack(EnPoSisters* this);
void EnPoSisters_SpinAttack(EnPoSisters* this, PlayState* play);
void EnPoSisters_DamageFlinch(EnPoSisters* this, PlayState* play);
void EnPoSisters_SpinToInvis(EnPoSisters* this, PlayState* play);
void EnPoSisters_SetupSpinBackToVisible(EnPoSisters* this, PlayState* play);
void EnPoSisters_SetupFlee(EnPoSisters* this);
void EnPoSisters_MegCloneVanish(EnPoSisters* this, PlayState* play);
void EnPoSisters_MegSurroundPlayer(EnPoSisters* this, PlayState* play);


#define POE_SISTERS_FLAG_UPDATE_SHAPE_ROT      (1 << 1)
#define POE_SISTERS_FLAG_REAL_MEG_ROTATION     (1 << 6) // Real Meg rotates different than her clones for one cycle

extern void EnPoSisters_MatchPlayerXZ(EnPoSisters*, PlayState*);
extern void EnPoSisters_SetupDamageFlinch(EnPoSisters*);

typedef enum {
    /* 0x0 */ POE_SISTERS_DMGEFF_NONE,
    /* 0x1 */ POE_SISTERS_DMGEFF_UNKDMG12, // set in DamageTable, but unused
    /* 0x4 */ POE_SISTERS_DMGEFF_LIGHTARROWS = 0x4,
    /* 0xE */ POE_SISTERS_DMGEFF_SPINATTACK = 0xE,
    /* 0xF */ POE_SISTERS_DMGEFF_DEKUNUT
} PoeSisterDamageEffect;

static DamageTable sDamageTable = {
    /* Deku Nut       */ DMG_ENTRY(0, POE_SISTERS_DMGEFF_DEKUNUT),
    /* Deku Stick     */ DMG_ENTRY(1, POE_SISTERS_DMGEFF_NONE),
    /* Horse trample  */ DMG_ENTRY(1, POE_SISTERS_DMGEFF_NONE),
    /* Explosives     */ DMG_ENTRY(1, POE_SISTERS_DMGEFF_NONE),
    /* Zora boomerang */ DMG_ENTRY(1, POE_SISTERS_DMGEFF_NONE),
    /* Normal arrow   */ DMG_ENTRY(1, POE_SISTERS_DMGEFF_NONE),
    /* UNK_DMG_0x06   */ DMG_ENTRY(0, POE_SISTERS_DMGEFF_NONE),
    /* Hookshot       */ DMG_ENTRY(1, POE_SISTERS_DMGEFF_NONE),
    /* Goron punch    */ DMG_ENTRY(1, POE_SISTERS_DMGEFF_NONE),
    /* Sword          */ DMG_ENTRY(1, POE_SISTERS_DMGEFF_NONE),
    /* Goron pound    */ DMG_ENTRY(0, POE_SISTERS_DMGEFF_NONE),
    /* Fire arrow     */ DMG_ENTRY(1, POE_SISTERS_DMGEFF_NONE),
    /* Ice arrow      */ DMG_ENTRY(1, POE_SISTERS_DMGEFF_NONE),
    /* Light arrow    */ DMG_ENTRY(2, POE_SISTERS_DMGEFF_LIGHTARROWS),
    /* Goron spikes   */ DMG_ENTRY(1, POE_SISTERS_DMGEFF_NONE),
    /* Deku spin      */ DMG_ENTRY(1, POE_SISTERS_DMGEFF_NONE),
    /* Deku bubble    */ DMG_ENTRY(1, POE_SISTERS_DMGEFF_NONE),
    /* Deku launch    */ DMG_ENTRY(2, POE_SISTERS_DMGEFF_NONE),
    /* UNK_DMG_0x12   */ DMG_ENTRY(0, POE_SISTERS_DMGEFF_UNKDMG12),
    /* Zora barrier   */ DMG_ENTRY(0, POE_SISTERS_DMGEFF_NONE),
    /* Normal shield  */ DMG_ENTRY(0, POE_SISTERS_DMGEFF_NONE),
    /* Light ray      */ DMG_ENTRY(0, POE_SISTERS_DMGEFF_NONE),
    /* Thrown object  */ DMG_ENTRY(1, POE_SISTERS_DMGEFF_NONE),
    /* Zora punch     */ DMG_ENTRY(1, POE_SISTERS_DMGEFF_NONE),
    /* Spin attack    */ DMG_ENTRY(1, POE_SISTERS_DMGEFF_SPINATTACK),
    /* Sword beam     */ DMG_ENTRY(0, POE_SISTERS_DMGEFF_NONE),
    /* Normal Roll    */ DMG_ENTRY(0, POE_SISTERS_DMGEFF_NONE),
    /* UNK_DMG_0x1B   */ DMG_ENTRY(0, POE_SISTERS_DMGEFF_NONE),
    /* UNK_DMG_0x1C   */ DMG_ENTRY(0, POE_SISTERS_DMGEFF_NONE),
    /* Unblockable    */ DMG_ENTRY(0, POE_SISTERS_DMGEFF_NONE),
    /* UNK_DMG_0x1E   */ DMG_ENTRY(0, POE_SISTERS_DMGEFF_NONE),
    /* Powder Keg     */ DMG_ENTRY(1, POE_SISTERS_DMGEFF_NONE),
};

RECOMP_PATCH void EnPoSisters_CheckCollision(EnPoSisters* this, PlayState* play) {
    Vec3f pos;

    int Difficulty = (int)recomp_get_config_double("diff_option");
    float visChance = 0.1f;

    switch (Difficulty) {
    case 0:
        visChance = 0.5f;
        break;

    case 1:
        visChance = 0.25f;
        break;

    default:
        break;
    }

    if (this->collider.base.acFlags & AC_HIT) {
        this->collider.base.acFlags &= ~AC_HIT;
        Actor_SetDropFlag(&this->actor, &this->collider.elem);

        if (this->megCloneId != POE_SISTERS_MEG_REAL) {
            ((EnPoSisters*)this->actor.parent)->megClonesRemaining--;
            Actor_PlaySfx(&this->actor, NA_SE_EN_PO_LAUGH2);
            EnPoSisters_MegCloneVanish(this, play);
            if (Rand_ZeroOne() < 0.2f) {
                pos.x = this->actor.world.pos.x;
                pos.y = this->actor.world.pos.y;
                pos.z = this->actor.world.pos.z;
                Item_DropCollectible(play, &pos, ITEM00_ARROWS_10);
            }
        }
        else if (this->collider.base.colMaterial != COL_MATERIAL_METAL) {
            if (this->actor.colChkInfo.damageEffect == POE_SISTERS_DMGEFF_DEKUNUT) {
                this->actor.world.rot.y = this->actor.shape.rot.y;
                this->poSisterFlags |= POE_SISTERS_FLAG_UPDATE_SHAPE_ROT;
                if ((play->gameplayFrames % 20 == 0) && (Rand_ZeroOne() < visChance)) EnPoSisters_SetupSpinBackToVisible(this, play);
                else if (this->actionFunc != EnPoSisters_SpinAttack) {
                    if (play->gameplayFrames % 5 == 0) Actor_PlaySfx(&this->actor, NA_SE_EN_PO_LAUGH2);
                    EnPoSisters_SetupAimlessIdleFlying(this);
                }
            }
            else if ((this->type == POE_SISTERS_TYPE_MEG) &&
                (this->actor.colChkInfo.damageEffect == POE_SISTERS_DMGEFF_SPINATTACK) &&
                (this->actionFunc == EnPoSisters_MegSurroundPlayer)) {
                if (this->megClonesRemaining == 0) {
                    // All Meg clones have been killed: Real Meg waits 45 frames then spin attacks
                    // Timer is negative because megClonesRemaining and megAttackTimer are the same union'd variable
                    this->megAttackTimer = -45;
                }
            }
            else {
                if (Actor_ApplyDamage(&this->actor)) {
                    Actor_PlaySfx(&this->actor, NA_SE_EN_PO_DAMAGE);
                }
                else {
                    Enemy_StartFinishingBlow(play, &this->actor);
                    Actor_PlaySfx(&this->actor, NA_SE_EN_PO_SISTER_DEAD);
                }

                if (this->actor.colChkInfo.damageEffect == POE_SISTERS_DMGEFF_LIGHTARROWS) {
                    this->drawDmgEffAlpha = 4.0f;
                    this->drawDmgEffScale = 0.5f;
                    Actor_Spawn(&play->actorCtx, play, ACTOR_EN_CLEAR_TAG, this->collider.elem.acDmgInfo.hitPos.x,
                        this->collider.elem.acDmgInfo.hitPos.y, this->collider.elem.acDmgInfo.hitPos.z, 0, 0, 0,
                        CLEAR_TAG_PARAMS(CLEAR_TAG_LARGE_LIGHT_RAYS));
                }
                EnPoSisters_SetupDamageFlinch(this);
            }
        }
    }
}

RECOMP_PATCH void EnPoSisters_MegSurroundPlayer(EnPoSisters* this, PlayState* play) {
    EnPoSisters* parent;
    int Difficulty = (int)recomp_get_config_double("diff_option");

    float spinMult = 1.0f;
    if (Difficulty == 0) {
        spinMult = 1.25f;
    }
    else if (Difficulty == 1) {
        spinMult = 1.75f;
    }

    DECR(this->megSurroundTimer);

    if ((this->megClonesRemaining > 0) && (this->megSurroundTimer >= 16)) {
        SkelAnime_Update(&this->skelAnime);
        if (this->megCloneId == POE_SISTERS_MEG_REAL) {
            if (ABS_ALT(16 - this->floatingBobbingTimer) < 14) {
                // Every N frames rotate around player. The fewer Meg clones remaining the faster they spin.
                this->actor.shape.rot.y += TRUNCF_BINANG(((0x580 - (this->megClonesRemaining * 0x180)) * spinMult) *
                    fabsf(Math_SinS(this->floatingBobbingTimer * 0x800)));
            }

            if (Difficulty != 1) {
                if ((this->megSurroundTimer >= 284) || (this->megSurroundTimer <= 30)) {
                    this->poSisterFlags |= POE_SISTERS_FLAG_REAL_MEG_ROTATION;
                }
                else {
                    this->poSisterFlags &= ~POE_SISTERS_FLAG_REAL_MEG_ROTATION;
                }
            }
        }
        else {
            this->actor.shape.rot.y = this->actor.parent->shape.rot.y + (this->megCloneId * 0x4000);
        }
    }

    // Twirl the real Meg backwards for a bit for a visual tell to player.
    if (this->megCloneId == POE_SISTERS_MEG_REAL) {
        if (Difficulty != 1) {
            if ((this->megSurroundTimer >= 284) || ((this->megSurroundTimer <= 30) && (this->megSurroundTimer >= 16))) {
                this->poSisterFlags |= POE_SISTERS_FLAG_REAL_MEG_ROTATION;
            }
            else {
                this->poSisterFlags &= ~POE_SISTERS_FLAG_REAL_MEG_ROTATION;
            }
        }
        else {
            this->poSisterFlags &= ~POE_SISTERS_FLAG_REAL_MEG_ROTATION;
        }
    }

    if (this->megSurroundTimer == 0) {
        if (this->megCloneId == POE_SISTERS_MEG_REAL) {
            EnPoSisters_SetupSpinAttack(this);
        }
        else {
            EnPoSisters_MegCloneVanish(this, play);
        }
    }
    else if (this->megCloneId != POE_SISTERS_MEG_REAL) {
        parent = (EnPoSisters*)this->actor.parent;
        if (parent->actionFunc == EnPoSisters_DamageFlinch) {
            // Clones flinch if you hit the real Meg
            EnPoSisters_SetupDamageFlinch(this);
        }
    } // It breaks badly when you have more than 0, so it stays at 0
    else if (this->megClonesRemaining <= 0 && this->megAttackTimer >= 0) {
        // All Meg clones have been killed: Real Meg waits 15 frames then spin attacks in retaliation
        // Timer is negative (inrecrements to zero)
        //  because megClonesRemaining and megAttackTimer are the same union'd variable
        this->megAttackTimer = -15;
    }
    else if (this->megAttackTimer < 0) {
        this->megAttackTimer++;
        if (this->megAttackTimer == 0) {
            EnPoSisters_SetupSpinAttack(this);
        }
    }

    EnPoSisters_MatchPlayerXZ(this, play);
    recomp_printf("Meg ATK: %d\n", this->megAttackTimer);
}

RECOMP_HOOK("EnPoSisters_Flee") void PoeFleeInvisChance(EnPoSisters* this, PlayState* play) {

    int Difficulty = (int)recomp_get_config_double("diff_option");
    float invisChance = 0.1f;

    switch (Difficulty) {
    case 0:
        invisChance = 0.1f;
        break;

    case 1:
        invisChance = 0.3f;
        break;

    default:
        break;
    }

    if (this->actor.bgCheckFlags & BGCHECKFLAG_WALL) {

    }
    else if ((this->fleeTimer == 0) && (this->actor.xzDistToPlayer > 480.0f)) {
        if (Rand_ZeroOne() < invisChance) {
            EnPoSisters_SpinToInvis(this,play);
        }
        else {
            if (Rand_ZeroOne() < invisChance / 2 && Difficulty == 1) {
                EnPoSisters_SetupSpinAttack(this);
            }
        }
    }

}

RECOMP_PATCH void EnPoSisters_SpinAttack(EnPoSisters* this, PlayState* play) {
    int Difficulty = (int)recomp_get_config_double("diff_option");
    Player* player = GET_PLAYER(play);
    float speedMult = 1.0f;

    switch (Difficulty) {
    case 0:
        speedMult = 1.0f;
        break;

    case 1:
        speedMult = 1.25f;
        break;

    default:
        break;
    }

    if (this->actor.speed <= 5.0f) {
        if (Difficulty == 1) {

            float dist = Math_Vec3f_DistXYZ(&this->actor.world.pos, &player->actor.world.pos);
            float predictionFrames = dist * 0.1f;

            Vec3f predictedPos;
            predictedPos.x = player->actor.world.pos.x + (player->actor.velocity.x * predictionFrames);
            predictedPos.y = player->actor.world.pos.y + (player->actor.velocity.y * predictionFrames);
            predictedPos.z = player->actor.world.pos.z + (player->actor.velocity.z * predictionFrames);

            this->actor.world.rot.y = Math_Vec3f_Yaw(&this->actor.world.pos, &predictedPos);
        }
        else {
            this->actor.world.rot.y = this->actor.yawTowardsPlayer;
        }
    }

    this->actor.speed = 5.0f * speedMult;

    SkelAnime_Update(&this->skelAnime);
    if (Animation_OnFrame(&this->skelAnime, 0.0f)) {
        DECR(this->spinTimer);
    }

    this->actor.shape.rot.y += (s32)(1152.0f * this->skelAnime.endFrame);

    if (this->spinTimer == 0) {
        s16 rotY = this->actor.shape.rot.y - this->actor.world.rot.y;

        if (ABS_ALT(rotY) < 0x1000) {
            if (this->type != POE_SISTERS_TYPE_MEG) {
                this->collider.base.colMaterial = COL_MATERIAL_HIT3;
                this->collider.base.acFlags &= ~AC_HARD;
                EnPoSisters_SetupAimlessIdleFlying(this);
            }
            else {
                Actor_PlaySfx(&this->actor, NA_SE_EN_PO_LAUGH2);
                EnPoSisters_MegCloneVanish(this, play);
            }
        }
    }

    if (Animation_OnFrame(&this->skelAnime, 1.0f)) {
        Actor_PlaySfx(&this->actor, NA_SE_EN_PO_ROLL);
    }
}

RECOMP_HOOK("EnPoSisters_Update") void NeoDefense(Actor* thisx, PlayState* play) {
	EnPoSisters* this = (EnPoSisters*)thisx;

    int Difficulty = (int)recomp_get_config_double("diff_option");
    if (this->type != POE_SISTERS_TYPE_MEG) {
        switch (Difficulty) {
        case 0:
            break;

        case 1:
            this->actor.colChkInfo.damage = (this->actor.colChkInfo.damage + 1) / 2;
            break;

        default:
            break;
        }
    }
}