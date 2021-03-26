//-------------------------------------------------------------------------
/*
Copyright (C) 1996, 2003 - 3D Realms Entertainment

This file is part of Duke Nukem 3D version 1.5 - Atomic Edition

Duke Nukem 3D is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

Original Source: 1996 - Todd Replogle
Prepared for public release: 03/21/2003 - Charlie Wiederhold, 3D Realms
Modifications for JonoF's port by Jonathon Fowler (jf@jonof.id.au)
*/
//-------------------------------------------------------------------------

#include "ns.h"
#include "duke3d.h"
#include "build.h"
#include "v_video.h"
#include "prediction.h"
#include "automap.h"
#include "dukeactor.h"
#include "interpolate.h"
#include "render.h"

#include "_polymost.cpp"

// temporary hack to pass along RRRA's global fog. Needs to be done better later.
extern PalEntry GlobalMapFog;
extern float GlobalFogDensity;

EXTERN_CVAR(Bool, testnewrenderer)

BEGIN_DUKE_NS

//---------------------------------------------------------------------------
//
// Floor Over Floor

// If standing in sector with SE42
// then draw viewing to SE41 and raise all =hi SE43 cielings.

// If standing in sector with SE43
// then draw viewing to SE40 and lower all =hi SE42 floors.

// If standing in sector with SE44
// then draw viewing to SE40.

// If standing in sector with SE45
// then draw viewing to SE41.
//
//---------------------------------------------------------------------------

/*static*/ int tempsectorz[MAXSECTORS];
/*static*/ int tempsectorpicnum[MAXSECTORS];
//short tempcursectnum;

void renderView(spritetype* playersprite, int sectnum, int x, int y, int z, binangle a, fixedhoriz h, lookangle rotscrnang, int smoothratio)
{
	if (!testnewrenderer)
	{
		// do screen rotation.
		renderSetRollAngle(rotscrnang.asbuildf());

		se40code(x, y, z, a, h, smoothratio);
		renderMirror(x, y, z, a, h, smoothratio);
		renderDrawRoomsQ16(x, y, z - (4 << 8), a.asq16(), h.asq16(), sectnum);
		fi.animatesprites(x, y, a.asbuild(), smoothratio);
		renderDrawMasks();
	}
	else
	{
		render_drawrooms(playersprite, { x, y, z }, sectnum, a, h, rotscrnang);
	}
}

//---------------------------------------------------------------------------
//
// 
//
//---------------------------------------------------------------------------

void animatecamsprite(double smoothratio)
{
	const int VIEWSCREEN_ACTIVE_DISTANCE = 8192;

	if (camsprite == nullptr)
		return;

	auto p = &ps[screenpeek];
	auto sp = &camsprite->s;

	if (p->newOwner != nullptr) camsprite->SetOwner(p->newOwner);

	if (camsprite->GetOwner() && dist(p->GetActor(), camsprite) < VIEWSCREEN_ACTIVE_DISTANCE)
	{
		auto tex = tileGetTexture(sp->picnum);
		TileFiles.MakeCanvas(TILE_VIEWSCR, tex->GetDisplayWidth(), tex->GetDisplayHeight());

		auto canvas = renderSetTarget(TILE_VIEWSCR);
		if (!canvas) return;

		screen->RenderTextureView(canvas, [=](IntRect& rect)
			{
				auto camera = &camsprite->GetOwner()->s;
				auto ang = buildang(camera->interpolatedang(smoothratio));
				display_mirror = 1; // should really be 'display external view'.
				if (!testnewrenderer)
				{
					// Note: no ROR or camera here - Polymost has no means to detect these things before rendering the scene itself.
					renderDrawRoomsQ16(camera->x, camera->y, camera->z, ang.asq16(), IntToFixed(camera->shade), camera->sectnum); // why 'shade'...?
					fi.animatesprites(camera->x, camera->y, ang.asbuild(), smoothratio);
				}
				else
				{
					render_drawrooms(camera, camera->pos, camera->sectnum, ang, buildhoriz(camera->shade), buildlook(0));
				}
				display_mirror = 0;
				renderDrawMasks();
			});
		renderRestoreTarget();
	}
}

//---------------------------------------------------------------------------
//
// RRRA's drug distortion effect
//
//---------------------------------------------------------------------------
int DrugTimer;

static int getdrugmode(player_struct *p, int oyrepeat)
{
	int now = I_GetBuildTime() >> 1;	// this function works on a 60 fps setup.
	if (playrunning() && p->DrugMode > 0)
	{
		if (now - DrugTimer > 4 || now - DrugTimer < 0) DrugTimer = now - 1;
		while (DrugTimer < now)
		{
			DrugTimer++;
			int var_8c;
			if (p->drug_stat[0] == 0)
			{
				p->drug_stat[1]++;
				var_8c = oyrepeat + p->drug_stat[1] * 5000;
				if (oyrepeat * 3 < var_8c)
				{
					p->drug_aspect = oyrepeat * 3;
					p->drug_stat[0] = 2;
				}
				else
				{
					p->drug_aspect = var_8c;
				}
			}
			else if (p->drug_stat[0] == 3)
			{
				p->drug_stat[1]--;
				var_8c = oyrepeat + p->drug_stat[1] * 5000;
				if (var_8c < oyrepeat)
				{
					p->DrugMode = 0;
					p->drug_stat[0] = 0;
					p->drug_stat[2] = 0;
					p->drug_stat[1] = 0;
					p->drug_aspect = oyrepeat;
				}
				else
				{
					p->drug_aspect = var_8c;
				}
			}
			else if (p->drug_stat[0] == 2)
			{
				if (p->drug_stat[2] > 30)
				{
					p->drug_stat[0] = 1;
				}
				else
				{
					p->drug_stat[2]++;
					p->drug_aspect = oyrepeat * 3 + p->drug_stat[2] * 500;
				}
			}
			else
			{
				if (p->drug_stat[2] < 1)
				{
					p->drug_stat[0] = 2;
					p->DrugMode--;
					if (p->DrugMode == 1)
						p->drug_stat[0] = 3;
				}
				else
				{
					p->drug_stat[2]--;
					p->drug_aspect = oyrepeat * 3 + p->drug_stat[2] * 500;
				}
			}
		}
		return p->drug_aspect;
	}
	else
	{
		DrugTimer = now;
		return oyrepeat;
	}
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

void displayrooms(int snum, double smoothratio)
{
	int cposx, cposy, cposz, fz, cz;
	short sect;
	binangle cang;
	lookangle rotscrnang;
	fixedhoriz choriz;
	struct player_struct* p;
	int tiltcs = 0; // JBF 20030807

	p = &ps[snum];

	if (automapMode == am_full || p->cursectnum == -1)
		return;

	// Do not light up the fog in RRRA's E2L1. Ideally this should apply to all foggy levels but all others use lookup table hacks for their fog.
	if (isRRRA() && fogactive)
	{
		p->visibility = ud.const_visibility;
	}

	g_visibility = p->visibility;

	videoSetCorrectedAspect();

	sect = p->cursectnum;
	if (sect < 0 || sect >= MAXSECTORS) return;

	GlobalMapFog = fogactive ? 0x999999 : 0;
	GlobalFogDensity = fogactive ? 350.f : 0.f;
	GLInterface.SetMapFog(fogactive != 0);
	DoInterpolations(smoothratio / 65536.);

	setgamepalette(BASEPAL);
	animatecamsprite(smoothratio);

	if (ud.cameraactor)
	{
		spritetype* s;

		s = &ud.cameraactor->s;

		if (s->yvel < 0) s->yvel = -100;
		else if (s->yvel > 199) s->yvel = 300;

		cang = buildfang(ud.cameraactor->tempang + MulScaleF(((s->ang + 1024 - ud.cameraactor->tempang) & 2047) - 1024, smoothratio, 16));

		auto bh = buildhoriz(s->yvel);
		renderView(s, s->sectnum, s->x, s->y, s->z, cang, bh, buildlook(0), smoothratio);
	}
	else
	{
		// Fixme: This should get the aspect ratio from the backend, not the current viewport size.
		int i = DivScale(1, isRR() ? 64 : p->GetActor()->s.yrepeat + 28, 22);
		int viewingaspect = !isRRRA() || !p->DrugMode ? xs_CRoundToInt(double(i) * tan(r_fov * (pi::pi() / 360.))) : getdrugmode(p, i);
		renderSetAspect(MulScale(viewingaspect, viewingrange, 16), yxaspect);

		// The camera texture must be rendered with the base palette, so this is the only place where the current global palette can be set.
		// The setting here will be carried over to the rendering of the weapon sprites, but other 2D content will always default to the main palette.
		setgamepalette(setpal(p));

		// set screen rotation.
		rotscrnang = !SyncInput() ? p->angle.rotscrnang : p->angle.interpolatedrotscrn(smoothratio);

		if ((snum == myconnectindex) && (numplayers > 1))
		{
			cposx = omyx + xs_CRoundToInt(MulScaleF(myx - omyx, smoothratio, 16));
			cposy = omyy + xs_CRoundToInt(MulScaleF(myy - omyy, smoothratio, 16));
			cposz = omyz + xs_CRoundToInt(MulScaleF(myz - omyz, smoothratio, 16));
			if (SyncInput())
			{
				fixed_t ohorz = (omyhoriz + omyhorizoff).asq16();
				fixed_t horz = (myhoriz + myhorizoff).asq16();
				choriz = q16horiz(ohorz + xs_CRoundToInt(MulScaleF(horz - ohorz, smoothratio, 16)));
				cang = bamang(xs_CRoundToUInt(omyang.asbam() + MulScaleF((myang - omyang).asbam(), smoothratio, 16)));
			}
			else
			{
				cang = myang;
				choriz = myhoriz + myhorizoff;
			}
			sect = mycursectnum;
		}
		else
		{
			cposx = p->oposx + xs_CRoundToInt(MulScaleF(p->posx - p->oposx, smoothratio, 16));
			cposy = p->oposy + xs_CRoundToInt(MulScaleF(p->posy - p->oposy, smoothratio, 16));
			cposz = p->oposz + xs_CRoundToInt(MulScaleF(p->posz - p->oposz, smoothratio, 16));
			if (SyncInput())
			{
				// Original code for when the values are passed through the sync struct
				choriz = p->horizon.interpolatedsum(smoothratio);
				cang = p->angle.interpolatedsum(smoothratio);
			}
			else
			{
				// This is for real time updating of the view direction.
				cang = p->angle.sum();
				choriz = p->horizon.sum();
			}
		}

		spritetype* viewer;
		if (p->newOwner != nullptr)
		{
			auto spr = &p->newOwner->s;
			cang = buildang(spr->interpolatedang(smoothratio));
			choriz = buildhoriz(spr->shade);
			cposx = spr->pos.x;
			cposy = spr->pos.y;
			cposz = spr->pos.z;
			sect = spr->sectnum;
			rotscrnang = buildlook(0);
			smoothratio = MaxSmoothRatio;
			viewer = spr;
		}
		else if (p->over_shoulder_on == 0)
		{
			if (cl_viewbob) cposz += p->opyoff + xs_CRoundToInt(MulScaleF(p->pyoff - p->opyoff, smoothratio, 16));
			viewer = &p->GetActor()->s;
		}
		else
		{
			cposz -= isRR() ? 3840 : 3072;

			if (!calcChaseCamPos(&cposx, &cposy, &cposz, &p->GetActor()->s, &sect, cang, choriz, smoothratio))
			{
				cposz += isRR() ? 3840 : 3072;
				calcChaseCamPos(&cposx, &cposy, &cposz, &p->GetActor()->s, &sect, cang, choriz, smoothratio);
			}
			viewer = &p->GetActor()->s;
		}

		cz = p->GetActor()->ceilingz;
		fz = p->GetActor()->floorz;

		if (earthquaketime > 0 && p->on_ground == 1)
		{
			cposz += 256 - (((earthquaketime) & 1) << 9);
			cang += buildang((2 - ((earthquaketime) & 2)) << 2);
		}

		if (p->GetActor()->s.pal == 1) cposz -= (18 << 8);

		else if (p->spritebridge == 0 && p->newOwner == nullptr)
		{
			if (cposz < (p->truecz + (4 << 8))) cposz = cz + (4 << 8);
			else if (cposz > (p->truefz - (4 << 8))) cposz = fz - (4 << 8);
		}

		if (sect >= 0)
		{
			getzsofslope(sect, cposx, cposy, &cz, &fz);
			if (cposz < cz + (4 << 8)) cposz = cz + (4 << 8);
			if (cposz > fz - (4 << 8)) cposz = fz - (4 << 8);
		}

		choriz = clamp(choriz, q16horiz(gi->playerHorizMin()), q16horiz(gi->playerHorizMax()));

		if (isRR() && sector[sect].lotag == 848 && !testnewrenderer)
		{
			renderSetRollAngle(rotscrnang.asbuildf());
			geometryEffect(cposx, cposy, cposz, cang, choriz, sect, smoothratio);
		}
		else
		{
			renderView(viewer, sect, cposx, cposy, cposz, cang, choriz, rotscrnang, smoothratio);
		}
	}
	//GLInterface.SetMapFog(false);
	RestoreInterpolations();

	if (!isRRRA() || !fogactive)
	{
		if (PlayClock < lastvisinc)
		{
			if (abs(p->visibility - ud.const_visibility) > 8)
				p->visibility += (ud.const_visibility - p->visibility) >> 2;
		}
		else p->visibility = ud.const_visibility;
	}
}

bool GameInterface::GenerateSavePic()
{
	displayrooms(myconnectindex, MaxSmoothRatio);
	return true;
}

void GameInterface::processSprites(int viewx, int viewy, int viewz, binangle viewang, double smoothRatio)
{
	fi.animatesprites(viewx, viewy, viewz, int(smoothRatio));
}


END_DUKE_NS
