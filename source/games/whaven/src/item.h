#pragma once

struct Item {

	using Callback = void (*)(PLAYER &plr, DWHActor* actor);
	
	int sizx, sizy;
	boolean treasures, cflag;
	Callback pickup;
	
	void Init(int sizx, int sizy, boolean treasure, boolean cflag, Callback call)
	{
		this->sizx = sizx;
		this->sizy = sizy;
		this->treasures = treasure;
		this->cflag = cflag;
		this->pickup = call;
	}
};

extern Item items[MAXITEMS - ITEMSBASE];


