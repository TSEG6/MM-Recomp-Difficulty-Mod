#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_en_st.h"
#include "overlays/actors/ovl_En_Clear_Tag/z_en_clear_tag.h"
#include "assets/objects/gameplay_keep/gameplay_keep.h"


RECOMP_PATCH s16 func_808A5BEC(EnSt* this) {
    s16 ret;

    int Difficulty = (int)recomp_get_config_double("diff_option");

    if (!(this->unk_18C & 4) || (this->actor.xzDistToPlayer > 180.0f)) {
        ret = this->actor.home.rot.y;
    }
    else {
        ret = this->actor.yawTowardsPlayer;

        if (DECR(this->unk_30E) == 0) {
            float turnAwayChance = 0.25f;

            if (this->unk_18C & 2) {
                this->unk_18C &= ~2;

                Actor_PlaySfx(&this->actor, NA_SE_EN_STALTU_ROLL);
            }
            else {

                switch (Difficulty) {
                case 0:
                    turnAwayChance = 0.33f;
                    break;

                case 1:
                    turnAwayChance = 0.10f;
                    break;
                }

                if (Rand_ZeroOne() < turnAwayChance) {
                    this->unk_18C |= 2;

                    Actor_PlaySfx(&this->actor, NA_SE_EN_STALTU_ROLL);
                }
            }

            this->unk_310 = 8;

            if (this->unk_18C & 1) {
                this->unk_310 *= 2;
            }

            this->unk_30E = 30;
        }

        if (this->unk_18C & 2) {
            ret = BINANG_ROT180(ret);
        }
    }

    return ret;
}