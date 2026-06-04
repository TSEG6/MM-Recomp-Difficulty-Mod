#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_en_talk_gibud.h"
#include "z64rumble.h"

int Atktimer = 600;

void EnTalkGibud_Idle(EnTalkGibud* this, PlayState* play);
void EnTalkGibud_Grab(EnTalkGibud* this, PlayState* play);
void EnTalkGibud_WalkToPlayer(EnTalkGibud* this, PlayState* play);
void EnTalkGibud_SetupWalkToHome(EnTalkGibud* this);
void EnTalkGibud_WalkToHome(EnTalkGibud* this, PlayState* play);
void EnTalkGibud_SetupDamage(EnTalkGibud* this);
void EnTalkGibud_Damage(EnTalkGibud* this, PlayState* play);
void EnTalkGibud_Dead(EnTalkGibud* this, PlayState* play);
void EnTalkGibud_Revive(EnTalkGibud* this, PlayState* play);
void EnTalkGibud_PassiveIdle(EnTalkGibud* this, PlayState* play);
void EnTalkGibud_Talk(EnTalkGibud* this, PlayState* play);
void EnTalkGibud_Disappear(EnTalkGibud* this, PlayState* play);

typedef struct {
    /* 0x0 */ PlayerItemAction itemAction;
    /* 0x4 */ ItemId item;
    /* 0x8 */ s32 amount;
    /* 0xC */ s16 isBottledItem;
} EnTalkGibudRequestedItem; // size = 0x10

typedef enum EnTalkGibudRequestedItemState {
    /* 0 */ EN_TALK_GIBUD_REQUESTED_ITEM_MET,
    /* 1 */ EN_TALK_GIBUD_REQUESTED_ITEM_NOT_ENOUGH_AMMO,
    /* 2 */ EN_TALK_GIBUD_REQUESTED_ITEM_NOT_MET
} EnTalkGibudRequestedItemState;

typedef enum EnTalkGibudRequestedItemIndex {
    /* 0 */ EN_TALK_GIBUD_REQUESTED_ITEM_INDEX_BLUE_POTION,
    /* 1 */ EN_TALK_GIBUD_REQUESTED_ITEM_INDEX_BEANS,
    /* 2 */ EN_TALK_GIBUD_REQUESTED_ITEM_INDEX_SPRING_WATER,
    /* 3 */ EN_TALK_GIBUD_REQUESTED_ITEM_INDEX_FISH,
    /* 4 */ EN_TALK_GIBUD_REQUESTED_ITEM_INDEX_BUGS,
    /* 5 */ EN_TALK_GIBUD_REQUESTED_ITEM_INDEX_DEKU_NUTS,
    /* 6 */ EN_TALK_GIBUD_REQUESTED_ITEM_INDEX_BOMBS,
    /* 7 */ EN_TALK_GIBUD_REQUESTED_ITEM_INDEX_HOT_SPRING_WATER,
    /* 8 */ EN_TALK_GIBUD_REQUESTED_ITEM_INDEX_BIG_POE,
    /* 9 */ EN_TALK_GIBUD_REQUESTED_ITEM_INDEX_MILK
} EnTalkGibudRequestedItemIndex;

typedef enum EnTalkGibudType {
    /* 0 */ EN_TALK_GIBUD_TYPE_GIBDO,
    /* 1 */ EN_TALK_GIBUD_TYPE_REDEAD
} EnTalkGibudType;

typedef enum EnTalkGibudAnimation {
    /* 0 */ EN_TALK_GIBUD_ANIM_GRAB_ATTACK,
    /* 1 */ EN_TALK_GIBUD_ANIM_GRAB_END,
    /* 2 */ EN_TALK_GIBUD_ANIM_GRAB_START,
    /* 3 */ EN_TALK_GIBUD_ANIM_LOOK_BACK,
    /* 4 */ EN_TALK_GIBUD_ANIM_CROUCH_WIPING_TEARS,
    /* 5 */ EN_TALK_GIBUD_ANIM_CROUCH_CRYING,
    /* 6 */ EN_TALK_GIBUD_ANIM_DEATH,
    /* 7 */ EN_TALK_GIBUD_ANIM_DAMAGE,
    /* 8 */ EN_TALK_GIBUD_ANIM_CROUCH_END,
    /* 9 */ EN_TALK_GIBUD_ANIM_IDLE,
    /* 10 */ EN_TALK_GIBUD_ANIM_WALK,
    /* 11 */ EN_TALK_GIBUD_ANIM_DANCE_SQUAT,
    /* 12 */ EN_TALK_GIBUD_ANIM_DANCE_PIROUETTE,
    /* 13 */ EN_TALK_GIBUD_ANIM_DANCE_CLAP,
    /* 14 */ EN_TALK_GIBUD_ANIM_MAX
} EnTalkGibudAnimation;

static AnimationInfo sAnimationInfo[EN_TALK_GIBUD_ANIM_MAX] = {
    { &gGibdoRedeadGrabAttackAnim, 1.0f, 0.0f, 0.0f, ANIMMODE_LOOP, -8.0f },    // EN_TALK_GIBUD_ANIM_GRAB_ATTACK
    { &gGibdoRedeadGrabEndAnim, 0.5f, 0.0f, 0.0f, ANIMMODE_ONCE_INTERP, 0.0f }, // EN_TALK_GIBUD_ANIM_GRAB_END
    { &gGibdoRedeadGrabStartAnim, 1.0f, 0.0f, 0.0f, ANIMMODE_ONCE, -8.0f },      // EN_TALK_GIBUD_ANIM_GRAB_START
    { &gGibdoRedeadLookBackAnim, 0.0f, 0.0f, 0.0f, ANIMMODE_ONCE, -8.0f },       // EN_TALK_GIBUD_ANIM_LOOK_BACK
    { &gGibdoRedeadWipingTearsAnim, 1.0f, 0.0f, 0.0f, ANIMMODE_ONCE, -8.0f }, // EN_TALK_GIBUD_ANIM_CROUCH_WIPING_TEARS
    { &gGibdoRedeadSobbingAnim, 1.0f, 0.0f, 0.0f, ANIMMODE_LOOP, -8.0f },     // EN_TALK_GIBUD_ANIM_CROUCH_CRYING
    { &gGibdoRedeadDeathAnim, 1.0f, 0.0f, 0.0f, ANIMMODE_ONCE, -8.0f },       // EN_TALK_GIBUD_ANIM_DEATH
    { &gGibdoRedeadDamageAnim, 1.0f, 0.0f, 0.0f, ANIMMODE_ONCE, -8.0f },       // EN_TALK_GIBUD_ANIM_DAMAGE
    { &gGibdoRedeadStandUpAnim, 1.0f, 0.0f, 0.0f, ANIMMODE_ONCE, -8.0f },     // EN_TALK_GIBUD_ANIM_CROUCH_END
    { &gGibdoRedeadIdleAnim, 1.0f, 0.0f, 0.0f, ANIMMODE_LOOP, -8.0f },         // EN_TALK_GIBUD_ANIM_IDLE
    { &gGibdoRedeadWalkAnim, 0.4f, 0.0f, 0.0f, ANIMMODE_LOOP_INTERP, -8.0f }, // EN_TALK_GIBUD_ANIM_WALK
    { &gGibdoRedeadSquattingDanceAnim, 1.0f, 0.0f, 0.0f, ANIMMODE_LOOP, -8.0f }, // EN_TALK_GIBUD_ANIM_DANCE_SQUAT
    { &gGibdoRedeadPirouetteAnim, 1.0f, 0.0f, 0.0f, ANIMMODE_LOOP, -8.0f },      // EN_TALK_GIBUD_ANIM_DANCE_PIROUETTE
    { &gGibdoRedeadClappingDanceAnim, 1.0f, 0.0f, 0.0f, ANIMMODE_LOOP, -8.0f },  // EN_TALK_GIBUD_ANIM_DANCE_CLAP
};

static EnTalkGibudRequestedItem sRequestedItemTable[] = {
    { PLAYER_IA_BOTTLE_POTION_BLUE, ITEM_POTION_BLUE, 1, true },
    { PLAYER_IA_MAGIC_BEANS, ITEM_MAGIC_BEANS, 5, false },
    { PLAYER_IA_BOTTLE_SPRING_WATER, ITEM_SPRING_WATER, 1, true },
    { PLAYER_IA_BOTTLE_FISH, ITEM_FISH, 1, true },
    { PLAYER_IA_BOTTLE_BUG, ITEM_BUG, 1, true },
    { PLAYER_IA_DEKU_NUT, ITEM_DEKU_NUT, 10, false },
    { PLAYER_IA_BOMB, ITEM_BOMB, 10, false },
    { PLAYER_IA_BOTTLE_HOT_SPRING_WATER, ITEM_HOT_SPRING_WATER, 1, true },
    { PLAYER_IA_BOTTLE_BIG_POE, ITEM_BIG_POE, 1, true },
    { PLAYER_IA_BOTTLE_MILK, ITEM_MILK_BOTTLE, 1, true },
};

void EnTalkGibud_SetupPassiveIdle(EnTalkGibud* this) {
    this->isTalking = false;
    if (this->actionFunc != EnTalkGibud_Talk) {
        Actor_ChangeAnimationByInfo(&this->skelAnime, sAnimationInfo, EN_TALK_GIBUD_ANIM_IDLE);
    }
    this->actionFunc = EnTalkGibud_PassiveIdle;
}

// If you failed to give an item for a duration of time it'll act like you aren't wearing the gibdo mask
RECOMP_PATCH void EnTalkGibud_CheckForGibdoMask(EnTalkGibud* this, PlayState* play) {
    if ((this->actionFunc != EnTalkGibud_Grab) && (this->actionFunc != EnTalkGibud_Dead) &&
        (this->actionFunc != EnTalkGibud_Disappear) && (this->actionFunc != EnTalkGibud_Revive) &&
        (this->actionFunc != EnTalkGibud_Damage) && (this->actionFunc != EnTalkGibud_Talk)) {

        if (this->actor.home.rot.z) {
            if (this->actionFunc == EnTalkGibud_PassiveIdle) {
                this->actor.flags &= ~(ACTOR_FLAG_ATTENTION_ENABLED | ACTOR_FLAG_FRIENDLY);
                this->actor.flags |= (ACTOR_FLAG_ATTENTION_ENABLED | ACTOR_FLAG_HOSTILE);
                if (this->type == EN_TALK_GIBUD_TYPE_REDEAD) {
                    this->actor.hintId = TATL_HINT_ID_REDEAD;
                }
                else {
                    this->actor.hintId = TATL_HINT_ID_GIBDO;
                }
                this->actor.textId = 0;
                EnTalkGibud_SetupWalkToHome(this);
            }
        }
        else if (this->actionFunc != EnTalkGibud_PassiveIdle) {
            if (Player_GetMask(play) == PLAYER_MASK_GIBDO) {
                this->actor.flags &= ~(ACTOR_FLAG_ATTENTION_ENABLED | ACTOR_FLAG_HOSTILE);
                this->actor.flags |= (ACTOR_FLAG_ATTENTION_ENABLED | ACTOR_FLAG_FRIENDLY);
                this->actor.hintId = TATL_HINT_ID_NONE;
                this->actor.textId = 0;
                EnTalkGibud_SetupPassiveIdle(this);
            }
        }
        else if (Player_GetMask(play) != PLAYER_MASK_GIBDO) {
            this->actor.flags &= ~(ACTOR_FLAG_ATTENTION_ENABLED | ACTOR_FLAG_FRIENDLY);
            this->actor.flags |= (ACTOR_FLAG_ATTENTION_ENABLED | ACTOR_FLAG_HOSTILE);
            if (this->type == EN_TALK_GIBUD_TYPE_REDEAD) {
                this->actor.hintId = TATL_HINT_ID_REDEAD;
            }
            else {
                this->actor.hintId = TATL_HINT_ID_GIBDO;
            }
            this->actor.textId = 0;
            EnTalkGibud_SetupWalkToHome(this);
        }
    }
}

// Set the failed to give item to true
RECOMP_HOOK("EnTalkGibud_Talk") void GibdoTalk(EnTalkGibud* this, PlayState* play) {

    Player* player = GET_PLAYER(play);
    EnTalkGibudRequestedItem* requestedItem;

    double raw_diff = recomp_get_config_double("diff_option");
    int Difficulty = (int)raw_diff;

    switch (Message_GetState(&play->msgCtx)) {
    case TEXT_STATE_DONE:
        if (Message_ShouldAdvance(play)) {
            if (this->textId == 0x138A) {
                requestedItem = &sRequestedItemTable[this->requestedItemIndex];
                if (!requestedItem->isBottledItem) {

                }
                else {

                }
            }
            else {
                if (Difficulty == 1) {
                    this->actor.home.rot.z = true;
                }
            }
        }
        break;
    }
}

// Timer to go back to normal if you failed to give an item & hides HUD if player is frozen
RECOMP_HOOK("EnTalkGibud_Update") void GibdoTalkUpdate(Actor* thisx, PlayState* play) {

    Player* player = GET_PLAYER(play);
    EnTalkGibud* this = (EnTalkGibud*)thisx;

    int Difficulty = (int)recomp_get_config_double("diff_option");
    float speedMultiplier = 1.0f;

    if (this->actor.home.rot.z) {

        if (Atktimer == 0) {

            this->actor.home.rot.z = false;
            Atktimer = 600;
        }
        Atktimer--;
    }

    switch (Difficulty) {
    case 0:
        speedMultiplier = 1.5f;
        this->actor.colChkInfo.damage = (this->actor.colChkInfo.damage) / 2;
        break;

    case 1:
        speedMultiplier = 3.0f;
        this->actor.colChkInfo.damage = (this->actor.colChkInfo.damage + 2) / 3;
        break;

    default:
        break;
    }

    if (this->actor.colChkInfo.health != 0 &&
        (this->actionFunc == EnTalkGibud_WalkToPlayer ||
            this->actionFunc == EnTalkGibud_WalkToHome)) {
        this->skelAnime.playSpeed = speedMultiplier;
    }

    if (this->actor.speed <= 1.0f &&
        (this->actionFunc == EnTalkGibud_WalkToPlayer ||
            this->actionFunc == EnTalkGibud_WalkToHome)) {
        this->actor.speed *= speedMultiplier;
    }
    if (this->actor.colChkInfo.health == 0) this->actor.speed = 0;

    if (player->actor.freezeTimer > 0) {

        Interface_SetHudVisibility(HUD_VISIBILITY_NONE);
    }
    else {

        Interface_SetHudVisibility(HUD_VISIBILITY_ALL);
    }
}