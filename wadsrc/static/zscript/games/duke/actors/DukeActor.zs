class DukeActor : CoreActor native
{ 
	virtual native void onSpawn();
	virtual native void Tick();
	virtual native void RunState();	// this is the CON function.
}

class DukeCrane : DukeActor
{
	override native void onSpawn();
	override native void Tick();
}

class DukeCranePole : DukeActor
{
}
