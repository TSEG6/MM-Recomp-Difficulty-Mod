#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_boss_hakugin.h"
#include "z64quake.h"
#include "z64rumble.h"
#include "overlays/actors/ovl_En_Bom/z_en_bom.h"
#include "overlays/actors/ovl_Door_Warp1/z_door_warp1.h"
#include "overlays/actors/ovl_En_Clear_Tag/z_en_clear_tag.h"
#include "overlays/actors/ovl_En_Hakurock/z_en_hakurock.h"
#include "overlays/actors/ovl_Item_B_Heart/z_item_b_heart.h"
#include "overlays/effects/ovl_Effect_Ss_Hitmark/z_eff_ss_hitmark.h"

#include "assets/objects/gameplay_keep/gameplay_keep.h"

#define GOHT_MAX_HEALTH 30

void BossHakugin_Run(BossHakugin* this, PlayState* play);
void BossHakugin_Charge(BossHakugin* this, PlayState* play);
void BossHakugin_Downed(BossHakugin* this, PlayState* play);
void BossHakugin_SetupWait(BossHakugin* this, PlayState* play);
void BossHakugin_Wait(BossHakugin* this, PlayState* play);
void BossHakugin_ShootLightning(BossHakugin* this, PlayState* play);
void BossHakugin_SetupCutsceneStart(BossHakugin* this);

typedef enum GohtElectricBallState {
    // The electric ball is not active. Goht will start charging the electric ball the next time the player hits it with
    // Goron Spikes while it's running.
    /* 0 */ GOHT_ELECTRIC_BALL_STATE_NONE,

    // Goht is charging the electric ball up and will shoot it forward after a short delay.
    /* 1 */ GOHT_ELECTRIC_BALL_STATE_CHARGE,

    // The electric ball is flying forward from Goht's head and slowing down until its speed reaches 50.0f.
    /* 2 */ GOHT_ELECTRIC_BALL_STATE_FLY_FORWARD,

    // The electric ball is rotating to track the player and gradually moves towards them.
    /* 3 */ GOHT_ELECTRIC_BALL_STATE_FLY_TOWARDS_PLAYER,

    // The electric ball hit the player or the environment, so it's fading away. It can't hurt the player in this state.
    /* 4 */ GOHT_ELECTRIC_BALL_STATE_FADE_OUT
} GohtElectricBallState;

typedef enum GohtDamageEffect {
    // If an attack with this damage effect hits Goht while it's standing upright or running, it will deal no damage and
    // spawn some spark effects. If it hits Goht while it's downed, however, it will deal damage with no special effect.
    /* 0x0 */ GOHT_DMGEFF_DOWNED_ONLY,

    // Deals damage and surrounds Goht with fire.
    /* 0x2 */ GOHT_DMGEFF_FIRE = 2,

    // Deals damage and surrounds Goht with ice that instantly shatters.
    /* 0x3 */ GOHT_DMGEFF_FREEZE,

    // Deals damage and surrounds Goht with yellow light orbs.
    /* 0x4 */ GOHT_DMGEFF_LIGHT_ORB,

    // Deals damage and can additionally knock Goht down if it hits Goht on the head, thorax, pelvis, or upper legs
    // while it isn't charging.
    /* 0xC */ GOHT_DMGEFF_EXPLOSIVE = 0xC,

    // Deals damage and surrounds Goht with blue light orbs.
    /* 0xD */ GOHT_DMGEFF_BLUE_LIGHT_ORB,

    // Deals damage with no special effect.
    /* 0xE */ GOHT_DMGEFF_NONE,

    // Deals damage and can additionally knock Goht down in the same manner as GOHT_DMGEFF_EXPLOSIVE. If Goht is hit by
    // an attack with this damage effect and it does *not* knock it down, however, then Goht will charge up and fire an
    // electric ball attack.
    /* 0xF */ GOHT_DMGEFF_GORON_SPIKES
} GohtDamageEffect;

static DamageTable sDamageTable = {
    /* Deku Nut       */ DMG_ENTRY(0, GOHT_DMGEFF_DOWNED_ONLY),
    /* Deku Stick     */ DMG_ENTRY(1, GOHT_DMGEFF_DOWNED_ONLY),
    /* Horse trample  */ DMG_ENTRY(1, GOHT_DMGEFF_DOWNED_ONLY),
    /* Explosives     */ DMG_ENTRY(1, GOHT_DMGEFF_EXPLOSIVE),
    /* Zora boomerang */ DMG_ENTRY(1, GOHT_DMGEFF_DOWNED_ONLY),
    /* Normal arrow   */ DMG_ENTRY(1, GOHT_DMGEFF_DOWNED_ONLY),
    /* UNK_DMG_0x06   */ DMG_ENTRY(0, GOHT_DMGEFF_DOWNED_ONLY),
    /* Hookshot       */ DMG_ENTRY(1, GOHT_DMGEFF_DOWNED_ONLY),
    /* Goron punch    */ DMG_ENTRY(1, GOHT_DMGEFF_DOWNED_ONLY),
    /* Sword          */ DMG_ENTRY(1, GOHT_DMGEFF_DOWNED_ONLY),
    /* Goron pound    */ DMG_ENTRY(1, GOHT_DMGEFF_DOWNED_ONLY),
    /* Fire arrow     */ DMG_ENTRY(1, GOHT_DMGEFF_DOWNED_ONLY),
    /* Ice arrow      */ DMG_ENTRY(1, GOHT_DMGEFF_DOWNED_ONLY),
    /* Light arrow    */ DMG_ENTRY(1, GOHT_DMGEFF_LIGHT_ORB),
    /* Goron spikes   */ DMG_ENTRY(1, GOHT_DMGEFF_GORON_SPIKES),
    /* Deku spin      */ DMG_ENTRY(1, GOHT_DMGEFF_DOWNED_ONLY),
    /* Deku bubble    */ DMG_ENTRY(1, GOHT_DMGEFF_DOWNED_ONLY),
    /* Deku launch    */ DMG_ENTRY(1, GOHT_DMGEFF_DOWNED_ONLY),
    /* UNK_DMG_0x12   */ DMG_ENTRY(0, GOHT_DMGEFF_DOWNED_ONLY),
    /* Zora barrier   */ DMG_ENTRY(0, GOHT_DMGEFF_DOWNED_ONLY),
    /* Normal shield  */ DMG_ENTRY(0, GOHT_DMGEFF_DOWNED_ONLY),
    /* Light ray      */ DMG_ENTRY(0, GOHT_DMGEFF_DOWNED_ONLY),
    /* Thrown object  */ DMG_ENTRY(1, GOHT_DMGEFF_DOWNED_ONLY),
    /* Zora punch     */ DMG_ENTRY(1, GOHT_DMGEFF_DOWNED_ONLY),
    /* Spin attack    */ DMG_ENTRY(1, GOHT_DMGEFF_DOWNED_ONLY),
    /* Sword beam     */ DMG_ENTRY(1, GOHT_DMGEFF_BLUE_LIGHT_ORB),
    /* Normal Roll    */ DMG_ENTRY(0, GOHT_DMGEFF_DOWNED_ONLY),
    /* UNK_DMG_0x1B   */ DMG_ENTRY(0, GOHT_DMGEFF_DOWNED_ONLY),
    /* UNK_DMG_0x1C   */ DMG_ENTRY(0, GOHT_DMGEFF_DOWNED_ONLY),
    /* Unblockable    */ DMG_ENTRY(0, GOHT_DMGEFF_DOWNED_ONLY),
    /* UNK_DMG_0x1E   */ DMG_ENTRY(0, GOHT_DMGEFF_DOWNED_ONLY),
    /* Powder Keg     */ DMG_ENTRY(1, GOHT_DMGEFF_EXPLOSIVE),
};

void BossHakugin_Thaw(BossHakugin* this, PlayState* play) {
    if (this->drawDmgEffType == ACTOR_DRAW_DMGEFF_FROZEN_NO_SFX) {
        this->drawDmgEffType = ACTOR_DRAW_DMGEFF_FIRE;
        this->drawDmgEffAlpha = 0.0f;
        Actor_SpawnIceEffects(play, &this->actor, this->bodyPartsPos, GOHT_BODYPART_MAX, 3, 0.7f, 0.5f);
    }
}

void BossHakugin_SetupDowned(BossHakugin* this) {
    Animation_MorphToPlayOnce(&this->skelAnime, &gGohtFallDownAnim, -3.0f);
    this->bodyCollider.base.atFlags &= ~AT_ON;
    this->bodyCollider.base.acFlags &= ~AC_HARD;
    this->frontHalfRotZ = 0;
    this->finishedFallingDown = false;
    this->chargingLightOrbScale = 0.0f;

    if (this->electricBallState == GOHT_ELECTRIC_BALL_STATE_CHARGE) {
        Math_Vec3f_Copy(&this->electricBallPos[0], &this->chargingLightningPos);
        this->electricBallSpeed = this->actor.speed + 100.0f;
        this->electricBallRot.x = Math_CosS(0xA00) * Math_SinS(this->actor.shape.rot.y);
        this->electricBallRot.y = Math_SinS(0xA00);
        this->electricBallRot.z = Math_CosS(0xA00) * Math_CosS(this->actor.shape.rot.y);
        this->electricBallState = GOHT_ELECTRIC_BALL_STATE_FLY_FORWARD;
        this->chargingLightOrbScale = 0.0f;
        Audio_PlaySfx_AtPos(&this->sfxPos, NA_SE_EN_COMMON_E_BALL_THR);
        this->electricBallCollider.base.atFlags |= AT_ON;
    }

    this->timer = 60;
    this->actor.speed = 20.0f;
    this->actionFunc = BossHakugin_Downed;
}

void BossHakugin_UpdateDrawDmgEffect(BossHakugin* this, PlayState* play, s32 colliderIndex) {
    if (this->actor.colChkInfo.damageEffect == GOHT_DMGEFF_FIRE) {
        this->drawDmgEffType = ACTOR_DRAW_DMGEFF_FIRE;
        this->drawDmgEffAlpha = 3.0f;
        this->drawDmgEffScale = 2.5f;
    }
    else if (this->actor.colChkInfo.damageEffect == GOHT_DMGEFF_LIGHT_ORB) {
        this->drawDmgEffType = ACTOR_DRAW_DMGEFF_LIGHT_ORBS;
        this->drawDmgEffAlpha = 3.0f;
        this->drawDmgEffScale = 2.5f;
        Actor_Spawn(&play->actorCtx, play, ACTOR_EN_CLEAR_TAG,
            this->bodyCollider.elements[colliderIndex].base.acDmgInfo.hitPos.x,
            this->bodyCollider.elements[colliderIndex].base.acDmgInfo.hitPos.y,
            this->bodyCollider.elements[colliderIndex].base.acDmgInfo.hitPos.z, 0, 0, 0,
            CLEAR_TAG_PARAMS(CLEAR_TAG_LARGE_LIGHT_RAYS));
    }
    else if (this->actor.colChkInfo.damageEffect == GOHT_DMGEFF_FREEZE) {
        this->drawDmgEffType = ACTOR_DRAW_DMGEFF_FROZEN_NO_SFX;
        this->drawDmgEffScale = 2.5f;
        this->drawDmgEffFrozenSteamScale = 3.75f;
        this->drawDmgEffAlpha = 1.0f;
    }
    else if (this->actor.colChkInfo.damageEffect == GOHT_DMGEFF_BLUE_LIGHT_ORB) {
        this->drawDmgEffType = ACTOR_DRAW_DMGEFF_BLUE_LIGHT_ORBS;
        this->drawDmgEffScale = 2.5f;
        this->drawDmgEffAlpha = 3.0f;
        Actor_Spawn(&play->actorCtx, play, ACTOR_EN_CLEAR_TAG,
            this->bodyCollider.elements[colliderIndex].base.acDmgInfo.hitPos.x,
            this->bodyCollider.elements[colliderIndex].base.acDmgInfo.hitPos.y,
            this->bodyCollider.elements[colliderIndex].base.acDmgInfo.hitPos.z, 0, 0, 3,
            CLEAR_TAG_PARAMS(CLEAR_TAG_LARGE_LIGHT_RAYS));
    }
}

s32 BossHakugin_ShouldWait(BossHakugin* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    s16 yawDiff;
    s32 absYawDiff;
    f32 posZ;
    f32 posX;

    if (this->transformedPlayerPos.z > 0.0f) {
        return false;
    }

    yawDiff = this->baseRotY - Camera_GetCamDirYaw(GET_ACTIVE_CAM(play));
    absYawDiff = ABS_ALT(yawDiff);

    if (absYawDiff < 0x4000) {
        if (absYawDiff < 0x1800) {
            this->lightningHitSomething = false;
        }
        return false;
    }

    // If Goht hit the player with lightning the last time it waited, then the only way for `lightningHitSomething` to
    // be false (in other words, the only way for us continue in this function) is because it was set to false by the
    // code above in a previous call to this function; this only happens if the difference between Goht's base yaw and
    // the active cam's yaw is small enough.
    if (this->lightningHitSomething == true) {
        return false;
    }

    posX = this->actor.world.pos.x * this->direction;
    posZ = this->actor.world.pos.z * this->direction;

    /**
     * Goht's arena is a square with rounded corners. If the player is standing in one of the "sides" of the square, and
     * Goht is standing near one of the opposite corners (which specific corner depends on whether Goht is running
     * clockwise or counterclockwise), then Goht will wait. The diagram below illustrates this; if the player is
     * standing in the box labeled P, and Goht is standing in the box G1 (if Goht is running counterclockwise) or G2 (if
     * Goht is running clockwise), then Goht will wait:
     *    _____________________
     *   /    |           |    \
     *  /     |     P     |     \
     * |      |___________|      |
     * |      |           |      |
     * |______|           |______|
     * |      |           |      |
     * |  G1  |           |  G2  |
     * |      |‾‾‾‾‾‾‾‾‾‾‾|      |
     *  \     |           |     /
     *   \____|___________|____/
     */
    if (((player->actor.world.pos.x > 1200.0f) && (player->actor.world.pos.z < 1200.0f) &&
        (player->actor.world.pos.z > -1200.0f) && (this->actor.world.pos.x < 0.0f) && (posZ > 1200.0f)) ||
        ((player->actor.world.pos.x < -1200.0f) && (player->actor.world.pos.z < 1200.0f) &&
            (player->actor.world.pos.z > -1200.0f) && (this->actor.world.pos.x > 0.0f) && (posZ < -1200.0f)) ||
        ((player->actor.world.pos.z > 1200.0f) && (player->actor.world.pos.x < 1200.0f) &&
            (player->actor.world.pos.x > -1200.0f) && (this->actor.world.pos.z < 0.0f) && (posX < -1200.0f)) ||
        ((player->actor.world.pos.z < -1200.0f) && (player->actor.world.pos.x < 1200.0f) &&
            (player->actor.world.pos.x > -1200.0f) && (this->actor.world.pos.z > 0.0f) && (posX > 1200.0f))) {
        return true;
    }

    return false;
}

static ColliderTrisElementInit sTrisElementsInit[1] = {
    {
        {
            ELEM_MATERIAL_UNK5,
            { 0x20000000, 0x03, 0x08 },
            { 0x00000000, 0x00, 0x00 },
            ATELEM_ON | ATELEM_SFX_NONE,
            ACELEM_NONE,
            OCELEM_NONE,
        },
        { { { 22.0f, 0.0f, 100.0f }, { 0.0f, 0.0f, -100.0f }, { -22.0f, 0.0f, 100.0f } } },
    },
};

static ColliderTrisInit sTrisInit = {
    {
        COL_MATERIAL_NONE,
        AT_ON | AT_TYPE_ENEMY,
        AC_NONE,
        OC1_NONE,
        OC2_TYPE_1,
        COLSHAPE_TRIS,
    },
    ARRAY_COUNT(sTrisElementsInit),
    sTrisElementsInit,
};

void BossHakugin_SpawnGravel(BossHakugin* this, Vec3f* pos) {
    s32 i;
    GohtRockEffect* rockEffect;

    for (i = 0; i < GOHT_ROCK_EFFECT_COUNT; i++) {
        rockEffect = &this->rockEffects[i];
        if (rockEffect->timer < 0) {
            VecGeo velocityGeo;

            Math_Vec3f_Copy(&rockEffect->pos, pos);
            velocityGeo.pitch = Rand_S16Offset(0x1000, 0x3000);
            velocityGeo.yaw = this->actor.shape.rot.y + ((s32)Rand_Next() >> 0x12) + 0x8000;
            velocityGeo.r = Rand_ZeroFloat(5.0f) + 7.0f;
            rockEffect->velocity.x = velocityGeo.r * Math_CosS(velocityGeo.pitch) * Math_SinS(velocityGeo.yaw);
            rockEffect->velocity.y = velocityGeo.r * Math_SinS(velocityGeo.pitch);
            rockEffect->velocity.z = velocityGeo.r * Math_CosS(velocityGeo.pitch) * Math_CosS(velocityGeo.yaw);
            rockEffect->pos.x = pos->x + (Rand_ZeroFloat(3.0f) * rockEffect->velocity.x);
            rockEffect->pos.y = pos->y + (Rand_ZeroFloat(3.0f) * rockEffect->velocity.y);
            rockEffect->pos.z = pos->z + (Rand_ZeroFloat(3.0f) * rockEffect->velocity.z);
            rockEffect->scale = (Rand_ZeroFloat(6.0f) + 15.0f) * 0.0001f;
            rockEffect->timer = 40;
            rockEffect->type = GOHT_ROCK_EFFECT_TYPE_BOULDER;
            break;
        }
    }
}

void BossHakugin_SetLightningSegmentColliderVertices(GohtLightningSegment* lightningSegment) {
    s32 i;
    Vec3f vertices[3];

    Matrix_SetTranslateRotateYXZ(lightningSegment->pos.x, lightningSegment->pos.y, lightningSegment->pos.z,
        &lightningSegment->rot);
    Matrix_Scale(1.0f, 1.0f, 1.0f, MTXMODE_APPLY);

    for (i = 0; i < ARRAY_COUNT(vertices); i++) {
        Matrix_MultVec3f(&sTrisElementsInit[0].dim.vtx[i], &vertices[i]);
    }

    Collider_SetTrisVertices(&lightningSegment->collider, 0, &vertices[0], &vertices[1], &vertices[2]);
}

RECOMP_HOOK("BossHakugin_Run") void RunningStuff(BossHakugin* this, PlayState* play) {
    int Difficulty = (int)recomp_get_config_double("diff_option");

    if (BossHakugin_ShouldWait(this, play)) {
        BossHakugin_SetupWait(this, play);
    }
    else {
        // The `damagedSpeedUpCounter` increases by 35 every time the player hits Goht (when it's not downed) and
        // decreases by 1 every frame. Thus, if the player repeatedly hits Goht in a short amount of time, this will
        // exceed 55; in this circumstance, Goht will run twice as fast as it normally does to get away from the player.
        if (this->damagedSpeedUpCounter > 55) {
            this->damagedSpeedUpCounter = 0;
            this->timer = 40;
            this->targetSpeed = (Difficulty == 1) ? 30.0f : 28.5f;
        }

        if ((this->timer == 40) &&
            (Math_SmoothStepToF(&this->actor.speed, this->targetSpeed, 0.15f, 1.0f, 0.1f) < 0.05f)) {
            this->timer--;
        }
        else if (this->timer != 40) {
            this->timer--;
            if (this->timer == 0) {
                if (this->actor.xzDistToPlayer > 1500.0f) {
                    this->targetSpeed = (Difficulty == 1) ? 15.5f : 13.5f;
                }
                else if (this->transformedPlayerPos.z > 0.0f) {
                    this->targetSpeed = (Difficulty == 1) ? 23.5f : 22.0f;
                }
                else {
                    this->targetSpeed = (Difficulty == 1) ? 17.5f : 16.0f;
                }

                this->targetSpeed += (30 - this->actor.colChkInfo.health) * (1.0f / 30.0f);
                this->timer = 40;
            }
        }

        float boulderChance = (Difficulty == 1) ? 0.05f : 0.01f;
        float stalactiteChance = (Difficulty == 1) ? 0.05f : 0.01f;
        float bombChance = (Difficulty == 1) ? 0.02f : 0.01f;

        Vec3f footPos;
        footPos.x = this->bodyCollider.elements[GOHT_COLLIDER_BODYPART_PELVIS].dim.worldSphere.center.x;
        footPos.y = this->actor.world.pos.y;
        footPos.z = this->bodyCollider.elements[GOHT_COLLIDER_BODYPART_PELVIS].dim.worldSphere.center.z;

        if (Rand_ZeroOne() < boulderChance) {
            Actor* boulder;
            s32 i;
            for (i = 0; i < ARRAY_COUNT(this->boulders); i++) {
                boulder = this->boulders[i];
                if (EN_HAKUROCK_GET_TYPE(boulder) == EN_HAKUROCK_TYPE_NONE) {
                    Math_Vec3f_Copy(&boulder->world.pos, &footPos);
                    boulder->params = EN_HAKUROCK_TYPE_BOULDER;
                    break;
                }
            }
        }

        if (Rand_ZeroOne() < stalactiteChance) {
            Actor* stalactite;
            s32 i;
            for (i = 0; i < ARRAY_COUNT(this->stalactites); i++) {
                stalactite = this->stalactites[i];
                if (EN_HAKUROCK_GET_TYPE(stalactite) == EN_HAKUROCK_TYPE_NONE) {
                    stalactite->params = EN_HAKUROCK_TYPE_FALLING_STALACTITE;
                    break;
                }
            }
        }

        if (Rand_ZeroOne() < bombChance) {
            EnBom* bomb;
            s16 yawDiff;

            float spawnX = this->bodyCollider.elements[GOHT_COLLIDER_BODYPART_THORAX].dim.worldSphere.center.x - (100.0f * Math_SinS(this->actor.shape.rot.y));
            float spawnY = this->bodyCollider.elements[GOHT_COLLIDER_BODYPART_THORAX].dim.worldSphere.center.y + 100.0f;
            float spawnZ = this->bodyCollider.elements[GOHT_COLLIDER_BODYPART_THORAX].dim.worldSphere.center.z - (Math_CosS(this->actor.shape.rot.y) * 100.0f);

            bomb = (EnBom*)Actor_Spawn(
                &play->actorCtx, play, ACTOR_EN_BOM,
                spawnX, spawnY, spawnZ,
                BOMB_EXPLOSIVE_TYPE_BOMB, 0, 0, BOMB_TYPE_BODY
            );

            if (bomb != NULL) {
                yawDiff = (this->actor.yawTowardsPlayer - this->actor.shape.rot.y) - 0x8000; 
                if (yawDiff > 0x2000) { 
                    bomb->actor.world.rot.y = this->actor.shape.rot.y + 0xA000; 
                }
                else if (yawDiff < -0x2000) { 
                    bomb->actor.world.rot.y = this->actor.shape.rot.y + 0x6000; 
                }
                else { 
                    bomb->actor.world.rot.y = this->actor.yawTowardsPlayer; 
                }

                bomb->timer = (Rand_Next() >> 0x1C) + 17; 
                bomb->actor.velocity.y = 2.0f; 
                bomb->actor.speed = this->actor.xzDistToPlayer * 0.01f;
                bomb->actor.speed = CLAMP(bomb->actor.speed, 6.0f, 12.0f);

                bomb->actor.gravity = -2.0f;

                Actor_SetScale(&bomb->actor, 0.02f);
            }
        }
    }
}

RECOMP_HOOK("BossHakugin_Wait") void ReduceWait(BossHakugin* this, PlayState* play) {

    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        this->timer--;
        this->skelAnime.playSpeed = 1.33f;
        break;

    case 1:
        this->timer = 0;
        this->skelAnime.playSpeed = 2.0f;
        break;

    default:
        break;
    }

}

RECOMP_HOOK("BossHakugin_SetupShootLightning") void buffedlight(BossHakugin* this) {

    int Difficulty = (int)recomp_get_config_double("diff_option");

    if (Difficulty == 1) this->chargeUpTimer = 1;

}

RECOMP_PATCH s32 BossHakugin_UpdateDamage(BossHakugin* this, PlayState* play) {
    if (this->bodyCollider.base.acFlags & AC_HIT) {
        s32 i;

        for (i = 0; i < GOHT_COLLIDER_BODYPART_MAX; i++) {
            if (this->bodyCollider.elements[i].base.acElemFlags & ACELEM_HIT) {
                break;
            }
        }

        if (i == GOHT_COLLIDER_BODYPART_MAX) {
            return false;
        }

        if (this->bodyCollider.elements[i].base.acHitElem->atDmgInfo.dmgFlags & (DMG_NORMAL_ARROW | DMG_ICE_ARROW | DMG_LIGHT_ARROW)) {
            return false;
        }

        if ((this->drawDmgEffType == ACTOR_DRAW_DMGEFF_FROZEN_NO_SFX) &&
            (this->bodyCollider.elements[i].base.acHitElem->atDmgInfo.dmgFlags & 0x000DB0B3)) {
            return false;
        }

        BossHakugin_Thaw(this, play);

        if (this->actionFunc == BossHakugin_Downed) {
            Actor_SetColorFilter(&this->actor, COLORFILTER_COLORFLAG_RED, 255, COLORFILTER_BUFFLAG_OPA, 15);
            BossHakugin_UpdateDrawDmgEffect(this, play, i);

            if (Actor_ApplyDamage(&this->actor) == 0) {
                Enemy_StartFinishingBlow(play, &this->actor);
                Actor_PlaySfx(&this->actor, NA_SE_EN_ICEB_DEAD_OLD);
                BossHakugin_SetupCutsceneStart(this);
            }
            else {
                Actor_PlaySfx(&this->actor, NA_SE_EN_ICEB_DAMAGE_OLD);
            }

            this->disableBodyCollidersTimer = 15;
            return true;
        }

        if (((this->actor.colChkInfo.damageEffect == GOHT_DMGEFF_GORON_SPIKES) ||
            (this->actor.colChkInfo.damageEffect == GOHT_DMGEFF_NONE) ||
            (this->actor.colChkInfo.damageEffect == GOHT_DMGEFF_FIRE) ||
            (this->actor.colChkInfo.damageEffect == GOHT_DMGEFF_FREEZE) ||
            (this->actor.colChkInfo.damageEffect == GOHT_DMGEFF_LIGHT_ORB) ||
            (this->actor.colChkInfo.damageEffect == GOHT_DMGEFF_BLUE_LIGHT_ORB) ||
            (this->actor.colChkInfo.damageEffect == GOHT_DMGEFF_EXPLOSIVE)) &&
            ((this->actionFunc == BossHakugin_Run) || (this->actionFunc == BossHakugin_ShootLightning) ||
                (this->actionFunc == BossHakugin_Wait) || (this->actionFunc == BossHakugin_Charge))) {
            Player* player = GET_PLAYER(play);

            if (this->actor.colChkInfo.damageEffect == GOHT_DMGEFF_GORON_SPIKES) {
                player->pushedSpeed = 15.0f;
                if ((s16)(this->actor.yawTowardsPlayer - this->actor.shape.rot.y) > 0) {
                    player->pushedYaw = this->actor.shape.rot.y + 0x4000;
                }
                else {
                    player->pushedYaw = this->actor.shape.rot.y - 0x4000;
                }
            }

            this->disableBodyCollidersTimer = 15;
            Actor_SetColorFilter(&this->actor, COLORFILTER_COLORFLAG_RED, 255, COLORFILTER_BUFFLAG_OPA, 15);
            this->damagedSpeedUpCounter += 35;
            BossHakugin_UpdateDrawDmgEffect(this, play, i);
            this->actor.colChkInfo.damage = this->bodyCollider.elements[i].base.acHitElem->atDmgInfo.damage;

            if (Actor_ApplyDamage(&this->actor) == 0) {
                Enemy_StartFinishingBlow(play, &this->actor);
                Actor_PlaySfx(&this->actor, NA_SE_EN_ICEB_DEAD_OLD);
                BossHakugin_SetupCutsceneStart(this);
            }
            else {
                if ((this->actor.colChkInfo.damageEffect == GOHT_DMGEFF_EXPLOSIVE) ||
                    ((this->actor.colChkInfo.damageEffect == GOHT_DMGEFF_GORON_SPIKES) &&
                        (this->actionFunc != BossHakugin_Charge) &&
                        ((this->bodyCollider.elements[GOHT_COLLIDER_BODYPART_HEAD].base.acElemFlags & ACELEM_HIT) ||
                            (this->bodyCollider.elements[GOHT_COLLIDER_BODYPART_THORAX].base.acElemFlags & ACELEM_HIT) ||
                            (this->bodyCollider.elements[GOHT_COLLIDER_BODYPART_PELVIS].base.acElemFlags & ACELEM_HIT) ||
                            (this->bodyCollider.elements[GOHT_COLLIDER_BODYPART_FRONT_RIGHT_UPPER_LEG].base.acElemFlags &
                                ACELEM_HIT) ||
                            (this->bodyCollider.elements[GOHT_COLLIDER_BODYPART_FRONT_LEFT_UPPER_LEG].base.acElemFlags &
                                ACELEM_HIT) ||
                            (this->bodyCollider.elements[GOHT_COLLIDER_BODYPART_BACK_RIGHT_THIGH].base.acElemFlags &
                                ACELEM_HIT) ||
                            (this->bodyCollider.elements[GOHT_COLLIDER_BODYPART_BACK_LEFT_THIGH].base.acElemFlags &
                                ACELEM_HIT) ||
                            (this->bodyCollider.elements[GOHT_COLLIDER_BODYPART_RIGHT_HORN].base.acElemFlags & ACELEM_HIT) ||
                            (this->bodyCollider.elements[GOHT_COLLIDER_BODYPART_LEFT_HORN].base.acElemFlags & ACELEM_HIT)))) {
                    BossHakugin_SetupDowned(this);
                }
                else if ((this->electricBallState == GOHT_ELECTRIC_BALL_STATE_NONE) &&
                    (this->electricBallCount == 0) && (this->actionFunc == BossHakugin_Run) &&
                    (this->actor.colChkInfo.damageEffect == GOHT_DMGEFF_GORON_SPIKES)) {
                    this->chargeUpTimer = 5;
                    this->electricBallState = GOHT_ELECTRIC_BALL_STATE_CHARGE;
                }

                Actor_PlaySfx(&this->actor, NA_SE_EN_ICEB_DAMAGE_OLD);
            }

            return true;
        }
        else {
            // This block is for attacks with the effect of `GOHT_DMGEFF_DOWNED_ONLY` hitting Goht while it's standing
            // upright. These attacks deal no damage and instead just spawn sparks and play a metal sound.
            s32 j;

            this->disableBodyCollidersTimer = 20;
            for (j = 0; j < ARRAY_COUNT(this->bodyColliderElements); j++) {
                Vec3f hitPos;
                ColliderElement* elem = &this->bodyCollider.elements[j].base;

                if ((elem->acElemFlags & ACELEM_HIT) && (elem->acHitElem != NULL) &&
                    !(elem->acHitElem->atElemFlags & ATELEM_SFX_NONE)) {
                    Math_Vec3s_ToVec3f(&hitPos, &elem->acDmgInfo.hitPos);
                    EffectSsHitmark_SpawnFixedScale(play, EFFECT_HITMARK_METAL, &hitPos);
                    CollisionCheck_SpawnShieldParticlesMetalSound(play, &hitPos, &this->actor.projectedPos);
                    break;
                }
            }
        }
    }

    return false;
}

RECOMP_HOOK("BossHakugin_Update") void DmgRedGoat(Actor* thisx, PlayState* play) {

    BossHakugin* this = (BossHakugin*)thisx;

    int Difficulty = (int)recomp_get_config_double("diff_option");
    static s32 callCounter = 0;

    switch (Difficulty) {
    case 0:
        if (this->actor.colChkInfo.damage > 0) {
            int reducedDamage = this->actor.colChkInfo.damage / 2;
            this->actor.colChkInfo.damage = (reducedDamage > 1) ? reducedDamage : 1;
        }
        break;

    case 1:
        if (this->actor.colChkInfo.damage > 0) {
            int reducedDamage = (this->actor.colChkInfo.damage + 2) / 3;
            this->actor.colChkInfo.damage = (reducedDamage > 1) ? reducedDamage : 1;
        }
        break;

    default:
        break;
    }

    this->skelAnime.playSpeed = 1.0f;

    if (this->actor.colChkInfo.health > 0) {
        callCounter++;

        if (callCounter >= 400) {
            this->actor.colChkInfo.health++;

            if (this->actor.colChkInfo.health > GOHT_MAX_HEALTH) {
                this->actor.colChkInfo.health = GOHT_MAX_HEALTH;
            }

            callCounter = 0;
        }
    }
}