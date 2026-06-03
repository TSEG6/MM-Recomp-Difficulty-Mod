#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_boss_02.h"
#include "z64rumble.h"
#include "z64shrink_window.h"
#include "attributes.h"
#include "overlays/actors/ovl_Door_Warp1/z_door_warp1.h"
#include "overlays/actors/ovl_En_Tanron5/z_en_tanron5.h"
#include "overlays/actors/ovl_Item_B_Heart/z_item_b_heart.h"
#include "assets/objects/gameplay_keep/gameplay_keep.h"

extern Boss02* sTwinmoldBattleHandler;
extern Boss02* sRedTwinmold;
extern f32 sGiantModeScaleFactor_ovl_Boss_02;
extern Vec3f D_809DFA2C[];
extern Vec3f D_809DF9C0[];
extern u8 sIsInGiantMode;
extern Color_RGBA8 D_809DFA98;
extern u8 sCanSkipMaskOnCs;

#define sGiantModeScaleFactor sGiantModeScaleFactor_ovl_Boss_02

extern void Boss02_SpawnEffectFragment(TwinmoldEffect*, Vec3f*);
extern void Boss02_SpawnEffectFlash(TwinmoldEffect*, Vec3f*);

RECOMP_PATCH void func_809DAB78(Boss02* this, PlayState* play) {
	s32 pad;
	Player* player = GET_PLAYER(play);
	CollisionPoly* spDC;
	Vec3f spD0;
	f32 spCC;
	f32 spC8;
	f32 spC4;
	s32 i;
	f32 temp_f0;
	f32 phi_f2;
	s16 temp_s0;
	s16 temp_s2;
	Boss02* otherTwinmold = this->otherTwinmold;
	Vec3f spA4;
	f32 spA0;
	f32 sp9C;
	Vec3f sp90;
	CollisionPoly* sp8C;
	s32 bgId;
	Vec3f sp7C;
	Vec3f sp70;
	Vec3f sp64;
	u8 isCutscene;
	f32 diffSpeedMult;
	f32 distToOther;
	f32 myDistToPlayer;
	f32 otherDistToPlayer;
	f32 arenaBounds;
	f32 pushX;
	f32 pushZ;
	f32 pushDist;
	u8 foreveralone;
	s32 myHealth;
	s32 otherHealth;
	f32 sweepX;
	f32 sweepZ;

	int Difficulty = (int)recomp_get_config_double("diff_option");

	foreveralone = (otherTwinmold == NULL) || (otherTwinmold->unk_0144 >= 20) || (otherTwinmold->actor.colChkInfo.health <= 0);

	spCC = this->unk_01B0.x - this->actor.world.pos.x;
	spC8 = this->unk_01B0.y - this->actor.world.pos.y;
	spC4 = this->unk_01B0.z - this->actor.world.pos.z;

	if ((this->unk_0144 != 10) && (this->unk_0144 <= 20)) {
		SkelAnime_Update(&this->skelAnime);
		temp_s0 = Math_Atan2S(spCC, spC4);
		temp_s2 = Math_Atan2S(spC8, sqrtf(SQ(spCC) + SQ(spC4)));
		Math_ApproachS(&this->actor.world.rot.y, temp_s0, 0x14, this->unk_0164);
		Math_ApproachS(&this->actor.shape.rot.x, temp_s2, 0x14, this->unk_0164);
		Math_ApproachS(&this->unk_0198, this->unk_019A, 1, 0x20);
		this->unk_0196 += this->unk_0198;
		Math_ApproachF(&this->unk_019C, this->unk_01A0, 0.1f, 100.0f);
		this->unk_01A4 = Math_SinS(this->unk_0196) * this->unk_019C;
		this->actor.world.rot.x = this->actor.shape.rot.x + this->unk_01A4;

		if (!(this->unk_014C & 0x1F) && (sTwinmoldBattleHandler->unk_1D20 == 0)) {
			this->unk_01A0 = Rand_ZeroFloat(0x1000) + 0x800;
			this->unk_019A = Rand_ZeroFloat(0x400) + 0x200;
		}

		isCutscene = (play->csCtx.state != 0) || (this->unk_0144 >= 20);

		diffSpeedMult = 1.0f;
		if (!isCutscene) {
			if (Difficulty == 0) {
				diffSpeedMult = 1.5f;
			}
			else if (Difficulty == 1) {
				diffSpeedMult = 2.0f;
			}
			if (foreveralone) {
				diffSpeedMult *= 1.2f;
			}
		}

		if (this->unk_0195 != 0) {
			Math_ApproachF(&this->unk_0164, this->unk_0168 * 1.5f, 1.0f, 62.5f * (diffSpeedMult * 2));
		}
		else {
			Math_ApproachF(&this->unk_0164, this->unk_0168, 1.0f, 50.0f * (diffSpeedMult * 2));
		}

		this->unk_0168 = 2000.0f;
		if (this->unk_0195 != 0) {
			this->actor.speed = this->unk_01A8 * sGiantModeScaleFactor * 1.5f * diffSpeedMult;
			this->skelAnime.playSpeed = 2.0f * diffSpeedMult;
		}
		else {
			this->actor.speed = this->unk_01A8 * sGiantModeScaleFactor * diffSpeedMult;
			if (!isCutscene) {
				this->skelAnime.playSpeed = 1.0f * diffSpeedMult;
			}
		}

		Actor_UpdateVelocityWithoutGravity(&this->actor);
		Actor_UpdatePos(&this->actor);

		spD0 = this->actor.world.pos;
		if (sIsInGiantMode) {
			spD0.y = 5000.0f;
		}
		else {
			spD0.y = 2000.0f;
		}

		temp_f0 = BgCheck_EntityRaycastFloor1(&play->colCtx, &spDC, &spD0);
		if (((this->unk_017C.y < temp_f0) && (temp_f0 <= this->unk_0188.y)) ||
			((temp_f0 < this->unk_017C.y) && (this->unk_0188.y <= temp_f0))) {
			this->unk_0170 = this->unk_017C;
			this->unk_0170.y = temp_f0;
			this->unk_016C = 120;
			Actor_PlaySfx(&this->actor, NA_SE_EN_INBOSS_ROAR_OLD);
		}

		this->actor.flags &= ~ACTOR_FLAG_ATTENTION_ENABLED;
		if (this->unk_0195 != 0) {
			this->actor.world.rot.z = Math_SinS(this->unk_014C * 0x1200) * 0xE00;
		}
		else {
			this->actor.world.rot.z = Math_SinS(this->unk_014C * 0xC00) * 0xE00;
		}

		this->unk_014E++;
		if (this->unk_014E >= ARRAY_COUNT(this->unk_01BC)) {
			this->unk_014E = 0;
		}

		this->unk_01BC[this->unk_014E].x = this->actor.world.pos.x;
		this->unk_01BC[this->unk_014E].y = this->actor.world.pos.y;
		this->unk_01BC[this->unk_014E].z = this->actor.world.pos.z;

		this->unk_0B1C[this->unk_014E].x = BINANG_TO_RAD_ALT(this->actor.world.rot.x);
		this->unk_0B1C[this->unk_014E].y = BINANG_TO_RAD_ALT(this->actor.world.rot.y);
		this->unk_0B1C[this->unk_014E].z = BINANG_TO_RAD_ALT(this->actor.world.rot.z);
	}

	if ((this->unk_0144 < 10) && (otherTwinmold->unk_0144 >= 20)) {
		this->unk_01B0.y = -1000.0f * sGiantModeScaleFactor;
		if (sIsInGiantMode) {
			this->unk_01B0.y += 3150.0f;
		}
		this->unk_0144 = 3;
		this->unk_0146[0] = 50;
	}

	if ((this->unk_0144 < 10) && !isCutscene) {
		if (otherTwinmold->unk_0144 < 10) {
			myHealth = this->actor.colChkInfo.health;
			otherHealth = otherTwinmold->actor.colChkInfo.health;

			//recomp_printf("Twinmold Health - Current: %d, Other Twinmold: %d, Task : % d\n", myHealth, otherHealth, this->unk_0144);

			distToOther = sqrtf(SQ(this->actor.world.pos.x - otherTwinmold->actor.world.pos.x) + SQ(this->actor.world.pos.z - otherTwinmold->actor.world.pos.z));

			if (this->unk_0144 != 5 && distToOther < (2000.0f * sGiantModeScaleFactor)) {
				pushX = this->actor.world.pos.x - otherTwinmold->actor.world.pos.x;
				pushZ = this->actor.world.pos.z - otherTwinmold->actor.world.pos.z;
				pushDist = sqrtf(SQ(pushX) + SQ(pushZ));
				if (pushDist > 1.0f) {
					this->unk_01B0.x += (pushX / pushDist) * 100.0f * sGiantModeScaleFactor;
					this->unk_01B0.z += (pushZ / pushDist) * 100.0f * sGiantModeScaleFactor;
				}
			}

			if (foreveralone || myHealth > otherHealth || (myHealth == otherHealth && TWINMOLD_GET_TYPE(&this->actor) == TWINMOLD_TYPE_RED)) {
				if (this->unk_0144 != 5) {
					this->unk_0144 = 5;
					this->unk_0146[0] = 150;
				}
			}
			else if (myHealth == otherHealth && TWINMOLD_GET_TYPE(&this->actor) != TWINMOLD_TYPE_RED && this->unk_0144 == 5) {
				this->unk_0144 = 2;
				this->unk_0146[0] = 150;
			}
			else if (myHealth <= otherHealth - 3) {
				if (this->unk_0144 == 5) {
					this->unk_0144 = 2;
					this->unk_01B0.x = -this->actor.world.pos.x;
					this->unk_01B0.z = -this->actor.world.pos.z;
					this->unk_01B0.y = 1500.0f * sGiantModeScaleFactor;
					if (sIsInGiantMode) this->unk_01B0.y += 3150.0f;
				}
			}
			else if (this->unk_0144 != 5 && otherTwinmold->unk_0144 != 5) {
				myDistToPlayer = sqrtf(SQ(this->actor.world.pos.x - player->actor.world.pos.x) + SQ(this->actor.world.pos.z - player->actor.world.pos.z));
				otherDistToPlayer = sqrtf(SQ(otherTwinmold->actor.world.pos.x - player->actor.world.pos.x) + SQ(otherTwinmold->actor.world.pos.z - player->actor.world.pos.z));
				if (myDistToPlayer < otherDistToPlayer) {
					this->unk_0144 = 2;
					this->unk_0146[0] = 150;
				}
			}
		}

		arenaBounds = 4000.0f * sGiantModeScaleFactor;
		if (this->unk_01B0.x > arenaBounds) this->unk_01B0.x = arenaBounds;
		if (this->unk_01B0.x < -arenaBounds) this->unk_01B0.x = -arenaBounds;
		if (this->unk_01B0.z > arenaBounds) this->unk_01B0.z = arenaBounds;
		if (this->unk_01B0.z < -arenaBounds) this->unk_01B0.z = -arenaBounds;
	}

	switch (this->unk_0144) {
	case 0:
		if ((TWINMOLD_GET_TYPE(&this->actor) == TWINMOLD_TYPE_RED) && (Rand_ZeroOne() < 0.75f)) {
			this->actor.world.pos.x = player->actor.world.pos.x;
			this->actor.world.pos.z = player->actor.world.pos.z;
			this->actor.world.pos.y = player->actor.world.pos.y - (600.0f * sGiantModeScaleFactor);
		}
		else {
			this->actor.world.pos.x = Rand_CenteredFloat(5000.0f * sGiantModeScaleFactor);
			this->actor.world.pos.z = Rand_CenteredFloat(5000.0f * sGiantModeScaleFactor);
			this->actor.world.pos.y = -500.0f * sGiantModeScaleFactor;
			if (sIsInGiantMode) {
				this->actor.world.pos.y += 3150.0f;
			}
		}

		if ((fabsf(this->actor.world.pos.x) < (500.0f * sGiantModeScaleFactor)) &&
			(fabsf(this->actor.world.pos.z) < (500.0f * sGiantModeScaleFactor))) {
			return;
		}
		FALLTHROUGH;
	case 100:
		this->actor.shape.rot.x = 0x4000;
		this->unk_01B0.x = this->actor.world.pos.x;
		this->unk_01B0.y = this->actor.world.pos.y + (1000.0f * sGiantModeScaleFactor);
		this->unk_01B0.z = this->actor.world.pos.z;
		this->unk_0146[0] = 100;
		this->unk_0144 = 1;
		FALLTHROUGH;
	case 1:
		if (this->unk_0146[0] == 0) {
			if (Rand_ZeroOne() < 0.3f) {
				this->unk_0144 = 5;
				this->unk_0146[0] = 150;
			}
			else {
				this->unk_0144 = 2;
				sweepX = Math_SinS(this->unk_014C * 0x800) * 3000.0f * sGiantModeScaleFactor;
				sweepZ = Math_CosS(this->unk_014C * 0x600) * 3000.0f * sGiantModeScaleFactor;

				this->unk_01B0.x = player->actor.world.pos.x + sweepX;
				this->unk_01B0.z = player->actor.world.pos.z + sweepZ;

				if ((fabsf(this->unk_01B0.x) < (500.0f * sGiantModeScaleFactor)) &&
					(fabsf(this->unk_01B0.z) < (500.0f * sGiantModeScaleFactor))) {
					this->unk_01B0.x = 500.0f;
					this->unk_01B0.z = 500.0f;
				}
				this->unk_01B0.y = Rand_ZeroFloat(800.0f * sGiantModeScaleFactor) + (200.0f * sGiantModeScaleFactor);
				if (sIsInGiantMode) {
					this->unk_01B0.y += 3150.0f;
				}
			}
			this->unk_0164 = 0.0f;
		}
		return;

	case 2:
		if (this->unk_0195 != 0) {
			phi_f2 = 700.0f;
		}
		else {
			phi_f2 = 500.0f;
		}
		if (sqrtf(SQ(spCC) + SQ(spC8) + SQ(spC4)) < (phi_f2 * sGiantModeScaleFactor)) {
			this->unk_0144 = 3;
			this->unk_01B0.y = -3000.0f * sGiantModeScaleFactor;
			if (sIsInGiantMode) {
				this->unk_01B0.y += 3150.0f;
			}
			this->unk_0146[0] = 150;
			this->unk_0164 = 0.0f;
		}
		return;

	case 3:
		if (this->unk_0146[0] == 0) {
			this->unk_0144 = 0;
		}
		return;

	case 5: {

		if (this->unk_0146[0] > 70) {
			f32 flyOutDist = 3000.0f * sGiantModeScaleFactor;
			this->unk_01B0.x = this->actor.world.pos.x + (Math_SinS(this->actor.world.rot.y) * flyOutDist);
			this->unk_01B0.z = this->actor.world.pos.z + (Math_CosS(this->actor.world.rot.y) * flyOutDist);
			this->unk_01B0.y = player->actor.world.pos.y + (1000.0f * sGiantModeScaleFactor);
		}
		else {
			f32 playerSpeed = sqrtf(SQ(player->actor.velocity.x) + SQ(player->actor.velocity.z));

			if (playerSpeed < 0.2f) {
				this->unk_01B0.x = player->actor.world.pos.x;
				this->unk_01B0.z = player->actor.world.pos.z;
			}
			else {
				this->unk_01B0.x = player->actor.world.pos.x + (player->actor.velocity.x * 20.0f);
				this->unk_01B0.z = player->actor.world.pos.z + (player->actor.velocity.z * 20.0f);
			}

			this->unk_01B0.y = player->actor.world.pos.y + (100.0f * sGiantModeScaleFactor);
		}

		if (this->unk_0146[0] == 0) {
			this->unk_0144 = 3;
			this->unk_01B0.x = Rand_CenteredFloat(500.0f * sGiantModeScaleFactor) + this->actor.world.pos.x;
			this->unk_01B0.y = -3000.0f * sGiantModeScaleFactor;
			if (sIsInGiantMode) {
				this->unk_01B0.y += 3150.0f;
			}
			this->unk_01B0.z = Rand_CenteredFloat(500.0f * sGiantModeScaleFactor) + this->actor.world.pos.z;
			this->unk_0146[0] = 150;
			this->unk_0164 = 0.0f;
		}
		return;
	}

	case 10:
		if (this->unk_1678 != 0) {
			this->unk_019A = 0x500;
			this->unk_01A0 = 0x1200;
		}
		else {
			this->colliderCylinder.dim.radius = 150.0f * sGiantModeScaleFactor;
			this->colliderCylinder.dim.height = 200.0f * sGiantModeScaleFactor;
			this->colliderCylinder.dim.yShift = 0;
			Collider_UpdateCylinder(&this->actor, &this->colliderCylinder);
			CollisionCheck_SetOC(play, &play->colChkCtx, &this->colliderCylinder.base);
			CollisionCheck_SetAC(play, &play->colChkCtx, &this->colliderCylinder.base);
		}
		return;

	case 11:
		if (this == sRedTwinmold) {
			this->unk_01B0.x = D_809DF9C0[this->unk_1D1A].x;
			this->unk_01B0.y = D_809DF9C0[this->unk_1D1A].y;
			this->unk_01B0.z = D_809DF9C0[this->unk_1D1A].z;
		}
		else {
			this->unk_01B0.x = D_809DFA2C[this->unk_1D1A].x;
			this->unk_01B0.y = D_809DFA2C[this->unk_1D1A].y;
			this->unk_01B0.z = D_809DFA2C[this->unk_1D1A].z;
		}
		this->actor.shape.rot.x = 0x4000;
		this->unk_0144 = 12;
		this->actor.world.pos.x = this->unk_01B0.x;
		this->actor.world.pos.y = -200.0f;
		this->actor.world.pos.z = this->unk_01B0.z;
		this->unk_01A8 = 25.0f;
		return;

	case 12:
		if (sqrtf(SQ(spCC) + SQ(spC8) + SQ(spC4)) < 500.0f) {
			this->unk_1D1A++;
			this->unk_0164 = 0.0f;
			if (this->unk_1D1A > 8) {
				this->unk_1D1A = 8;
			}
			if (this == sRedTwinmold) {
				this->unk_01B0.x = D_809DF9C0[this->unk_1D1A].x;
				this->unk_01B0.y = D_809DF9C0[this->unk_1D1A].y;
				this->unk_01B0.z = D_809DF9C0[this->unk_1D1A].z;
			}
			else {
				this->unk_01B0.x = D_809DFA2C[this->unk_1D1A].x;
				this->unk_01B0.y = D_809DFA2C[this->unk_1D1A].y;
				this->unk_01B0.z = D_809DFA2C[this->unk_1D1A].z;
			}
		}
		return;

	case 20:
		this->unk_0152 = 15;
		if ((s16)(BREG(71) + 140) < this->unk_0146[1]) {
			if (this->unk_0146[0] == 0) {
				Matrix_RotateYS(Math_Atan2S(-player->actor.world.pos.x, -player->actor.world.pos.z), MTXMODE_NEW);
				Matrix_MultVecZ(1500.0f * sGiantModeScaleFactor, &spA4);
				this->unk_0146[0] = 50;
				this->unk_01B0.x = player->actor.world.pos.x + spA4.x;
				this->unk_01B0.y =
					Rand_CenteredFloat(500.0f * sGiantModeScaleFactor) + (600.0f * sGiantModeScaleFactor);
				if (sIsInGiantMode) {
					this->unk_01B0.y += 3150.0f;
				}
				this->unk_01B0.z = player->actor.world.pos.z + spA4.z;
			}
			this->unk_0168 = 3000.0f;
		}
		else {
			this->unk_01B0.y += 10.0f * sGiantModeScaleFactor;
			this->unk_0168 = 5000.0f;
		}
		this->unk_019A = 0x1000;
		this->unk_01A8 = 25.0f;
		this->unk_01A0 = 0x1800;
		if (this->unk_0146[1] == 0) {
			this->unk_015C = 1;
			this->unk_0144 = 21;
			this->unk_0146[0] = 20;
			this->unk_0152 = 0;
			sTwinmoldBattleHandler->unk_1D20 = 102;
			sTwinmoldBattleHandler->subCamAtVel = 0.0f;
			Audio_PlaySfx(NA_SE_EN_INBOSS_DEAD_PRE2_OLD);
		}
		else if (!(this->unk_0146[1] & 0xF) && (Rand_ZeroOne() < 0.5f)) {
			Actor_PlaySfx(&this->actor, NA_SE_EN_INBOSS_DAMAGE_OLD);
		}
		return;

	case 21:
		this->unk_01A8 = 0.0f;
		this->actor.speed = 0.0f;
		if (this->unk_0146[0] == 0) {
			this->unk_0146[0] = 3;

			for (i = 0; i < 35; i++) {
				Boss02_SpawnEffectFragment(play->specialEffects, &this->unk_147C[this->unk_1678]);
			}

			Boss02_SpawnEffectFlash(play->specialEffects, &this->unk_147C[this->unk_1678]);
			Audio_PlaySfx(NA_SE_EV_EXPLOSION);

			this->unk_1678--;
			if (this->unk_1678 <= 0) {
				this->unk_0144 = 22;
				this->actor.gravity = -1.0f * sGiantModeScaleFactor;
				this->actor.velocity.y = 0.0f;
				this->actor.terminalVelocity = -1000.0f * sGiantModeScaleFactor;
				this->unk_0164 = Rand_CenteredFloat(0.05f);

				spCC = player->actor.world.pos.x - this->actor.world.pos.x;
				spC4 = player->actor.world.pos.z - this->actor.world.pos.z;
				if (sqrtf(SQ(spCC) + SQ(spC4)) < (400.0f * sGiantModeScaleFactor)) {
					this->actor.speed = 15.0f * sGiantModeScaleFactor;
				}

				spCC = this->actor.world.pos.x;
				spC4 = this->actor.world.pos.z;
				if (sqrtf(SQ(spCC) + SQ(spC4)) < (400.0f * sGiantModeScaleFactor)) {
					this->actor.speed = 15.0f * sGiantModeScaleFactor;
				}

				if (otherTwinmold->unk_0144 >= 10) {
					SEQCMD_PLAY_SEQUENCE(SEQ_PLAYER_BGM_MAIN, 0, NA_BGM_CLEAR_BOSS | SEQ_FLAG_ASYNC);
				}

				Actor_PlaySfx(&this->actor, NA_SE_EN_INBOSS_DEAD_OLD);
			}
		}
		return;

	case 22:
		i = (this->unk_014E + 196) % ARRAY_COUNT(this->unk_01BC);

		Math_Vec3f_Copy(&this->unk_01BC[i], &this->actor.world.pos);
		this->unk_0B1C[i].y += this->unk_0164;
		Math_ApproachF(&this->unk_0B1C[i].x, -(M_PIf / 2), 0.1f, 0.07f);
		Actor_MoveWithGravity(&this->actor);
		Actor_UpdateBgCheckInfo(play, &this->actor, 50.0f, 150.0f, 100.0f, UPDBGCHECKINFO_FLAG_4);

		if (this->actor.bgCheckFlags & BGCHECKFLAG_GROUND) {
			this->unk_0144 = 23;
			this->actor.speed = 0.0f;
			this->unk_0170 = this->unk_017C;
			this->unk_016C = 30;
			this->unk_0170.y = this->actor.floorHeight;
			sTwinmoldBattleHandler->unk_1D20 = 103;
			sTwinmoldBattleHandler->unk_1D1C = 0;
			sTwinmoldBattleHandler->unk_0146[0] = 15;
			sTwinmoldBattleHandler->unk_0150 = 0;
			Audio_PlaySfx(NA_SE_EV_LIGHTNING);

			for (i = 0; i < 30; i++) {
				Boss02_SpawnEffectFragment(play->specialEffects, &this->unk_0170);
			}

			this->unk_0146[0] = 10;
		}
		break;

	case 23:
		i = (this->unk_014E + 196) % ARRAY_COUNT(this->unk_01BC);
		Math_Vec3f_Copy(&this->unk_01BC[i], &this->actor.world.pos);
		Math_ApproachF(&this->unk_0B1C[i].x, -(M_PIf / 2), 0.05f, 0.07f);

		if (this->unk_0146[0] & 1) {
			sp9C = Rand_ZeroFloat(M_PIf);

			for (i = 0; i < 15; i++) {
				Matrix_RotateYF(((2.0f * (i * M_PIf)) / 15.0f) + sp9C, MTXMODE_NEW);
				Matrix_MultVecZ((10 - this->unk_0146[0]) * (sGiantModeScaleFactor * 300.0f) * 0.1f, &sp90);
				spD0.x = this->unk_0170.x + sp90.x;
				spD0.y = this->unk_0170.y + (1000.0f * sGiantModeScaleFactor);
				spD0.z = this->unk_0170.z + sp90.z;
				if (BgCheck_EntityRaycastFloor3(&play->colCtx, &sp8C, &bgId, &spD0) != BGCHECK_Y_MIN) {
					spA0 = BgCheck_EntityRaycastFloor1(&play->colCtx, &sp8C, &spD0);
					Matrix_MultVecZ(5.0f * sGiantModeScaleFactor, &sp70);
					sp70.y = 2.0f * sGiantModeScaleFactor;
					sp64.y = 0.3f * sGiantModeScaleFactor;
					sp64.z = 0.0f;
					sp64.x = 0.0f;
					sp7C.x = spD0.x;
					sp7C.y = spA0;
					sp7C.z = spD0.z;
					func_800B0F18(play, &sp7C, &sp70, &sp64, &D_809DFA98, &D_809DFA98,
						(Rand_ZeroFloat(500.0f) + 1200.0f) * sGiantModeScaleFactor,
						20.0f * sGiantModeScaleFactor, Rand_ZeroFloat(5.0f) + 14.0f);
				}
			}
		}
		break;

	default:
		return;
	}

	this->colliderCylinder.dim.radius = 150.0f * sGiantModeScaleFactor;
	this->colliderCylinder.dim.height = 200.0f * sGiantModeScaleFactor;
	this->colliderCylinder.dim.yShift = 0;
	Collider_UpdateCylinder(&this->actor, &this->colliderCylinder);
	CollisionCheck_SetAT(play, &play->colChkCtx, &this->colliderCylinder.base);
}

RECOMP_HOOK("Boss02_Twinmold_Update") void TwinUpdate(Actor* thisx, PlayState* play) {

	Boss02* this = (Boss02*)thisx;
    sCanSkipMaskOnCs = true;
	if (this->actor.colChkInfo.damage < 3) this->actor.colChkInfo.damage = 3;
	// winning
}