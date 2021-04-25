#pragma once

#include "resourcefile.h"
#include "build.h"

extern FixedBitArray<MAXSPRITES> activeSprites;

// Savegame utilities
class FileReader;

FString G_BuildSaveName (const char *prefix);
int G_ValidateSavegame(FileReader &fr, FString *savetitle, bool formenu);

void G_LoadGame(const char* filename);
void G_SaveGame(const char* fn, const char* desc, bool ok4q, bool forceq);

void M_Autosave();

#define SAVEGAME_EXT ".dsave"


inline FSerializer& Serialize(FSerializer& arc, const char* keyname, spritetype*& w, spritetype** def)
{
	int ndx = w ? int(w - sprite) : -1;
	arc(keyname, ndx);
	w = ndx == -1 ? nullptr : sprite + ndx;
	return arc;
}

inline FSerializer& Serialize(FSerializer& arc, const char* keyname, sectortype*& w, sectortype** def)
{
	int ndx = w ? int(w - sector) : -1;
	arc(keyname, ndx);
	w = ndx == -1 ? nullptr : sector + ndx;
	return arc;
}

inline FSerializer& Serialize(FSerializer& arc, const char* keyname, walltype*& w, walltype** def)
{
	int ndx = w ? int(w - wall) : -1;
	arc(keyname, ndx);
	w = ndx == -1 ? nullptr : wall + ndx;
	return arc;
}

