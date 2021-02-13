//-------------------------------------------------------------------------
/*
Copyright (C) 2010-2019 EDuke32 developers and contributors
Copyright (C) 2019 Nuke.YKT

This file is part of NBlood.

NBlood is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License version 2
as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
//-------------------------------------------------------------------------
#include "ns.h"	// Must come before everything else!

#include "build.h"
#include "compat.h"
#include "blood.h"

BEGIN_BLD_NS

int mirrorcnt, mirrorsector, mirrorwall[4];

typedef struct
{
    int type;
    int link;
    int dx;
    int dy;
    int dz;
    int wallnum;
} MIRROR;

MIRROR mirror[16];

void InitMirrors(void)
{
    r_rortexture = 4080;
    r_rortexturerange = 16;

    mirrorcnt = 0;
    tileDelete(504);
    
	for (int i = 0; i < 16; i++)
	{
		tileDelete(4080 + i);
	}
    for (int i = numwalls - 1; i >= 0; i--)
    {
        if (mirrorcnt == 16)
            break;
        int nTile = 4080+mirrorcnt;
        if (wall[i].overpicnum == 504)
        {
            if (wall[i].extra > 0 && GetWallType(i) == kWallStack)
            {
                wall[i].overpicnum = nTile;
                mirror[mirrorcnt].wallnum = i;
                mirror[mirrorcnt].type = 0;
                wall[i].cstat |= 32;
                int tmp = xwall[wall[i].extra].data;
                int j;
                for (j = numwalls - 1; j >= 0; j--)
                {
                    if (j == i)
                        continue;
                    if (wall[j].extra > 0 && GetWallType(i) == kWallStack)
                    {
                        if (tmp != xwall[wall[j].extra].data)
                            continue;
                        wall[i].hitag = j;
                        wall[j].hitag = i;
                        mirror[mirrorcnt].link = j;
                        break;
                    }
                }
                if (j < 0)
                    I_Error("wall[%d] has no matching wall link! (data=%d)\n", i, tmp);
                mirrorcnt++;
            }
            continue;
        }
        if (wall[i].picnum == 504)
        {
            mirror[mirrorcnt].link = i;
            mirror[mirrorcnt].wallnum = i;
            wall[i].picnum = nTile;
            mirror[mirrorcnt].type = 0;
            wall[i].cstat |= 32;
            mirrorcnt++;
            continue;
        }
    }
    for (int i = numsectors - 1; i >= 0; i--)
    {
        if (mirrorcnt >= 15)
            break;

        if (sector[i].floorpicnum == 504)
        {
            int nLink = gUpperLink[i];
            if (nLink < 0)
                continue;
            int nLink2 = sprite[nLink].owner & 0xfff;
            int j = sprite[nLink2].sectnum;
            if (sector[j].ceilingpicnum != 504)
                I_Error("Lower link sector %d doesn't have mirror picnum\n", j);
            mirror[mirrorcnt].type = 2;
            mirror[mirrorcnt].dx = sprite[nLink2].x-sprite[nLink].x;
            mirror[mirrorcnt].dy = sprite[nLink2].y-sprite[nLink].y;
            mirror[mirrorcnt].dz = sprite[nLink2].z-sprite[nLink].z;
            mirror[mirrorcnt].wallnum = i;
            mirror[mirrorcnt].link = j;
            sector[i].floorpicnum = 4080+mirrorcnt;
            mirrorcnt++;
            mirror[mirrorcnt].type = 1;
            mirror[mirrorcnt].dx = sprite[nLink].x-sprite[nLink2].x;
            mirror[mirrorcnt].dy = sprite[nLink].y-sprite[nLink2].y;
            mirror[mirrorcnt].dz = sprite[nLink].z-sprite[nLink2].z;
            mirror[mirrorcnt].wallnum = j;
            mirror[mirrorcnt].link = i;
            sector[j].ceilingpicnum = 4080+mirrorcnt;
            mirrorcnt++;
        }
    }
    mirrorsector = numsectors;
    for (int i = 0; i < 4; i++)
    {
        mirrorwall[i] = numwalls+i;
        wall[mirrorwall[i]].picnum = 504;
        wall[mirrorwall[i]].overpicnum = 504;
        wall[mirrorwall[i]].cstat = 0;
        wall[mirrorwall[i]].nextsector = -1;
        wall[mirrorwall[i]].nextwall = -1;
        wall[mirrorwall[i]].point2 = numwalls+i+1;
    }
    wall[mirrorwall[3]].point2 = mirrorwall[0];
    sector[mirrorsector].ceilingpicnum = 504;
    sector[mirrorsector].floorpicnum = 504;
    sector[mirrorsector].wallptr = mirrorwall[0];
    sector[mirrorsector].wallnum = 4;
}

void TranslateMirrorColors(int nShade, int nPalette)
{
}

void sub_5571C(char mode)
{
    for (int i = mirrorcnt-1; i >= 0; i--)
    {
        int nTile = 4080+i;
        if (TestBitString(gotpic, nTile))
        {
            switch (mirror[i].type)
            {
                case 1:
                    if (mode)
                        sector[mirror[i].wallnum].ceilingstat |= 1;
                    else
                        sector[mirror[i].wallnum].ceilingstat &= ~1;
                    break;
                case 2:
                    if (mode)
                        sector[mirror[i].wallnum].floorstat |= 1;
                    else
                        sector[mirror[i].wallnum].floorstat &= ~1;
                    break;
            }
        }
    }
}

void sub_557C4(int x, int y, int interpolation)
{
    if (spritesortcnt == 0) return;
    int nViewSprites = spritesortcnt-1;
    for (int nTSprite = nViewSprites; nTSprite >= 0; nTSprite--)
    {
        tspritetype *pTSprite = &tsprite[nTSprite];
        pTSprite->xrepeat = pTSprite->yrepeat = 0;
    }
    for (int i = mirrorcnt-1; i >= 0; i--)
    {
        int nTile = 4080+i;
        if (TestBitString(gotpic, nTile))
        {
            if (mirror[i].type == 1 || mirror[i].type == 2)
            {
                int nSector = mirror[i].link;
                int nSector2 = mirror[i].wallnum;
                int nSprite;
                SectIterator it(nSector);
                while ((nSprite = it.NextIndex()) >= 0)
                {
                    spritetype *pSprite = &sprite[nSprite];
                    if (pSprite == gView->pSprite)
                        continue;
                    int top, bottom;
                    GetSpriteExtents(pSprite, &top, &bottom);
                    int zCeil, zFloor;
                    getzsofslope(nSector, pSprite->x, pSprite->y, &zCeil, &zFloor);
                    if (pSprite->statnum == kStatDude && (top < zCeil || bottom > zFloor))
                    {
                        int j = i;
                        if (mirror[i].type == 2)
                            j++;
                        else
                            j--;
                        int dx = mirror[j].dx;
                        int dy = mirror[j].dy;
                        int dz = mirror[j].dz;
                        tspritetype *pTSprite = &tsprite[spritesortcnt];
                        memset(pTSprite, 0, sizeof(tspritetype));
                        pTSprite->type = pSprite->type;
                        pTSprite->index = pSprite->index;
                        pTSprite->sectnum = nSector2;
                        pTSprite->x = pSprite->x+dx;
                        pTSprite->y = pSprite->y+dy;
                        pTSprite->z = pSprite->z+dz;
                        pTSprite->ang = pSprite->ang;
                        pTSprite->picnum = pSprite->picnum;
                        pTSprite->shade = pSprite->shade;
                        pTSprite->pal = pSprite->pal;
                        pTSprite->xrepeat = pSprite->xrepeat;
                        pTSprite->yrepeat = pSprite->yrepeat;
                        pTSprite->xoffset = pSprite->xoffset;
                        pTSprite->yoffset = pSprite->yoffset;
                        pTSprite->cstat = pSprite->cstat;
                        pTSprite->statnum = kStatDecoration;
                        pTSprite->owner = pSprite->index;
                        pTSprite->extra = pSprite->extra;
                        pTSprite->flags = pSprite->hitag|0x200;
                        pTSprite->x = dx+interpolate(pSprite->ox, pSprite->x, interpolation);
                        pTSprite->y = dy+interpolate(pSprite->oy, pSprite->y, interpolation);
                        pTSprite->z = dz+interpolate(pSprite->oz, pSprite->z, interpolation);
                        pTSprite->ang = pSprite->interpolatedang(interpolation);
                        spritesortcnt++;
                    }
                }
            }
        }
    }
    for (int nTSprite = spritesortcnt-1; nTSprite >= nViewSprites; nTSprite--)
    {
        tspritetype *pTSprite = &tsprite[nTSprite];
        int nAnim = 0;
        switch (picanm[pTSprite->picnum].extra&7)
        {
        case 1:
        {
            int dX = x - pTSprite->x;
            int dY = y - pTSprite->y;
            RotateVector(&dX, &dY, 128 - pTSprite->ang);
            nAnim = GetOctant(dX, dY);
            if (nAnim <= 4)
            {
                pTSprite->cstat &= ~4;
            }
            else
            {
                nAnim = 8 - nAnim;
                pTSprite->cstat |= 4;
            }
            break;
        }
        case 2:
        {
            int dX = x - pTSprite->x;
            int dY = y - pTSprite->y;
            RotateVector(&dX, &dY, 128 - pTSprite->ang);
            nAnim = GetOctant(dX, dY);
            break;
        }
        }
        while (nAnim > 0)
        {
            pTSprite->picnum += picanm[pTSprite->picnum].num+1;
            nAnim--;
        }
    }
}

void DrawMirrors(int x, int y, int z, fixed_t a, fixed_t horiz, int smooth, int viewPlayer)
{
    for (int i = mirrorcnt - 1; i >= 0; i--)
    {
        int nTile = 4080+i;
        if (TestBitString(gotpic, nTile))
        {
            ClearBitString(gotpic, nTile);
            switch (mirror[i].type)
            {
            case 0:
            {
                int nWall = mirror[i].link;
                int nSector = sectorofwall(nWall);
                walltype *pWall = &wall[nWall];
                int nNextWall = pWall->nextwall;
                int nNextSector = pWall->nextsector;
                pWall->nextwall = mirrorwall[0];
                pWall->nextsector = mirrorsector;
                wall[mirrorwall[0]].nextwall = nWall;
                wall[mirrorwall[0]].nextsector = nSector;
                wall[mirrorwall[0]].x = wall[pWall->point2].x;
                wall[mirrorwall[0]].y = wall[pWall->point2].y;
                wall[mirrorwall[1]].x = pWall->x;
                wall[mirrorwall[1]].y = pWall->y;
                wall[mirrorwall[2]].x = wall[mirrorwall[1]].x+(wall[mirrorwall[1]].x-wall[mirrorwall[0]].x)*16;
                wall[mirrorwall[2]].y = wall[mirrorwall[1]].y+(wall[mirrorwall[1]].y-wall[mirrorwall[0]].y)*16;
                wall[mirrorwall[3]].x = wall[mirrorwall[0]].x+(wall[mirrorwall[0]].x-wall[mirrorwall[1]].x)*16;
                wall[mirrorwall[3]].y = wall[mirrorwall[0]].y+(wall[mirrorwall[0]].y-wall[mirrorwall[1]].y)*16;
                sector[mirrorsector].floorz = sector[nSector].floorz;
                sector[mirrorsector].ceilingz = sector[nSector].ceilingz;
                int cx, cy, ca;
                if (GetWallType(nWall) == kWallStack)
                {
                     cx = x - (wall[pWall->hitag].x-wall[pWall->point2].x);
                     cy = y - (wall[pWall->hitag].y-wall[pWall->point2].y);
                     ca = a;
                }
                else
                {
                    renderPrepareMirror(x,y,z,a,horiz,nWall,&cx,&cy,&ca);
                }
                int32_t didmirror = renderDrawRoomsQ16(cx, cy, z, ca,horiz,mirrorsector|MAXSECTORS);
                viewProcessSprites(cx,cy,z,FixedToInt(ca),smooth);
                renderDrawMasks();
                if (GetWallType(nWall) != kWallStack)
                    renderCompleteMirror();
                if (wall[nWall].pal != 0 || wall[nWall].shade != 0)
                    TranslateMirrorColors(wall[nWall].shade, wall[nWall].pal);
                pWall->nextwall = nNextWall;
                pWall->nextsector = nNextSector;
                return;
            }
            case 1:
            {
                r_rorphase = 1;
                int nSector = mirror[i].link;
                int bakCstat;
                if (viewPlayer >= 0)
                {
                    bakCstat = gPlayer[viewPlayer].pSprite->cstat;
                    if (gViewPos == 0)
                    {
                        gPlayer[viewPlayer].pSprite->cstat |= 32768;
                    }
                    else
                    {
                        gPlayer[viewPlayer].pSprite->cstat |= 514;
                    }
                }
                renderDrawRoomsQ16(x+mirror[i].dx, y+mirror[i].dy, z+mirror[i].dz, a, horiz, nSector|MAXSECTORS);
                viewProcessSprites(x+mirror[i].dx, y+mirror[i].dy, z+mirror[i].dz, FixedToInt(a), smooth);
                short fstat = sector[nSector].floorstat;
                sector[nSector].floorstat |= 1;
                renderDrawMasks();
                sector[nSector].floorstat = fstat;
                for (int i = 0; i < 16; i++)
                    ClearBitString(gotpic, 4080+i);
                if (viewPlayer >= 0)
                {
                    gPlayer[viewPlayer].pSprite->cstat = bakCstat;
                }
                r_rorphase = 0;
                return;
            }
            case 2:
            {
                r_rorphase = 1;
                int nSector = mirror[i].link;
                int bakCstat;
                if (viewPlayer >= 0)
                {
                    bakCstat = gPlayer[viewPlayer].pSprite->cstat;
                    if (gViewPos == 0)
                    {
                        gPlayer[viewPlayer].pSprite->cstat |= 32768;
                    }
                    else
                    {
                        gPlayer[viewPlayer].pSprite->cstat |= 514;
                    }
                }
                renderDrawRoomsQ16(x+mirror[i].dx, y+mirror[i].dy, z+mirror[i].dz, a, horiz, nSector|MAXSECTORS);
                viewProcessSprites(x+mirror[i].dx, y+mirror[i].dy, z+mirror[i].dz, FixedToInt(a), smooth);
                short cstat = sector[nSector].ceilingstat;
                sector[nSector].ceilingstat |= 1;
                renderDrawMasks();
                sector[nSector].ceilingstat = cstat;
                for (int i = 0; i < 16; i++)
                    ClearBitString(gotpic, 4080+i);
                if (viewPlayer >= 0)
                {
                    gPlayer[viewPlayer].pSprite->cstat = bakCstat;
                }
                r_rorphase = 0;
                return;
            }
            }
        }
    }
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

FSerializer& Serialize(FSerializer& arc, const char* keyname, MIRROR& w, MIRROR* def)
{
	if (arc.BeginObject(keyname))
	{
		arc ("type", w.type)
			("link", w.link)
			("dx", w.dx)
			("dy", w.dy)
			("dz", w.dz)
			("wallnum", w.wallnum)
			.EndObject();
	}
	return arc;
}

void SerializeMirrors(FSerializer& arc)
{
	if (arc.BeginObject("mirror"))
	{
		arc("mirrorcnt", mirrorcnt)
			("mirrorsector", mirrorsector)
			.Array("mirror", mirror, countof(mirror))
			.Array("mirrorwall", mirrorwall, countof(mirrorwall))
			.EndObject();
	}

	if (arc.isReading())
	{

		tileDelete(504);

#ifdef USE_OPENGL
		r_rortexture = 4080;
		r_rortexturerange = 16;

#endif

		for (int i = 0; i < 16; i++)
		{
			tileDelete(4080 + i);
		}
		for (int i = 0; i < 4; i++)
		{
			wall[mirrorwall[i]].picnum = 504;
			wall[mirrorwall[i]].overpicnum = 504;
			wall[mirrorwall[i]].cstat = 0;
			wall[mirrorwall[i]].nextsector = -1;
			wall[mirrorwall[i]].nextwall = -1;
			wall[mirrorwall[i]].point2 = numwalls + i + 1;
		}
		wall[mirrorwall[3]].point2 = mirrorwall[0];
		sector[mirrorsector].ceilingpicnum = 504;
		sector[mirrorsector].floorpicnum = 504;
		sector[mirrorsector].wallptr = mirrorwall[0];
		sector[mirrorsector].wallnum = 4;
	}
}

END_BLD_NS
