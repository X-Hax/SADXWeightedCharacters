#include "pch.h"
#include "SADXModLoader.h"
#include "IniFile.hpp"
#include "ModelInfo.h"
#include "FunctionHook.h"
#include <map>
#include <vector>

using std::map;
using std::vector;
using std::string;

struct ModelWeightInfo
{
	int rightHandNode;
	int leftHandNode;
	int rightFootNode;
	int leftFootNode;
	int user0Node;
	int user1Node;
	int rightHandDir;
	int leftHandDir;
	int rightFootDir;
	int leftFootDir;
	int user0Dir;
	int user1Dir;
	WeightInfo* weights;
};

struct CharInfo
{
	const char* modelPath;
	const char* objectsArray;
	int objectsLength;
	const char* actionsArray;
	int actionsLength;
	const intptr_t* pointersArray;
	int pointersLength;
	map<NJS_OBJECT*, ModelWeightInfo> modelWeights;
};

const intptr_t sonicWeldPointers[] = {
	0x49AB7E,
	0x49ABAC,
	0x49ACB6
};

const intptr_t tailsWeldPointers[] = {
	0x461896
};

const intptr_t knucklesWeldPointers[] = {
	0x47A89E
};

const intptr_t amyWeldPointers[] = {
	0x48AD0B
};

const intptr_t gammaWeldPointers[] = {
	0x483630
};

const intptr_t bigWeldPointers[] = {
	0x490C14
};

const intptr_t metalWeldPointers[] = {
	0x49ACC2
};

map<int, CharInfo> charInfos = {
	{
		Characters_Sonic,
		{
			"\\models\\sonic\\",
			"___SONIC_OBJECTS",
			79,
			"___SONIC_ACTIONS",
			149,
			arrayptrandlengthT(sonicWeldPointers, int)
		}
	},
	{
		Characters_Tails,
		{
			"\\models\\tails\\",
			"___MILES_OBJECTS",
			72,
			"___MILES_ACTIONS",
			114,
			arrayptrandlengthT(tailsWeldPointers, int)
		}
	},
	{
		Characters_Knuckles,
		{
			"\\models\\knuckles\\",
			"___KNUCKLES_OBJECTS",
			74,
			"___KNUCKLES_ACTIONS",
			90,
			arrayptrandlengthT(knucklesWeldPointers, int)
		}
	},
	{
		Characters_Amy,
		{
			"\\models\\amy\\",
			"___AMY_OBJECTS",
			39,
			"___AMY_ACTIONS",
			80,
			arrayptrandlengthT(amyWeldPointers, int)
		}
	},
	{
		Characters_Gamma,
		{
			"\\models\\gamma\\",
			"___E102_OBJECTS",
			28,
			"___E102_ACTIONS",
			78,
			arrayptrandlengthT(gammaWeldPointers, int)
		}
	},
	{
		Characters_Big,
		{
			"\\models\\big\\",
			"___BIG_OBJECTS",
			44,
			"___BIG_ACTIONS",
			90,
			arrayptrandlengthT(bigWeldPointers, int)
		}
	},
	{
		Characters_MetalSonic,
		{
			"\\models\\metalsonic\\",
			"___SONIC_OBJECTS",
			79,
			"___SONIC_ACTIONS",
			149,
			arrayptrandlengthT(metalWeldPointers, int)
		}
	}
};

const BasicWeightFuncs* weightFuncs;

NJS_MATRIX matrix;
void ProcessWeights(CharObj2* a3, NJS_OBJECT*& object, NJS_MOTION* motion, float frame)
{
	NJS_ACTION action = { object, motion };
	map<NJS_OBJECT*, ModelWeightInfo>* weightinfo = (map<NJS_OBJECT*, ModelWeightInfo>*)a3->AnimationThing.WeldInfo;
	auto nodeweights = weightinfo->find(object);
	if (nodeweights != weightinfo->end())
	{
		switch (a3->AnimationThing.field_2)
		{
		case 0:
			weightFuncs->Init(nodeweights->second.weights, object);
			a3->AnimationThing.field_2 = 1;
			break;
		case 1:
			weightFuncs->Apply(nodeweights->second.weights, &action, frame);
			{
				int* nodeidx = &nodeweights->second.rightHandNode;
				int* dir = &nodeweights->second.rightHandDir;
				for (int i = 0; i < 6; i++)
				{
					NJS_VECTOR pos;
					NJS_VECTOR norm;
					(&norm.x)[dir[i]] = 1;
					SetInstancedMatrix(nodeidx[i], matrix);
					njCalcPoint(matrix, &pos, &a3->SoManyVectors[i]);
					njCalcVector(matrix, &norm, &a3->SoManyVectors[i + 6]);
				}
			}
			break;
		default:
			weightFuncs->DeInit(nodeweights->second.weights, object);
			break;
		}
	}
}

FunctionHook<void, EntityData1*, EntityData2*, CharObj2*> ProcessVertexWelds_h(ProcessVertexWelds);
void __cdecl ProcessVertexWelds_Check(EntityData1* a1, EntityData2* a2, CharObj2* a3)
{
	char id = a1->CharID;
	if (id == 0 && MetalSonicFlag)
		id = Characters_MetalSonic;
	auto charinf = charInfos.find(id);
	if (charinf != charInfos.end() && charinf->second.modelWeights.size() > 0)
	{
		NJS_OBJECT* object;
		NJS_MOTION* motion;
		if (a3->AnimationThing.State == 2)
		{
			object = a3->AnimationThing.action->object;
			motion = a3->AnimationThing.action->motion;
		}
		else
		{
			object = a3->AnimationThing.AnimData[(unsigned __int16)a3->AnimationThing.Index].Animation->object;
			motion = a3->AnimationThing.AnimData[(unsigned __int16)a3->AnimationThing.Index].Animation->motion;
		}
		if (MetalSonicFlag)
		{
			if (object = SONIC_OBJECTS[0])
				object = SONIC_OBJECTS[68];
			else if (object = SONIC_OBJECTS[66])
				object = SONIC_OBJECTS[69];
			else if (object = SONIC_OBJECTS[67])
				object = SONIC_OBJECTS[70];
		}
		ProcessWeights(a3, object, motion, a3->AnimationThing.Frame);
	}
	else
		ProcessVertexWelds_h.Original(a1, a2, a3);
}

FunctionHook<void, char, NJS_OBJECT*, NJS_MOTION*, float> EPJoinVertexes_h(EPJoinVertexes);
void __cdecl EPJoinVertexes_Check(char index, NJS_OBJECT* object, NJS_MOTION* motion, float frame)
{
	char id = GetCharacterID(index);
	if (id == 0 && MetalSonicFlag)
	{
		id = Characters_MetalSonic;
		if (object = SONIC_OBJECTS[0])
			object = SONIC_OBJECTS[68];
		else if (object = SONIC_OBJECTS[66])
			object = SONIC_OBJECTS[69];
		else if (object = SONIC_OBJECTS[67])
			object = SONIC_OBJECTS[70];
	}
	auto charinf = charInfos.find(id);
	if (charinf != charInfos.end() && charinf->second.modelWeights.size() > 0)
		ProcessWeights(GetCharObj2(index), object, motion, frame);
	else
		EPJoinVertexes_h.Original(index, object, motion, frame);
}

extern "C"
{
	__declspec(dllexport) void Init(const char* path, const HelperFunctions& helperFunctions)
	{
		if (helperFunctions.Version < 17)
		{
			PrintDebug("This version of the mod loader does not support weights! Please update your mod loader to use this mod.\n");
			return;
		}
		weightFuncs = helperFunctions.Weights;
		HMODULE hmod = GetModuleHandle(L"CHRMODELS_orig");
		for (auto &charinf : charInfos)
		{
			NJS_OBJECT** objectsArray = (NJS_OBJECT**)GetProcAddress(hmod, charinf.second.objectsArray);
			NJS_ACTION** actionsArray = (NJS_ACTION**)GetProcAddress(hmod, charinf.second.actionsArray);
			NJS_OBJECT** objbak = new NJS_OBJECT * [charinf.second.objectsLength];
			memcpy(objbak, objectsArray, charinf.second.objectsLength * sizeof(NJS_OBJECT*));
			string mdlpath = string(path) + charinf.second.modelPath;
			map<string, void*> labels;
			WIN32_FIND_DATAA data;
			string tmpstr = mdlpath + "*.sa1mdl";
			HANDLE hFind = FindFirstFileA(tmpstr.c_str(), &data);
			if (hFind == INVALID_HANDLE_VALUE)
			{
				// No files found.
				continue;
			}

			do
			{
				const std::string modFile = mdlpath + data.cFileName;
				ModelInfo* mdl = new ModelInfo(modFile.c_str());
				auto map = mdl->getlabels();
				for (auto i = map->cbegin(); i != map->cend(); ++i)
					labels[i->first] = i->second;
				auto root = mdl->getmodel();
				charinf.second.modelWeights.insert_or_assign(root, ModelWeightInfo{
					GetNodeIndex(root, mdl->getrighthandnode()),
					GetNodeIndex(root, mdl->getlefthandnode()),
					GetNodeIndex(root, mdl->getrightfootnode()),
					GetNodeIndex(root, mdl->getleftfootnode()),
					GetNodeIndex(root, mdl->getuser0node()),
					GetNodeIndex(root, mdl->getuser1node()),
					mdl->getrighthanddir(),
					mdl->getlefthanddir(),
					mdl->getrightfootdir(),
					mdl->getleftfootdir(),
					mdl->getuser0dir(),
					mdl->getuser1dir(),
					mdl->getweightinfo()
					});
			} while (FindNextFileA(hFind, &data) != 0);
			FindClose(hFind);
			const IniFile* mdlini = new IniFile(mdlpath + "models.ini");
			const IniGroup* mdlgrp = mdlini->getGroup("");
			for (auto i = mdlgrp->cbegin(); i != mdlgrp->cend(); ++i)
				if (labels.find(i->second) != labels.cend())
					objectsArray[std::stol(i->first)] = (NJS_OBJECT*)labels[i->second];
			delete mdlini;
			for (int i = 0; i < charinf.second.actionsLength; i++)
				if (actionsArray[i])
					for (int j = 0; j < charinf.second.objectsLength; j++)
						if (actionsArray[i]->object == objbak[j])
						{
							actionsArray[i]->object = objectsArray[j];
							break;
						}
			delete[] objbak;
			for (int i = 0; i < charinf.second.pointersLength; i++)
				WriteData((decltype(CharInfo::modelWeights)**)charinf.second.pointersArray[i], &charinf.second.modelWeights);
		}
		if (charInfos[Characters_Sonic].modelWeights.size() > 0)
		{
			WriteData((char*)0x49BE77, (char)0xEB); // disable crystal ring swap
			WriteData((char*)0x493500, (char)0xC3); // disable stretchy shoes
			NJS_OBJECT** objectsArray = (NJS_OBJECT**)GetProcAddress(hmod, "___SONIC_OBJECTS");
			auto& weights = charInfos[Characters_Sonic].modelWeights;
			auto rootweights = weights.find(objectsArray[0]);
			if (rootweights != weights.end())
			{
				auto lsrweights = weights.find(objectsArray[60]);
				auto lslweights = weights.find(objectsArray[58]);
				if (lsrweights != weights.end() || lslweights != weights.end())
				{
					int newcnt = rootweights->second.weights->nodeCount;
					if (lsrweights != weights.end())
						newcnt += lsrweights->second.weights->nodeCount;
					if (lslweights != weights.end())
						newcnt += lslweights->second.weights->nodeCount;
					auto tmp = new WeightNode[newcnt];
					memcpy(tmp, rootweights->second.weights->nodes, rootweights->second.weights->nodeCount * sizeof(WeightNode));
					int dst = rootweights->second.weights->nodeCount;
					if (lsrweights != weights.end())
					{
						memcpy(&tmp[dst], lsrweights->second.weights->nodes, lsrweights->second.weights->nodeCount * sizeof(WeightNode));
						dst += lsrweights->second.weights->nodeCount;
					}
					if (lslweights != weights.end())
					{
						memcpy(&tmp[dst], lslweights->second.weights->nodes, lslweights->second.weights->nodeCount * sizeof(WeightNode));
						dst += lslweights->second.weights->nodeCount;
					}
					rootweights->second.weights = new WeightInfo{ tmp, newcnt };
				}
			}
		}
		WriteData<2>((void*)0x49BE22, 0x90u); // enable welds for sonic's spin model
		ProcessVertexWelds_h.Hook(ProcessVertexWelds_Check);
		EPJoinVertexes_h.Hook(EPJoinVertexes_Check);
	}

	__declspec(dllexport) ModInfo SADXModInfo = { ModLoaderVer };
}