#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_en_snowman.h"
#include "overlays/actors/ovl_En_Clear_Tag/z_en_clear_tag.h"

void EnSnowman_SetupMoveSnowPile(EnSnowman* this);
void EnSnowman_SetupEmerge(EnSnowman* this, PlayState* play);
void EnSnowman_SetupSplitDoNothing(EnSnowman* this);
void EnSnowman_UpdateSnowball(Actor* thisx, PlayState* play);
void EnSnowman_DrawSnowball(Actor* thisx, PlayState* play);

typedef enum {
    /* 0x0 */ EN_SNOWMAN_DMGEFF_NONE,
    /* 0x1 */ EN_SNOWMAN_DMGEFF_STUN,
    /* 0x2 */ EN_SNOWMAN_DMGEFF_MELT,
    /* 0x4 */ EN_SNOWMAN_DMGEFF_LIGHT_ORB = 0x4,
    /* 0x5 */ EN_SNOWMAN_DMGEFF_ELECTRIC_STUN,
    /* 0xF */ EN_SNOWMAN_DMGEFF_HOOKSHOT = 0xF // Damages small Eenos, pulls the player towards large Eenos
} EnTalkGibudDamageEffect;

static DamageTable sDamageTable = {
    /* Deku Nut       */ DMG_ENTRY(0, EN_SNOWMAN_DMGEFF_STUN),
    /* Deku Stick     */ DMG_ENTRY(1, EN_SNOWMAN_DMGEFF_NONE),
    /* Horse trample  */ DMG_ENTRY(1, EN_SNOWMAN_DMGEFF_NONE),
    /* Explosives     */ DMG_ENTRY(1, EN_SNOWMAN_DMGEFF_NONE),
    /* Zora boomerang */ DMG_ENTRY(1, EN_SNOWMAN_DMGEFF_NONE),
    /* Normal arrow   */ DMG_ENTRY(1, EN_SNOWMAN_DMGEFF_NONE),
    /* UNK_DMG_0x06   */ DMG_ENTRY(0, EN_SNOWMAN_DMGEFF_NONE),
    /* Hookshot       */ DMG_ENTRY(1, EN_SNOWMAN_DMGEFF_HOOKSHOT),
    /* Goron punch    */ DMG_ENTRY(2, EN_SNOWMAN_DMGEFF_NONE),
    /* Sword          */ DMG_ENTRY(1, EN_SNOWMAN_DMGEFF_NONE),
    /* Goron pound    */ DMG_ENTRY(1, EN_SNOWMAN_DMGEFF_NONE),
    /* Fire arrow     */ DMG_ENTRY(2, EN_SNOWMAN_DMGEFF_MELT),
    /* Ice arrow      */ DMG_ENTRY(1, EN_SNOWMAN_DMGEFF_NONE),
    /* Light arrow    */ DMG_ENTRY(2, EN_SNOWMAN_DMGEFF_LIGHT_ORB),
    /* Goron spikes   */ DMG_ENTRY(1, EN_SNOWMAN_DMGEFF_NONE),
    /* Deku spin      */ DMG_ENTRY(0, EN_SNOWMAN_DMGEFF_STUN),
    /* Deku bubble    */ DMG_ENTRY(1, EN_SNOWMAN_DMGEFF_NONE),
    /* Deku launch    */ DMG_ENTRY(2, EN_SNOWMAN_DMGEFF_NONE),
    /* UNK_DMG_0x12   */ DMG_ENTRY(0, EN_SNOWMAN_DMGEFF_STUN),
    /* Zora barrier   */ DMG_ENTRY(0, EN_SNOWMAN_DMGEFF_ELECTRIC_STUN),
    /* Normal shield  */ DMG_ENTRY(0, EN_SNOWMAN_DMGEFF_NONE),
    /* Light ray      */ DMG_ENTRY(0, EN_SNOWMAN_DMGEFF_NONE),
    /* Thrown object  */ DMG_ENTRY(1, EN_SNOWMAN_DMGEFF_NONE),
    /* Zora punch     */ DMG_ENTRY(1, EN_SNOWMAN_DMGEFF_NONE),
    /* Spin attack    */ DMG_ENTRY(1, EN_SNOWMAN_DMGEFF_NONE),
    /* Sword beam     */ DMG_ENTRY(0, EN_SNOWMAN_DMGEFF_NONE),
    /* Normal Roll    */ DMG_ENTRY(0, EN_SNOWMAN_DMGEFF_NONE),
    /* UNK_DMG_0x1B   */ DMG_ENTRY(0, EN_SNOWMAN_DMGEFF_NONE),
    /* UNK_DMG_0x1C   */ DMG_ENTRY(0, EN_SNOWMAN_DMGEFF_NONE),
    /* Unblockable    */ DMG_ENTRY(0, EN_SNOWMAN_DMGEFF_NONE),
    /* UNK_DMG_0x1E   */ DMG_ENTRY(0, EN_SNOWMAN_DMGEFF_NONE),
    /* Powder Keg     */ DMG_ENTRY(1, EN_SNOWMAN_DMGEFF_NONE),
};

static CollisionCheckInfoInit sColChkInfoInit = { 2, 60, 80, 150 };

static ColliderCylinderInit sEenoCylinderInit = {
    {
        COL_MATERIAL_HIT4,
        AT_NONE,
        AC_ON | AC_TYPE_PLAYER,
        OC1_ON | OC1_TYPE_ALL,
        OC2_TYPE_1,
        COLSHAPE_CYLINDER,
    },
    {
        ELEM_MATERIAL_UNK0,
        { 0.0f, 0x00, 0x00 },
        { 0xF7CFFFFF, 0x00, 0x00 },
        ATELEM_NONE | ATELEM_SFX_NORMAL,
        ACELEM_ON | ACELEM_HOOKABLE,
        OCELEM_ON,
    },
    { 60, 80, 0, { 0, 0, 0 } },
};

static ColliderCylinderInit sSnowballCylinderInit = {
    {
        COL_MATERIAL_NONE,
        AT_ON | AT_TYPE_ENEMY,
        AC_ON | AC_TYPE_PLAYER,
        OC1_NONE | OC1_TYPE_ALL,
        OC2_TYPE_1,
        COLSHAPE_CYLINDER,
    },
    {
        ELEM_MATERIAL_UNK0,
        { 0xF7CFFFFF, 0x00, 0x04 },
        { 0xF7CFFFFF, 0x00, 0x00 },
        ATELEM_ON | ATELEM_SFX_NONE,
        ACELEM_ON,
        OCELEM_ON,
    },
    { 60, 80, 0, { 0, 0, 0 } },
};


static InitChainEntry sInitChain[] = {
    ICHAIN_S8(hintId, TATL_HINT_ID_EENO, ICHAIN_CONTINUE),
    ICHAIN_F32(lockOnArrowOffset, 3000, ICHAIN_CONTINUE),
    ICHAIN_F32_DIV1000(gravity, -1000, ICHAIN_STOP),
};

RECOMP_PATCH void EnSnowman_Init(Actor* thisx, PlayState* play) {
    s32 pad;
    EnSnowman* this = (EnSnowman*)thisx;
    s32 attackRange;

    int Difficulty = (int)recomp_get_config_double("diff_option");

    Actor_ProcessInitChain(thisx, sInitChain);
    attackRange = EN_SNOWMAN_GET_ATTACK_RANGE(thisx);
    if (attackRange == 0xFF) {
        attackRange = 0;
    }

    thisx->params &= 7;

    if (EN_SNOWMAN_GET_TYPE(thisx) < EN_SNOWMAN_TYPE_SMALL_SNOWBALL) {
        SkelAnime_InitFlex(play, &this->skelAnime, &gEenoSkel, &gEenoEmergeAnim,
            this->jointTable, this->morphTable, EENO_LIMB_MAX);

        SkelAnime_InitFlex(play, &this->snowPileSkelAnime, &gEenoSnowPileSkel,
            &gEenoSnowPileMoveAnim, this->snowPileJointTable,
            this->snowPileMorphTable, EENO_SNOW_PILE_LIMB_MAX);

        CollisionCheck_SetInfo(&thisx->colChkInfo, &sDamageTable, &sColChkInfoInit);
        Collider_InitAndSetCylinder(play, &this->collider, thisx, &sEenoCylinderInit);

        if (EN_SNOWMAN_GET_TYPE(thisx) == EN_SNOWMAN_TYPE_LARGE) {
            thisx->flags |= ACTOR_FLAG_HOOKSHOT_PULLS_PLAYER;

            Actor_SpawnAsChild(&play->actorCtx, thisx, play, ACTOR_EN_SNOWMAN,
                thisx->world.pos.x, thisx->world.pos.y, thisx->world.pos.z,
                0, 0, 0, EN_SNOWMAN_TYPE_SPLIT);

            thisx->parent = Actor_SpawnAsChildAndCutscene(&play->actorCtx, play,
                ACTOR_EN_SNOWMAN, thisx->world.pos.x, thisx->world.pos.y,
                thisx->world.pos.z, 0, 0, 0,
                EN_SNOWMAN_TYPE_SPLIT, CS_ID_NONE,
                thisx->halfDaysBits, NULL);

            if ((thisx->child == NULL) || (thisx->parent == NULL)) {
                if (thisx->child != NULL) {
                    Actor_Kill(thisx->child);
                }

                if (thisx->parent != NULL) {
                    Actor_Kill(thisx->parent);
                }

                Actor_Kill(thisx);
                return;
            }

            thisx->parent->child = thisx;
            thisx->child->child = thisx->parent;
            thisx->parent->parent = thisx->child;

            if (1) {}

            Actor_SetScale(thisx, 0.02f);
        }

        this->eenoScale = thisx->scale.x * 100.0f;
        this->attackRange =
            (240.0f * this->eenoScale) + (attackRange * 0.1f * 40.0f);

        if (EN_SNOWMAN_GET_TYPE(thisx) == EN_SNOWMAN_TYPE_SPLIT) {
            EnSnowman_SetupSplitDoNothing(this);
        }
        else {
            EnSnowman_SetupMoveSnowPile(this);
        }

    }
    else {

        Player* player = GET_PLAYER(play);

        Vec3f predictedPos;
        f32 projectileSpeed;
        f32 dx;
        f32 dz;
        f32 distance;
        f32 travelTime;
        s16 targetYaw;

        thisx->flags &= ~ACTOR_FLAG_ATTENTION_ENABLED;

        Collider_InitAndSetCylinder(play, &this->collider,
            thisx, &sSnowballCylinderInit);

        if (EN_SNOWMAN_GET_TYPE(thisx) == EN_SNOWMAN_TYPE_SMALL_SNOWBALL) {
            projectileSpeed = 15.0f;
        }
        else {
            projectileSpeed = 22.5f;
        }

        dx = player->actor.world.pos.x - thisx->world.pos.x;
        dz = player->actor.world.pos.z - thisx->world.pos.z;

        distance = sqrtf(SQ(dx) + SQ(dz));

        travelTime = distance / projectileSpeed;

        f32 predictFactor;

        if (Difficulty == 1) {
            predictFactor = 0.75f;
        }
        else {
            predictFactor = 0.25f;
        }

        travelTime = CLAMP(travelTime, 0.0f, 12.0f);

        predictedPos.x = player->actor.world.pos.x +
            (player->actor.velocity.x * travelTime * predictFactor);

        predictedPos.y = player->actor.world.pos.y;

        predictedPos.z = player->actor.world.pos.z +
            (player->actor.velocity.z * travelTime * predictFactor);

        targetYaw = Math_Vec3f_Yaw(&thisx->world.pos, &predictedPos);

        thisx->world.rot.y = targetYaw;

        thisx->velocity.y =
            (Math_Vec3f_DistXZ(&thisx->world.pos, &predictedPos) * 0.035f) - 5.0f;

        thisx->velocity.y = CLAMP_MAX(thisx->velocity.y, 3.5f);

        if (EN_SNOWMAN_GET_TYPE(thisx) == EN_SNOWMAN_TYPE_SMALL_SNOWBALL) {
            thisx->speed = 15.0f;
        }
        else {
            thisx->speed = 22.5f;
            thisx->velocity.y *= 1.5f;
        }

        if (Difficulty == 1) {
            thisx->speed *= 1.15f;
        }

        thisx->world.pos.x +=
            thisx->speed * Math_SinS(thisx->world.rot.y);

        thisx->world.pos.y += thisx->velocity.y;

        thisx->world.pos.z +=
            thisx->speed * Math_CosS(thisx->world.rot.y);

        if (EN_SNOWMAN_GET_TYPE(thisx) == EN_SNOWMAN_TYPE_SMALL_SNOWBALL) {

            this->collider.dim.radius = 8;
            this->collider.dim.height = 12;
            this->collider.dim.yShift = -6;

            ActorShape_Init(&thisx->shape, 0.0f,
                ActorShadow_DrawCircle, 10.0f);

        }
        else {

            this->collider.dim.radius = 50;
            this->collider.dim.height = 122;
            this->collider.dim.yShift = -8;
            this->collider.elem.atDmgInfo.damage = 16;

            thisx->world.pos.y -= 32.0f;

            Actor_SetScale(thisx, 0.006f);

            ActorShape_Init(&thisx->shape,
                16000.0f / 3.0f,
                ActorShadow_DrawCircle,
                170.0f);

            thisx->gravity = -1.5f;
        }

        thisx->flags |= ACTOR_FLAG_UPDATE_CULLING_DISABLED;

        thisx->update = EnSnowman_UpdateSnowball;
        thisx->draw = EnSnowman_DrawSnowball;

        this->work.timer = 5;
    }
}

RECOMP_HOOK("EnSnowman_Update") void SpeedupSnow(Actor* thisx, PlayState* play) {

    EnSnowman* this = (EnSnowman*)thisx;

    int Difficulty = (int)recomp_get_config_double("diff_option");
    u8 baseHealth = this->actor.colChkInfo.health;

    switch (Difficulty) {
    case 0:
        if (this->actor.colChkInfo.health != 0) this->skelAnime.playSpeed = 1.5f;
        this->combineTimer = 2;
        break;

    case 1:
        if (this->actor.colChkInfo.health != 0) this->skelAnime.playSpeed = 2.25f;
        this->work.snowballsToThrowBeforeIdling = 0;
        this->combineTimer = 2;
        break;

    default:
        break;
    }
}

RECOMP_HOOK("EnSnowman_MoveSnowPile") void SnowMovement(EnSnowman* this, PlayState* play) {

    Player* player = GET_PLAYER(play);

    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        this->attackRange =
            (240.0f * this->eenoScale * 0.75f) +
            (this->attackRange * 0.01f * 50.0f);
        break;

    case 1:
        this->attackRange =
            (240.0f * this->eenoScale * 0.75f) +
            (this->attackRange * 0.01f * 60.0f);
        break;

    default:
        break;
    }

    if ((this->work.timer == 0) &&
        (fabsf(this->actor.playerHeightRel) < 60.0f) &&
        (this->actor.xzDistToPlayer < this->attackRange) &&
        (Player_GetMask(play) != PLAYER_MASK_STONE) &&
        !(player->stateFlags1 & PLAYER_STATE1_800000)) {

        EnSnowman_SetupEmerge(this, play);
    }
}

RECOMP_HOOK("EnSnowman_SetupIdle") void IdleChanges(EnSnowman* this) {

    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        this->work.timer = 40;
        break;

    case 1:
        this->work.timer = 20;
        break;

    default:
        break;
    }

}