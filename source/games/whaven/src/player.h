#pragma once

#include "packet.h"
#include "gameinput.h"

BEGIN_WH_NS

struct PLAYER {
	
	InputPacket plInput;
	int spellnum;
	union {
		struct
		{
			int32_t x, y, z;
		};
		vec3_t pos;
	};
	PlayerAngle angle;
    PlayerHorizon horizon;
    int height;
    int hvel;               
    int sector;
	int oldsector;
	short spritenum;
	DWHActor* actor() { return &whActors[spritenum]; }
	boolean keytoggle;
	int flags;
	int weapon[MAXWEAPONS], preenchantedweapon[MAXWEAPONS];
	int ammo[MAXWEAPONS], preenchantedammo[MAXWEAPONS];
	int orbammo[MAXNUMORBS];
	int treasure[MAXTREASURES];
	int orbactive[MAXNUMORBS];
	int orb[MAXNUMORBS];
	int potion[MAXPOTIONS];
    int lvl;
    int score;
    int health;
    int maxhealth;
    int armor;
    int armortype;
    int onsomething;
    int fallz;
    
    boolean dead;
    
	int shadowtime;
	int helmettime;
	int scoretime;
	int vampiretime;
	
    int selectedgun;
    int currweapon;
	int currweapontics;
	int currweaponanim;
	int currweaponframe;
	int currweaponfired;
	int currweaponattackstyle;
	int currweaponflip;
	int hasshot;
	
	int currentpotion;
	int strongtime, manatime, invisibletime=-1;
	
	int orbshot;
	int spellbooktics;
	int spellbook;
	int spellbookframe;
	int spellbookflip;
	int nightglowtime;

	int showbook;
	int showbooktype;
	int showbookflip;
	int showbookanim;
	int currentorb;

	int spelltime;
	
	int shieldpoints;
	int shieldtype;

	int poisoned;
	int poisontime;
	int shockme = -1;
	int invincibletime;
	
	int spiked;
	int spiketics;
	int spikeframe;
	int currspikeframe;
	boolean godMode;
	boolean noclip;

	bool over_shoulder_on;
	int justwarpedfx;
	int justwarpedcnt;


	// no copy assignments for this one!
	PLAYER() = default;
	PLAYER(const PLAYER&) = delete;
	PLAYER& operator=(const PLAYER&) = delete;
};

struct WEAPONINF {
	int daweapontics;
	int daweaponframe;
	int currx;
	int curry;
};

END_WH_NS
