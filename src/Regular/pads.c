#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_en_raf.h"

void EnRaf_SetupIdle(EnRaf* this);
void EnRaf_SetupGrab(EnRaf* this);

typedef enum CarnivorousLilyAction {
    /* 0 */ CARNIVOROUS_LILY_ACTION_IDLE,
    /* 1 */ CARNIVOROUS_LILY_ACTION_GRAB,
    /* 2 */ CARNIVOROUS_LILY_ACTION_CHEW,
    /* 3 */ CARNIVOROUS_LILY_ACTION_THROW,
    /* 4 */ CARNIVOROUS_LILY_ACTION_EXPLODE,
    /* 5 */ CARNIVOROUS_LILY_ACTION_CONVULSE,
    /* 6 */ CARNIVOROUS_LILY_ACTION_DISSOLVE,
    /* 7 */ CARNIVOROUS_LILY_ACTION_DORMANT
} CarnivorousLilyAction;

typedef enum CarnivorousLilyGrabTarget {
    /* 0 */ CARNIVOROUS_LILY_GRAB_TARGET_PLAYER,
    /* 1 */ CARNIVOROUS_LILY_GRAB_TARGET_EXPLOSIVE,
    /* 2 */ CARNIVOROUS_LILY_GRAB_TARGET_GORON_PLAYER
} CarnivorousLilyGrabTarget;

typedef enum CarnivorousLilyPetalScaleType {
    /* 0 */ CARNIVOROUS_LILY_PETAL_SCALE_TYPE_DEAD,
    /* 1 */ CARNIVOROUS_LILY_PETAL_SCALE_TYPE_GRAB,
    /* 2 */ CARNIVOROUS_LILY_PETAL_SCALE_TYPE_CHEW,
    /* 3 */ CARNIVOROUS_LILY_PETAL_SCALE_TYPE_IDLE_OR_THROW
} CarnivorousLilyPetalScaleType;

typedef enum CarnivorousLilyAnimation {
    /* 0 */ CARNIVOROUS_LILY_ANIM_IDLE,
    /* 1 */ CARNIVOROUS_LILY_ANIM_CLOSE,
    /* 2 */ CARNIVOROUS_LILY_ANIM_CHEW,
    /* 3 */ CARNIVOROUS_LILY_ANIM_SPIT,
    /* 4 */ CARNIVOROUS_LILY_ANIM_CONVULSE,
    /* 5 */ CARNIVOROUS_LILY_ANIM_DEATH,
    /* 6 */ CARNIVOROUS_LILY_ANIM_MAX
} CarnivorousLilyAnimation;

extern void EnRaf_ChangeAnim(EnRaf* this, s32 animIndex);

// Link stood on it, prepare to be eaten (or he can escape too)
RECOMP_HOOK("EnRaf_Idle")
void HungryPads(EnRaf* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    int Difficulty = (int)recomp_get_config_double("diff_option");
    int EatDelay = (Difficulty == 0) ? 40 : 20;

    if (player->transformation != PLAYER_FORM_DEKU) {
        return;
    }

    if (this->timer != 0) {
        return;
    }

    if (this->dissolveTimer == -1) {
        EnRaf_SetupIdle(this);
        this->dissolveTimer = 0;
        return;
    }

    if (this->dissolveTimer == 0) {
        if (!DynaPolyActor_IsPlayerOnTop(&this->dyna) ||
            this->dyna.actor.xzDistToPlayer >= (BREG(48) + 80.0f) ||
            player->invincibilityTimer != 0 ||
            (player->stateFlags1 & PLAYER_STATE1_8000000)) {
            return;
        }
    }

    this->dissolveTimer++;

    if (this->dissolveTimer >= EatDelay) {
        if (DynaPolyActor_IsPlayerOnTop(&this->dyna) &&
            this->dyna.actor.xzDistToPlayer < (BREG(48) + 80.0f) &&
            player->invincibilityTimer == 0 &&
            !(player->stateFlags1 & PLAYER_STATE1_8000000)) {
            this->grabTarget = CARNIVOROUS_LILY_GRAB_TARGET_PLAYER;
            player->av2.actionVar2 = 50;
            this->playerRotYWhenGrabbed = player->actor.world.rot.y;

            EnRaf_SetupGrab(this);

            if (play->grabPlayer(play, player)) {
                player->actor.parent = &this->dyna.actor;
            }
        }
        else {
            EnRaf_ChangeAnim(this, CARNIVOROUS_LILY_ANIM_CLOSE);
            this->petalScaleType = CARNIVOROUS_LILY_PETAL_SCALE_TYPE_GRAB;
            Actor_PlaySfx(&this->dyna.actor, NA_SE_EN_SUISEN_DRINK);
            this->timer = 40;
            this->dissolveTimer = -1;
        }

        if (this->dissolveTimer != -1) {
            this->dissolveTimer = 0;
        }
    }
}