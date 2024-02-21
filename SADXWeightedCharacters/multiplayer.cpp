#include "pch.h"
#include "SADXModLoader.h"
#include "FunctionHook.h"
#include <map>
#include <vector>
#include "weights.h"

using std::map;
using std::vector;
using std::string;

//The multiplayer mod updates the render for each screen using the display function of the task.
//This makes the weights rendering wrong when they are running in a main function (such as ProcessWelds in Sonic_Main)
//We move the weights in the display of each character if the MP mod is detected so it will render properly


#define TaskHook FunctionHook<void, task*>

TaskHook SonicDisplay_t(SonicDisplay);
TaskHook MilesDisplay_t(MilesDisplay);
TaskHook KnuxDisplay_t(KnucklesDisplay);
TaskHook AmyDisplay_t(AmyDisplay);
TaskHook BigDisplay_t(BigDisplay);

bool isMPMod = false;

void SetWeights(task* tp)
{
	if (!tp || !tp->twp)
		return;

	char id = tp->twp->counter.b[1];

	if (id == 0 && MetalSonicFlag)
		id = Characters_MetalSonic;

	auto charinf = charInfos.find(id);

	if (charinf != charInfos.end() && charinf->second.modelWeights.size() > 0)
	{
		EntityData1* a1 = (EntityData1*)tp->twp;
		EntityData2* a2 = (EntityData2*)tp->mwp;
		if (a2)
		{
			CharObj2* a3 = a2->CharacterData;
			ProcessVertexWelds_Check_(a1, a2, a3);
		}
	}
}

void SonicDisplay_r(task* tp)
{
	SetWeights(tp);
	SonicDisplay_t.Original(tp);
}

void MilesDisplay_r(task* tp)
{
	SetWeights(tp);
	MilesDisplay_t.Original(tp);
}

void KnuxDisplay_r(task* tp)
{
	SetWeights(tp);
	KnuxDisplay_t.Original(tp);
}

void AmyDisplay_r(task* tp)
{
	SetWeights(tp);
	AmyDisplay_t.Original(tp);
}

void BigDisplay_r(task* tp)
{
	SetWeights(tp);
	BigDisplay_t.Original(tp);
}

void initWeightsMultiplayer()
{
	isMPMod = GetModuleHandle(L"sadx-multiplayer");

	if (isMPMod)
	{
		SonicDisplay_t.Hook(SonicDisplay_r);
		MilesDisplay_t.Hook(MilesDisplay_r);
		KnuxDisplay_t.Hook(KnuxDisplay_r);
		AmyDisplay_t.Hook(AmyDisplay_r);
		BigDisplay_t.Hook(BigDisplay_r);
	}
}