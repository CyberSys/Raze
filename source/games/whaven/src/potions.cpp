#include "ns.h"
#include "wh.h"

BEGIN_WH_NS
	
int potiontilenum;
	
void potiontext(PLAYER& plr) {    
	if( plr.potion[plr.currentpotion] > 0)
		switch(plr.currentpotion) {
		case 0:
			showmessage("Health Potion", 240);
		break;
		case 1:
			showmessage("Strength Potion", 240);
		break;
		case 2:
			showmessage("Cure Poison Potion", 240);
		break;
		case 3:
			showmessage("Resist Fire Potion", 240);
		break;
		case 4:
			showmessage("Invisibility Potion", 240);
		break;
		}
}
	
void potionchange(int snum)
{
	PLAYER& plr = player[snum];
	if (!(plr.plInput.actions & (SB_INVPREV | SB_INVNEXT | SB_ITEM_BIT_1 | SB_ITEM_BIT_2 | SB_ITEM_BIT_3 | SB_ITEM_BIT_4 | SB_ITEM_BIT_5)))
		return;

	if (plr.plInput.actions & SB_INVPREV)
	{
		if (--plr.currentpotion < 0)
			plr.currentpotion = MAXPOTIONS - 1;
	}
	if (plr.plInput.actions & SB_INVNEXT)
	{
		if (++plr.currentpotion >= MAXPOTIONS)
			plr.currentpotion = 0;
	}
	if (plr.plInput.actions & SB_ITEM_BIT_1)
		plr.currentpotion = 0;
	if (plr.plInput.actions & SB_ITEM_BIT_2)
		plr.currentpotion = 1;
	if (plr.plInput.actions & SB_ITEM_BIT_3)
		plr.currentpotion = 2;
	if (plr.plInput.actions & SB_ITEM_BIT_4)
		plr.currentpotion = 3;
	if (plr.plInput.actions & SB_ITEM_BIT_5)
		plr.currentpotion = 4;
	SND_Sound(S_BOTTLES);
		potiontext(plr);
}
	
void usapotion(PLAYER& plr) {

	if( plr.currentpotion == 0  && plr.health >= plr.maxhealth )
		return;
		
	if( plr.currentpotion == 2  && plr.poisoned == 0 )
		return;
		
	if( plr.potion[plr.currentpotion] <= 0)
		return;
	else
		plr.potion[plr.currentpotion]--;
		
	switch(plr.currentpotion) {
	case 0: // health potion
		if( plr.health+25 > plr.maxhealth) {
			plr.health=0;
			SND_Sound(S_DRINK);
			addhealth(plr, plr.maxhealth);
		}
		else {   
			SND_Sound(S_DRINK);
			addhealth(plr, 25);
		}
		startblueflash(10);
	break;
	case 1: // strength
		plr.strongtime=3200;
		SND_Sound(S_DRINK);
		startredflash(10);
	break;
	case 2: // anti venom
		SND_Sound(S_DRINK);
		plr.poisoned=0;
		plr.poisontime=0;
		startwhiteflash(10);
		showmessage("poison cured", 360);
		addhealth(plr, 0);
	break;
	case 3: // fire resist
		SND_Sound(S_DRINK);
		plr.manatime=3200;
		startwhiteflash(10);
		soundEngine->StopSound(CHAN_LAVA);
	break;
	case 4: // invisi
		SND_Sound(S_DRINK);
		plr.invisibletime=3200;
		startgreenflash(10);
	break;
	}
}
	
boolean potionspace(PLAYER& plr, int vial) {
	if(plr.potion[vial] > 9) 
		return false;
	else
		return true;
}
	
void updatepotion(PLAYER& plr, int vial) {
	switch(vial) {
		case HEALTHPOTION:
			plr.potion[0]++;
		break;
		case STRENGTHPOTION:
			plr.potion[1]++;
		break;
		case ARMORPOTION:
			plr.potion[2]++;
		break;
		case FIREWALKPOTION:
			plr.potion[3]++;
		break;
		case INVISIBLEPOTION:
			plr.potion[4]++;
		break;
	}
}
	
void randompotion(int i) {
	if ((krand() % 100) > 20)
		return;

	auto spawnedactor = InsertActor(sprite[i].sectnum, (short)0);
	auto& spawned = spawnedactor->s();

	spawned.x = sprite[i].x;
	spawned.y = sprite[i].y;
	spawned.z = sprite[i].z - (12 << 8);
	spawned.shade = -12;
	spawned.pal = 0;
	spawned.cstat = 0;
	spawned.cstat &= ~3;
	spawned.xrepeat = 64;
	spawned.yrepeat = 64;
	int type = krand() % 4;
	spawned.picnum = (short)(FLASKBLUE + type);
	spawned.detail = (short)(FLASKBLUETYPE + type);
	spawned.backuploc();
}

END_WH_NS
