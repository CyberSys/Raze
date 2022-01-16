BEGIN_DUKE_NS

class DDukeCrane : public DDukeActor
{
public:
	DECLARE_CLASS(DDukeCrane, DDukeActor)
public:
	int crane;	// first picnum.

	void onSpawn() override;
	void Tick() override;

	void Serialize(FSerializer& arc) override // temporary workaround until this can use real textures
	{
		Super::Serialize(arc);
		arc("cranepic", crane);
	}
};

class DDukeCranePole : public DDukeActor
{
public:
	DECLARE_CLASS(DDukeCranePole, DDukeActor)
};

IMPLEMENT_CLASS(DDukeCrane, false, false)
IMPLEMENT_CLASS(DDukeCranePole, false, false)


//---------------------------------------------------------------------------
//
// 
//
//---------------------------------------------------------------------------

void initcrane(DDukeCrane* act)
{
	act->crane = act->spr.picnum;
	auto sect = act->sector();
	act->spr.cstat |= CSTAT_SPRITE_BLOCK_ALL | CSTAT_SPRITE_ONE_SIDE;

	act->spr.picnum += 2;
	act->spr.pos.Z = sect->ceilingz + (48 << 8);
	act->temp_data[4] = cranes.Reserve(1);

	auto& apt = cranes[act->temp_data[4]];
	apt.pos = act->spr.pos;
	apt.poleactor = nullptr;

	DukeStatIterator it(STAT_DEFAULT);
	while (auto actk = it.Next())
	{
		if (act->spr.hitag == actk->spr.hitag && actk->IsKindOf(RUNTIME_CLASS(DDukeCranePole)))
		{
			apt.poleactor = actk;

			act->temp_sect = actk->sector();

			actk->spr.xrepeat = 48;
			actk->spr.yrepeat = 128;

			apt.pole.X = actk->spr.pos.X;
			apt.pole.Y = actk->spr.pos.Y;

			actk->spr.pos = act->spr.pos;
			actk->spr.shade = act->spr.shade;

			SetActor(actk, actk->spr.pos);
			break;
		}
	}

	act->SetOwner(nullptr);
	act->spr.extra = 8;
	ChangeActorStat(act, STAT_STANDABLE);
}

//---------------------------------------------------------------------------
//
// split out of movestandables
//
//---------------------------------------------------------------------------

void movecrane(DDukeCrane *actor)
{
	int crane = actor->crane;
	auto sectp = actor->sector();
	int x;
	auto& cpt = cranes[actor->temp_data[4]];

	//actor->temp_data[0] = state
	//actor->temp_data[1] = checking sector number

	if (actor->spr.xvel) getglobalz(actor);

	if (actor->temp_data[0] == 0) //Waiting to check the sector
	{
		DukeSectIterator it(actor->temp_sect);
		while (auto a2 = it.Next())
		{
			switch (a2->spr.statnum)
			{
			case STAT_ACTOR:
			case STAT_ZOMBIEACTOR:
			case STAT_STANDABLE:
			case STAT_PLAYER:
				actor->spr.ang = getangle(cpt.pole.X - actor->spr.pos.X, cpt.pole.Y - actor->spr.pos.Y);
				SetActor(a2, { cpt.pole.X, cpt.pole.Y, a2->spr.pos.Z });
				actor->temp_data[0]++;
				return;
			}
		}
	}

	else if (actor->temp_data[0] == 1)
	{
		if (actor->spr.xvel < 184)
		{
			actor->spr.picnum = crane + 1;
			actor->spr.xvel += 8;
		}
		//IFMOVING;	// JBF 20040825: see my rant above about this
		ssp(actor, CLIPMASK0);
		if (actor->sector() == actor->temp_sect)
			actor->temp_data[0]++;
	}
	else if (actor->temp_data[0] == 2 || actor->temp_data[0] == 7)
	{
		actor->spr.pos.Z += (1024 + 512);

		if (actor->temp_data[0] == 2)
		{
			if ((sectp->floorz - actor->spr.pos.Z) < (64 << 8))
				if (actor->spr.picnum > crane) actor->spr.picnum--;

			if ((sectp->floorz - actor->spr.pos.Z) < (4096 + 1024))
				actor->temp_data[0]++;
		}
		if (actor->temp_data[0] == 7)
		{
			if ((sectp->floorz - actor->spr.pos.Z) < (64 << 8))
			{
				if (actor->spr.picnum > crane) actor->spr.picnum--;
				else
				{
					if (actor->IsActiveCrane())
					{
						int p = findplayer(actor, &x);
						S_PlayActorSound(isRR() ? 390 : DUKE_GRUNT, ps[p].GetActor());
						if (ps[p].on_crane == actor)
							ps[p].on_crane = nullptr;
					}
					actor->temp_data[0]++;
					actor->SetActiveCrane(false);
				}
			}
		}
	}
	else if (actor->temp_data[0] == 3)
	{
		actor->spr.picnum++;
		if (actor->spr.picnum == (crane + 2))
		{
			int p = checkcursectnums(actor->temp_sect);
			if (p >= 0 && ps[p].on_ground)
			{
				actor->SetActiveCrane(true);
				ps[p].on_crane = actor;
				S_PlayActorSound(isRR() ? 390 : DUKE_GRUNT, ps[p].GetActor());
				ps[p].angle.settarget(actor->spr.ang + 1024);
			}
			else
			{
				DukeSectIterator it(actor->temp_sect);
				while (auto a2 = it.Next())
				{
					switch (a2->spr.statnum)
					{
					case 1:
					case 6:
						actor->SetOwner(a2);
						break;
					}
				}
			}

			actor->temp_data[0]++;//Grabbed the sprite
			actor->temp_data[2] = 0;
			return;
		}
	}
	else if (actor->temp_data[0] == 4) //Delay before going up
	{
		actor->temp_data[2]++;
		if (actor->temp_data[2] > 10)
			actor->temp_data[0]++;
	}
	else if (actor->temp_data[0] == 5 || actor->temp_data[0] == 8)
	{
		if (actor->temp_data[0] == 8 && actor->spr.picnum < (crane + 2))
			if ((sectp->floorz - actor->spr.pos.Z) > 8192)
				actor->spr.picnum++;

		if (actor->spr.pos.Z < cpt.pos.Z)
		{
			actor->temp_data[0]++;
			actor->spr.xvel = 0;
		}
		else
			actor->spr.pos.Z -= (1024 + 512);
	}
	else if (actor->temp_data[0] == 6)
	{
		if (actor->spr.xvel < 192)
			actor->spr.xvel += 8;
		actor->spr.ang = getangle(cpt.pos.X - actor->spr.pos.X, cpt.pos.Y - actor->spr.pos.Y);
		ssp(actor, CLIPMASK0);
		if (((actor->spr.pos.X - cpt.pos.X) * (actor->spr.pos.X - cpt.pos.X) + (actor->spr.pos.Y - cpt.pos.Y) * (actor->spr.pos.Y - cpt.pos.Y)) < (128 * 128))
			actor->temp_data[0]++;
	}

	else if (actor->temp_data[0] == 9)
		actor->temp_data[0] = 0;

	if (cpt.poleactor)
		SetActor(cpt.poleactor, { actor->spr.pos.X, actor->spr.pos.Y, actor->spr.pos.Z - (34 << 8) });

	auto Owner = actor->GetOwner();
	if (Owner != nullptr || actor->IsActiveCrane())
	{
		int p = findplayer(actor, &x);

		int j = fi.ifhitbyweapon(actor);
		if (j >= 0)
		{
			if (actor->IsActiveCrane())
				if (ps[p].on_crane == actor)
					ps[p].on_crane = nullptr;
			actor->SetActiveCrane(false);
			actor->spr.picnum = crane;
			return;
		}

		if (Owner != nullptr)
		{
			SetActor(Owner, actor->spr.pos);

			Owner->opos = actor->spr.pos;

			actor->spr.zvel = 0;
		}
		else if (actor->IsActiveCrane())
		{
			auto ang = ps[p].angle.ang.asbuild();
			ps[p].opos.X = ps[p].pos.X;
			ps[p].opos.Y = ps[p].pos.Y;
			ps[p].opos.Z = ps[p].pos.Z;
			ps[p].pos.X = actor->spr.pos.X - bcos(ang, -6);
			ps[p].pos.Y = actor->spr.pos.Y - bsin(ang, -6);
			ps[p].pos.Z = actor->spr.pos.Z + (2 << 8);
			SetActor(ps[p].GetActor(), ps[p].pos);
			ps[p].setCursector(ps[p].GetActor()->sector());
		}
	}
}

void DDukeCrane::onSpawn()
{
	initcrane(this);
}

void DDukeCrane::Tick()
{
	movecrane(this);
}

END_DUKE_NS
