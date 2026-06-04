#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_en_fz.h"
#include "attributes.h"
#include "overlays/actors/ovl_En_Wiz/z_en_wiz.h"
#include "assets/objects/object_fz/object_fz.h"
#include "assets/objects/gameplay_keep/gameplay_keep.h"

void func_80932784(EnFz* this, PlayState* play);
void func_809330D4(EnFz* this);
void func_809340BC(EnFz* this, Vec3f* a, Vec3f* b, Vec3f* c, f32 arg4, f32 arg5, s16 arg6, u8 arg7);

extern void func_80933014(EnFz*);

RECOMP_PATCH void func_809338E0(EnFz* this, PlayState* play) {
    Vec3f sp64;
    Vec3f sp58;
    Vec3f sp4C;
    Vec3f sp40;
    u8 sp3F;
    s16 sp3C;
    s16 rotY;

    int Difficulty = (int)recomp_get_config_double("diff_option");

    if (this->unk_BC6 & (0x80 | 0x40)) {
        func_80933014(this);
        func_80932784(this, play);
        return;
    }

    sp3F = 0;
    sp3C = 150;
    Actor_PlaySfx_Flagged(&this->actor, NA_SE_EN_FREEZAD_BREATH - SFX_FLAG);

    if ((this->unk_BC6 & 0x3F) >= 0x30) {
        sp3C = 630 - ((this->unk_BC6 & 0x3F) * 10);
    }

    sp40.x = sp40.z = 0.0f;
    sp40.y = 0.6f;

    sp58.x = this->actor.world.pos.x;
    sp58.y = this->actor.world.pos.y + 20.0f;
    sp58.z = this->actor.world.pos.z;

    rotY = this->actor.shape.rot.y;

    if (Difficulty == 1) {
        rotY += (s16)(Math_SinS(this->unk_BC6 * 0x600) * 8192.0f);
    }

    Matrix_RotateYS(rotY, MTXMODE_NEW);

    sp64.x = 0.0f;
    sp64.y = -2.0f;
    sp64.z = ((ENFZ_GET_F(&this->actor) == ENFZ_F_1) ? 10.0f
        : (ENFZ_GET_F(&this->actor) == ENFZ_F_2) ? 20.0f
        : 0.0f) +
        20;

    Matrix_MultVec3f(&sp64, &sp4C);

    if (!(this->unk_BC6 & 7)) {
        sp3F = 1;
    }

    func_809340BC(this, &sp58, &sp4C, &sp40, 2.0f, 25.0f, sp3C, sp3F);
    sp58.x += sp4C.x * 0.5f;
    sp58.y += sp4C.y * 0.5f;
    sp58.z += sp4C.z * 0.5f;
    func_809340BC(this, &sp58, &sp4C, &sp40, 2.0f, 25.0f, sp3C, 0);
}

void func_809334B8(EnFz* this, PlayState* play) {
    Vec3f sp64;
    Vec3f sp58;
    Vec3f sp4C;
    Vec3f sp40;
    u8 sp3F;
    s16 sp3C;
    s16 rotY;
    int Difficulty = (int)recomp_get_config_double("diff_option");

    if (this->unk_BCA == 0) {
        func_809330D4(this);
        return;
    }

    if (this->unk_BCA > 10) {
        sp3F = 0;
        sp3C = 150;
        Actor_PlaySfx_Flagged(&this->actor, NA_SE_EN_FREEZAD_BREATH - SFX_FLAG);
        if ((this->unk_BCA - 10) < 16) {
            sp3C = (this->unk_BCA * 10) - 100;
        }

        sp40.x = sp40.z = 0.0f;
        sp40.y = 0.6f;

        sp58.x = this->actor.world.pos.x;
        sp58.y = this->actor.world.pos.y + 20.0f;
        sp58.z = this->actor.world.pos.z;
        rotY = this->actor.shape.rot.y;

        if (Difficulty == 1) {
            rotY += (s16)(Math_SinS(this->unk_BCA * 0x600) * 8192.0f);
        }

        Matrix_RotateYS(rotY, MTXMODE_NEW);

        sp64.x = 0.0f;
        sp64.y = -2.0f;
        sp64.z = ((ENFZ_GET_F(&this->actor) == ENFZ_F_1) ? 10.0f
            : (ENFZ_GET_F(&this->actor) == ENFZ_F_2) ? 20.0f
            : 0.0f) +
            20;

        Matrix_MultVec3f(&sp64, &sp4C);

        if ((this->unk_BCA & 7) == 0) {
            sp3F = 1;
        }

        func_809340BC(this, &sp58, &sp4C, &sp40, 2.0f, 25.0f, sp3C, sp3F);

        sp58.x += sp4C.x * 0.5f;
        sp58.y += sp4C.y * 0.5f;
        sp58.z += sp4C.z * 0.5f;

        func_809340BC(this, &sp58, &sp4C, &sp40, 2.0f, 25.0f, sp3C, 0);
    }
}

RECOMP_HOOK("EnFz_Update") void FzUpdate(Actor* thisx, PlayState* play) {

    EnFz* this = (EnFz*)thisx;

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