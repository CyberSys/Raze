#include "ns.h"
#include "wh.h"
#include "gameinput.h"
#include "inputstate.h"
#include "menu.h"
#include "gamestate.h"

BEGIN_WH_NS 

int ihaveflag;
int justplayed;
int lopoint;
int walktoggle;
int runningtime;
double oldhoriz;


static int osdcmd_third_person_view(CCmdFuncPtr parm)
{
	if (gamestate != GS_LEVEL) return CCMD_OK;
	/*	if (gViewPos > VIEWPOS_0)
		gViewPos = VIEWPOS_0;
	else
		gViewPos = VIEWPOS_1;
		*/
	return CCMD_OK;
}

static int osdcmd_coop_view(CCmdFuncPtr parm)
{
	if (gamestate != GS_LEVEL) return CCMD_OK;

	if (numplayers > 1)
	{
		pyrn = connectpoint2[pyrn];
		if (pyrn < 0) pyrn = connecthead;
	}
	return CCMD_OK;
}

static int osdcmd_cast_spell(CCmdFuncPtr parm)
{
	// There's no more space in the packet so this gets sent as a separate network message.
	if (gamestate != GS_LEVEL) return CCMD_OK;
	if (parm->numparms == 0) return CCMD_SHOWHELP;
	uint8_t spellnum = (uint8_t)strtoll(parm->parms[0], nullptr, 10);
	Net_WriteByte(DEM_SPELL);
	Net_WriteByte(spellnum);

	if (numplayers > 1)
	{
		pyrn = connectpoint2[pyrn];
		if (pyrn < 0) pyrn = connecthead;
	}
	return CCMD_OK;
}

void cast_spell(int plr, uint8_t** stream, bool skip)
{
	int cheat = ReadByte(stream);
	if (skip) return;
	player[plr].spellnum = cheat;
}


int32_t registerosdcommands(void)
{
	//C_RegisterFunction("warptocoords", "warptocoords [x] [y] [z] [ang] (optional) [horiz] (optional): warps the player to the specified coordinates", osdcmd_warptocoords);
	C_RegisterFunction("third_person_view", "Switch to third person view", osdcmd_third_person_view);
	C_RegisterFunction("coop_view", "Switch player to view from in coop", osdcmd_coop_view);
	C_RegisterFunction("cast_spell", "castspell <spellnum> Cast a spell", osdcmd_cast_spell);
	//C_RegisterFunction("show_weapon", "Show opponents' weapons", osdcmd_show_weapon);
	Net_SetCommandHandler(DEM_SPELL, cast_spell);
	return 0;
}


/*
	loc.bits = ctrlGetInputKey(GameKeys.Jump, false) ? CtrlJump : 0; = SB_JUMP
	loc.bits |= ctrlGetInputKey(GameKeys.Crouch, false) ? CtrlCrouch : 0; = SB_CROUCH
	loc.bits |= ctrlGetInputKey(GameKeys.Weapon_Fire, false) ? CtrlWeapon_Fire : 0; = SB_FIRE
	loc.bits |= ctrlGetInputKey(WhKeys.Aim_Up, false) ? CtrlAim_Up : 0; = SB_AIM_UP
	loc.bits |= ctrlGetInputKey(WhKeys.Aim_Down, false) ? CtrlAim_Down : 0; = SB_AIM_DOWN
	loc.bits |= ctrlGetInputKey(WhKeys.Aim_Center, true) ? CtrlAim_Center : 0; = SB_CENTERVIEW
	loc.bits |= ctrlGetInputKey(GameKeys.Run, false) ? CtrlRun : 0; = SB_RUN
	if (ctrlGetInputKey(WhKeys.Inventory_Left, true)) loc.bits |= (6) << 16; = SB_INVPREV
	if (ctrlGetInputKey(WhKeys.Inventory_Right, true)) loc.bits |= (7) << 16; = SB_INVNEXT
	if (ctrlGetInputKey(GameKeys.Previous_Weapon, true)) loc.bits |= (11) << 8;  // WeaponSel_Prev
	if (ctrlGetInputKey(GameKeys.Next_Weapon, true)) loc.bits |= (12) << 8; // WeaponSel_next
	for (int i = 0; i < 10; i++) if (ctrlGetInputKey(whcfg.keymap[i + WhKeys.Weapon_1.getNum()], true)) loc.bits |= (i + 1) << 8; //1 << 12 // weaponsel
	loc.bits |= whcfg.gMouseAim ? CtrlMouseAim : 0; ~SB_AIMMODE
	loc.bits |= ctrlGetInputKey(GameKeys.Open, true) ? CtrlOpen : 0;	SB_OPEN
	loc.bits |= ctrlGetInputKey(WhKeys.Inventory_Use, true) ? CtrlInventory_Use : 0; SB_INVUSE
	loc.bits |= ctrlGetInputKey(WhKeys.Fly_up, false) ? CtrlFlyup : 0; // SB_FLYUP
	loc.bits |= ctrlGetInputKey(WhKeys.Fly_down, false) ? CtrlFlydown : 0; // SB_FLYDOWN
	loc.bits |= ctrlGetInputKey(WhKeys.End_flying, true) ? CtrlEndflying : 0; // SB_FLYSTOP
	if (ctrlGetInputKey(WhKeys.Health_potion, true))	loc.bits |= (1) << 16;  //  SB_ITEM_BIT_1
	if (ctrlGetInputKey(WhKeys.Strength_potion, true))	loc.bits |= (2) << 16; // 1 << 17 // SB_ITEM_BIT_2
	if (ctrlGetInputKey(WhKeys.Curepoison_potion, true))	loc.bits |= (3) << 16; SB_ITEM_BIT_3
	if (ctrlGetInputKey(WhKeys.Fireresist_potion, true))	loc.bits |= (4) << 16; // 1 << 18 SB_ITEM_BIT_4
	if (ctrlGetInputKey(WhKeys.Invisibility_potion, true))	loc.bits |= (5) << 16; SB_ITEM_BIT_5
	loc.bits |= ctrlGetInputKey(WhKeys.Cast_spell, true) ? CtrlCastspell : 0; // SB_SPELL

*/

InputPacket localInput;
static double lPlayerXVel, lPlayerYVel;

void GameInterface::clearlocalinputstate()
{
    localInput = {};
	lPlayerXVel = lPlayerYVel = 0;
}

static void UpdatePlayerSpriteAngle(PLAYER& plr)
{
	sprite[plr.spritenum].ang = plr.angle.ang.asbuild();
}

void GameInterface::GetInput(InputPacket* packet, ControlInfo* const hidInput)
{
    if (paused || M_Active())
    {
        localInput = {};
        lPlayerYVel = 0;
        lPlayerXVel = 0;
        return;
    }

    PLAYER& plr = player[myconnectindex];

    if (packet != nullptr)
    {
        localInput = {};
        ApplyGlobalInput(localInput, hidInput);
        if (plr.dead) localInput.actions &= SB_OPEN;
    }

    double const scaleAdjust = InputScale();
    InputPacket input{};

    if (plr.dead)
    {
        lPlayerYVel = 0;
        lPlayerXVel = 0;
    }
    else
    {
        processMovement(&input, &localInput, hidInput, scaleAdjust);
    }

    if (!cl_syncinput)
    {
		plr.angle.applyinput(input.avel, &plr.plInput.actions, scaleAdjust);
		plr.horizon.applyinput(input.horz, &plr.plInput.actions, scaleAdjust);

        plr.angle.processhelpers(scaleAdjust);
        plr.horizon.processhelpers(scaleAdjust);
        UpdatePlayerSpriteAngle(plr);
    }

    if (packet)
    {
		double k = 0.92;
		double sin = plr.angle.ang.fsin() * 16384.;
		double cos = plr.angle.ang.fcos() * 16384.;
		double xvel = (localInput.fvel * cos) + (localInput.svel * sin);
		double yvel = (localInput.fvel * sin) - (localInput.svel * cos);
		double len = sqrt(xvel * xvel + yvel * yvel);
		auto keymove = localInput.actions & SB_RUN ? gi->playerKeyMove() << 1 : gi->playerKeyMove();

		if (len > (keymove << 14))
		{
			xvel = (xvel / len) * (keymove << 14);
			yvel = (yvel / len) * (keymove << 14);
		}

		lPlayerXVel = MulScaleF(lPlayerXVel + (xvel * k), 0xD000, 16);
		lPlayerYVel = MulScaleF(lPlayerYVel + (yvel * k), 0xD000, 16);

		if (abs(lPlayerXVel) < 2048 && abs(lPlayerYVel) < 2048)
		{
			lPlayerXVel = lPlayerYVel = 0;
		}

		localInput.fvel = xs_CRoundToInt(lPlayerXVel);
		localInput.svel = xs_CRoundToInt(lPlayerYVel);

		*packet = localInput;
		localInput = {};
    }
}

 
void processinput(int num) {

	int goalz, lohit = 0, loz = 0, tics, xvect, yvect;

	int oldposx, oldposy;
	int dist;
	int feetoffground;
	short onsprite = -1;

	PLAYER& plr = player[num];
	oldposx = plr.x;
	oldposy = plr.y;

	plr.angle.resetadjustment();
	plr.horizon.resetadjustment();

	auto& bits = plr.plInput.actions;

	if ((bits & SB_JUMP) == 0)
		plr.keytoggle = false;

	boolean onground;

	if (plr.health <= 0) {
		playerdead(plr);
		if (plr.dead) {
			if (plr.horizon.horiz.asq16() < gi->playerHorizMax())
				plr.horizon.addadjustment(TICSPERFRAME << 1);
		}
		return;
	}

	xvect = yvect = 0;
	tics = TICSPERFRAME;

	plr.plInput.fvel += (tics * damage_vel) << 14;
	plr.plInput.svel += (tics * damage_svel) << 14;
	plr.angle.addadjustment(damage_angvel);


	if (damage_vel != 0)
		damage_vel = std::min(damage_vel + (tics << 3), 0);
	if (damage_svel != 0)
		damage_svel = std::max(damage_svel - (tics << 3), 0);
	if (damage_angvel != 0)
		damage_angvel = std::max(damage_angvel - (tics << 1), 0);

	sprite[plr.spritenum].cstat ^= 1;
	getzrange(plr.x, plr.y, plr.z, plr.sector, 128, CLIPMASK0);

	loz = zr_florz;
	lohit = zr_florhit;

	sprite[plr.spritenum].cstat ^= 1;

	if ((lohit & 0xc000) == 49152) {
		if ((sprite[lohit & 4095].z - plr.z) <= (getPlayerHeight() << 8))
			onsprite = (short)(lohit & 4095);
	}
	else
		onsprite = -1;

	feetoffground = (plr.sector != -1) ? (sector[plr.sector].floorz - plr.z) : 0;

	if (abs(plr.plInput.svel) > gi->playerKeyMove() || abs(plr.plInput.fvel) > gi->playerKeyMove()) {
		if (feetoffground > (32 << 8))
			tics += tics >> 1;
	}

	if (cl_syncinput)
	{
		plr.horizon.applyinput(plr.plInput.horz, &bits);
	}

	if ((bits & SB_FLYSTOP) != 0)
		plr.orbactive[5] = -1;

	if ((bits & SB_FIRE) != 0 && plr.hasshot == 0) { // Les 07/27/95
		if (plr.currweaponfired == 0)
			plrfireweapon(plr);
	}

	// cast
	if ((bits & SB_SPELL) != 0 && plr.orbshot == 0 && plr.currweaponflip == 0 && plr.currweaponanim == 0) {
		if (plr.orb[plr.currentorb] == 1 && plr.currweapon == 0) {
			if (lvlspellcheck(plr)) {
				plr.orbshot = 1;
				activatedaorb(plr);
			}
		}
		if (plr.currweapon != 0) {
			autoweaponchange(plr, 0);
		}
	}

	if ((bits & SB_INVUSE) != 0) {
		if (plr.potion[plr.currentpotion] > 0) {
			usapotion(plr);
		}
	}

	if ((bits & SB_OPEN) != 0) {
		if (netgame) {
			//				   netdropflag();
		}
		else {
			plruse(plr);
		}
	}

	if (cl_syncinput)
	{
		plr.angle.applyinput(plr.plInput.avel, &bits);
	}

	if (plr.sector != -1 && ((sector[plr.sector].floorpicnum != LAVA || sector[plr.sector].floorpicnum != SLIME
		|| sector[plr.sector].floorpicnum != WATER || sector[plr.sector].floorpicnum != HEALTHWATER
		|| sector[plr.sector].floorpicnum != ANILAVA || sector[plr.sector].floorpicnum != LAVA1
		|| sector[plr.sector].floorpicnum != LAVA2) && feetoffground <= (32 << 8))) {
		plr.plInput.fvel /= 3;
		plr.plInput.svel /= 3;
	}

	if ((plr.sector != -1 && (sector[plr.sector].floorpicnum == LAVA || sector[plr.sector].floorpicnum == SLIME
		|| sector[plr.sector].floorpicnum == WATER || sector[plr.sector].floorpicnum == HEALTHWATER
		|| sector[plr.sector].floorpicnum == ANILAVA || sector[plr.sector].floorpicnum == LAVA1
		|| sector[plr.sector].floorpicnum == LAVA2)) && plr.orbactive[5] < 0 // loz
		&& plr.z >= sector[plr.sector].floorz - (plr.height << 8) - (8 << 8)) {
		goalz = loz - (32 << 8);
		switch (sector[plr.sector].floorpicnum) {
		case ANILAVA:
		case LAVA:
		case LAVA1:
		case LAVA2:
			if (plr.treasure[TYELLOWSCEPTER] == 1) {
				goalz = loz - (getPlayerHeight() << 8);
				break;
			}
			else {
				plr.plInput.fvel -= plr.plInput.fvel >> 3;
				plr.plInput.svel -= plr.plInput.svel >> 3;
			}

			if (plr.invincibletime > 0 || plr.manatime > 0 || plr.godMode)
				break;
			else {
				playsound(S_FIRELOOP1, 0, 0, -1, CHAN_LAVA);
				addhealth(plr, -1);
				startredflash(10);
			}
			break;
		case WATER:
			if (plr.treasure[TBLUESCEPTER] == 1) {
				goalz = loz - (getPlayerHeight() << 8);
			}
			else {
				plr.plInput.fvel -= plr.plInput.fvel >> 3;
				plr.plInput.svel -= plr.plInput.svel >> 3;
			}
			break;
		case HEALTHWATER:
			if (plr.health < plr.maxhealth) {
				addhealth(plr, 1);
				startblueflash(5);
			}
			break;
		}
	}
	else if (plr.orbactive[5] > 0) {
		goalz = plr.z - (plr.height << 8);
		plr.hvel = 0;
	}
	else
		goalz = loz - (plr.height << 8);

	if (plr.orbactive[5] < 0 && (bits & SB_JUMP) != 0 && !plr.keytoggle) {
		if (plr.onsomething != 0) {
			if (isWh2())
				plr.hvel -= WH2JUMPVEL;
			else plr.hvel -= JUMPVEL;

			plr.onsomething = 0;
			plr.keytoggle = true;
		}
	}

	if ((bits & SB_CROUCH) != 0) {
		if (plr.sector != -1 && goalz < ((sector[plr.sector].floorz) - (plr.height >> 3))) {
			if (isWh2())
				goalz += (48 << 8);
			else
				goalz += (24 << 8);
		}
	}

	onground = plr.onsomething != 0;
	int vel = (abs(plr.plInput.fvel) + abs(plr.plInput.svel)) >> 16;
	if (vel < 16) vel = 0;
	if ((bits & SB_FLYUP) != 0 || (bits & SB_JUMP) != 0)
		dophysics(plr, goalz, 1, vel);
	else if ((bits & SB_FLYDOWN) != 0 || (bits & SB_CROUCH) != 0)
		dophysics(plr, goalz, -1, vel);
	else
		dophysics(plr, goalz, 0, vel);

	if (!onground && plr.onsomething != 0) {
		if (plr.fallz > 32768) {
			if ((krand() % 2) != 0)
				spritesound(S_PLRPAIN1 + (krand() % 2), &sprite[plr.spritenum]);
			else
				spritesound(S_PUSH1 + (krand() % 2), &sprite[plr.spritenum]);

			addhealth(plr, -(plr.fallz >> 13));
			plr.fallz = 0;// wango
		}
		else if (plr.fallz > 8192) {
			spritesound(S_BREATH1 + (krand() % 2), &sprite[plr.spritenum]);
		}
	}

	if (ihaveflag > 0) { // WHNET
		plr.plInput.fvel -= plr.plInput.fvel >> 2;
		plr.plInput.svel -= plr.plInput.svel >> 2;
	}

	if (plr.plInput.fvel != 0 || plr.plInput.svel != 0) {

		xvect = plr.plInput.fvel;
		yvect = plr.plInput.svel;

		//			xvect = yvect = 0;
		//			if (plr.plInput.fvel != 0) {
		//				xvect = (int) (plr.plInput.fvel * tics * plr.angle.ang.bcos());
		//				yvect = (int) (plr.plInput.fvel * tics * plr.angle.ang.bsin());
		//			}
		//			if (plr.plInput.svel != 0) {
		//				xvect += (plr.plInput.svel * tics * plr.angle.ang.bsin());
		//				yvect -= (plr.plInput.svel * tics * plr.angle.ang.bcos());
		//			}

		if (plr.noclip) {
			plr.x += xvect >> 14;
			plr.y += yvect >> 14;
			int sect = plr.sector;
			updatesector(plr.x, plr.y, &sect);
			if (sect != -1)
				plr.sector = sect;
		}
		else {
			int sect = plr.sector;
			clipmove(&plr.pos, &sect, xvect, yvect, 128, 4 << 8, 4 << 8, CLIPMASK0);
			if (sect != -1) plr.sector = sect;

			sect = plr.sector;
			pushmove(&plr.pos, &sect, 128, 4 << 8, 4 << 8, CLIPMASK0);
			if (sect != -1) plr.sector = sect;
		}

		// JSA BLORB

		if (plr.sector != plr.oldsector) {
				switch (sector[plr.sector].floorpicnum) {
				case ANILAVA:
				case LAVA:
				case LAVA1:
				case LAVA2:
					break;
				default:
					soundEngine->StopSound(CHAN_LAVA);
					break;
				}
			}
			sectorsounds();
		}

		// walking on sprite
		plr.horizon.addadjustment(-oldhoriz);

		dist = ksqrt((plr.x - oldposx) * (plr.x - oldposx) + (plr.y - oldposy) * (plr.y - oldposy));

		if (abs(plr.plInput.svel) > gi->playerKeyMove() || abs(plr.plInput.fvel) > gi->playerKeyMove()) {
			dist >>= 2;

		if (dist > 0 && feetoffground <= (plr.height << 8) || onsprite != -1) {
			oldhoriz = ((dist * bsinf(PlayClock << 5)) * (1. / 524288.)) * (1. / 4.);
			plr.horizon.addadjustment(oldhoriz);
		}
		else
			oldhoriz = 0;

		if (onsprite != -1 && dist > 50 && lopoint == 1 && justplayed == 0) {

			switch (sprite[onsprite].picnum) {
			case WALLARROW:
			case OPENCHEST:
			case GIFTBOX:
				if (walktoggle != 0)
					playsound(S_WOOD1, (plr.x + 3000), plr.y);
				else
					playsound(S_WOOD1, plr.x, (plr.y + 3000));
				walktoggle ^= 1;
				justplayed = 1;
				break;
			case WOODPLANK: // wood planks
				if (walktoggle != 0)
					playsound(S_SOFTCHAINWALK, (plr.x + 3000), plr.y);
				else
					playsound(S_SOFTCHAINWALK, plr.x, (plr.y + 3000));
				walktoggle ^= 1;
				justplayed = 1;

				break;
			case SQUAREGRATE: // square grating
			case SQUAREGRATE + 1:
				if (walktoggle != 0)
					playsound(S_LOUDCHAINWALK, (plr.x + 3000), plr.y);
				else
					playsound(S_LOUDCHAINWALK, plr.x, (plr.y + 3000));
				walktoggle ^= 1;
				justplayed = 1;
				break;
			case SPACEPLANK: // spaced planks
				if (walktoggle != 0)
					playsound(S_SOFTCREAKWALK, (plr.x + 3000), plr.y);
				else
					playsound(S_SOFTCREAKWALK, plr.x, (plr.y + 3000));
				walktoggle ^= 1;
				justplayed = 1;
				break;
			case SPIDER:
				// STOMP
				spritesound(S_DEADSTEP, &sprite[onsprite]);
				justplayed = 1;
				newstatus(onsprite, DIE);
				break;

			case FREDDEAD:
			case 1980:
			case 1981:
			case 1984:
			case 1979:
			case 1957:
			case 1955:
			case 1953:
			case 1952:
			case 1941:
			case 1940:
				spritesound(S_DEADSTEP, &sprite[plr.spritenum]);
				justplayed = 1;

				break;

			default:
				break;
			}

			if (sprite[onsprite].picnum == RAT)
			{
				spritesound(S_RATS1 + (krand() % 2), &sprite[onsprite]);
				justplayed = 1;
				deletesprite(onsprite);
			}
		}

		if (lopoint == 0 && oldhoriz == -2 && justplayed == 0)
			lopoint = 1;

		if (lopoint == 1 && oldhoriz != -2 && justplayed == 1) {
			lopoint = 0;
			justplayed = 0;
		}

		vel = (abs(plr.plInput.fvel) + abs(plr.plInput.svel)) >> 16;
		if (vel > 40 && dist > 10)
			runningtime += TICSPERFRAME;
		else
			runningtime -= TICSPERFRAME;

		if (runningtime < -360)
			runningtime = 0;

		if (runningtime > 360) {
			SND_Sound(S_PLRPAIN1);
			runningtime = 0;
		}
	}

	//game.pInt.setsprinterpolate(plr.spritenum, sprite[plr.spritenum]);
	setsprite(plr.spritenum, plr.x, plr.y, plr.z + (plr.height << 8));
	UpdatePlayerSpriteAngle(plr);

	if (plr.sector >= 0 && getceilzofslope(plr.sector, plr.x, plr.y) > getflorzofslope(plr.sector, plr.x, plr.y) - (8 << 8))
		addhealth(plr, -10);

	if (plr.currweaponfired != 1 && plr.currweaponfired != 6)
		plr.hasshot = 0;
	weaponchange(num);
	bookprocess(num);
	potionchange(num);
}


 END_WH_NS
 
