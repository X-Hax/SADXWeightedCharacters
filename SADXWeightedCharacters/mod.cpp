#include "pch.h"
#include "SADXModLoader.h"
#include "IniFile.hpp"
#include "ModelInfo.h"
#include "FunctionHook.h"
#include "UsercallFunctionHandler.h"
#include <map>
#include <vector>
#include "weights.h"

using std::map;
using std::vector;
using std::string;
extern bool isMPMod;


const intptr_t sonicWeldPointers[] = {
	0x49AB7E,
	0x49ABAC,
	0x49ACB6,
	0x7D27BA,
	0x7D30A0,
	0x7D58B1,
	0x7D5D21
};

const intptr_t tailsWeldPointers[] = {
	0x461896,
	0x7D2867,
	0x7D3149,
	0x7D595A,
	0x7D5DB5
};

const intptr_t knucklesWeldPointers[] = {
	0x47A89E,
	0x7D2914,
	0x7D31F6,
	0x7D33D3,
	0x7D5A01
};

const intptr_t amyWeldPointers[] = {
	0x48AD0B,
	0x7D29C1,
	0x7D5AB6
};

const intptr_t gammaWeldPointers[] = {
	0x483630
};

const intptr_t bigWeldPointers[] = {
	0x490C14,
	0x7D2B04,
	0x7D329B,
	0x7D5BF4
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
	map<NJS_OBJECT*, ModelWeightInfo>* weightinfo = (map<NJS_OBJECT*, ModelWeightInfo>*)a3->AnimationThing.WeldInfo;
	switch (a3->AnimationThing.field_2)
	{
	case 0:
		for (auto& nodeweights : *weightinfo)
			weightFuncs->Init(nodeweights.second.weights, nodeweights.first);
		a3->AnimationThing.field_2 = 1;
		break;
	case 1:
	{
		NJS_ACTION action = { object, motion };
		auto nodeweights = weightinfo->find(object);
		if (nodeweights != weightinfo->end())
		{
			weightFuncs->Apply(nodeweights->second.weights, &action, frame);
			{
				int* nodeidx = &nodeweights->second.rightHandNode;
				int* dir = &nodeweights->second.rightHandDir;
				for (int i = 0; i < 6; i++)
					if (*nodeidx != -1)
					{
						NJS_VECTOR pos{};
						NJS_VECTOR norm{};
						(&norm.x)[dir[i]] = 1;
						SetInstancedMatrix(nodeidx[i], matrix);
						njCalcPoint(matrix, &pos, &a3->SoManyVectors[i]);
						njCalcVector(matrix, &norm, &a3->SoManyVectors[i + 6]);
					}
			}
		}
	}
	break;
	default:
		for (auto& nodeweights : *weightinfo)
			weightFuncs->DeInit(nodeweights.second.weights, nodeweights.first);
		break;
	}
}

void __cdecl ProcessVertexWelds_Check_(EntityData1* a1, EntityData2* a2, CharObj2* a3)
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
		if (object == SONIC_OBJECTS[0])
			object = SONIC_OBJECTS[68];
		else if (object == SONIC_OBJECTS[66])
			object = SONIC_OBJECTS[69];
		else if (object == SONIC_OBJECTS[67])
			object = SONIC_OBJECTS[70];
	}
	ProcessWeights(a3, object, motion, a3->AnimationThing.Frame);
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
		if (isMPMod) //we don't want this whole function to run at all if MP is enabled, the code will run externally in the character display function for accurate render
			return;

		ProcessVertexWelds_Check_(a1, a2, a3);
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
		if (object == SONIC_OBJECTS[0])
			object = SONIC_OBJECTS[68];
		else if (object == SONIC_OBJECTS[66])
			object = SONIC_OBJECTS[69];
		else if (object == SONIC_OBJECTS[67])
			object = SONIC_OBJECTS[70];
	}
	auto charinf = charInfos.find(id);
	if (charinf != charInfos.end() && charinf->second.modelWeights.size() > 0)
		ProcessWeights(GetCharObj2(index), object, motion, frame);
	else
		EPJoinVertexes_h.Original(index, object, motion, frame);
}

UsercallFuncVoid(mr_join_vertex_init, (PL_JOIN_VERTEX* join_vtx_p), (join_vtx_p), 0x51A7A0, rEAX);
void __cdecl mr_join_vertex_init_Check(PL_JOIN_VERTEX* join_vtx_p)
{
	for (auto& ch : charInfos)
		if ((void*)join_vtx_p == &ch.second.modelWeights)
		{
			for (auto& nodeweights : ch.second.modelWeights)
				weightFuncs->Init(nodeweights.second.weights, nodeweights.first);
			return;
		}
	mr_join_vertex_init.Original(join_vtx_p);
}

UsercallFuncVoid(mr_join_vertex_exec, (Uint32 obj_num, PL_JOIN_VERTEX* join_vtx_p, NJS_ACTION* act_p, NJS_MATRIX* mat_p, Float frame), (obj_num, join_vtx_p, act_p, mat_p, frame), 0x52EBA0, rEAX, rECX, stack4, stack4, stack4);
void __cdecl mr_join_vertex_exec_Check(Uint32 obj_num, PL_JOIN_VERTEX* join_vtx_p, NJS_ACTION* act_p, NJS_MATRIX* mat_p, Float frame)
{
	for (auto& ch : charInfos)
		if ((void*)join_vtx_p == &ch.second.modelWeights)
		{
			auto nodeweights = ch.second.modelWeights.find(act_p->object);
			if (nodeweights != ch.second.modelWeights.end())
				weightFuncs->Apply(nodeweights->second.weights, act_p, frame);
			return;
		}

	mr_join_vertex_exec.Original(obj_num, join_vtx_p, act_p, mat_p, frame);
}

UsercallFuncVoid(mr_join_vertex_end, (PL_JOIN_VERTEX* join_vtx_p), (join_vtx_p), 0x52ED50, rEAX);
void __cdecl mr_join_vertex_end_Check(PL_JOIN_VERTEX* join_vtx_p)
{
	for (auto& ch : charInfos)
		if ((void*)join_vtx_p == &ch.second.modelWeights)
		{
			for (auto& nodeweights : ch.second.modelWeights)
				weightFuncs->DeInit(nodeweights.second.weights, nodeweights.first);
			return;
		}
	mr_join_vertex_end.Original(join_vtx_p);
}

UsercallFuncVoid(ec_join_vertex_end, (PL_JOIN_VERTEX* join_vtx_p), (join_vtx_p), 0x51A8D0, rEAX);
void __cdecl ec_join_vertex_end_Check(PL_JOIN_VERTEX* join_vtx_p)
{
	for (auto& ch : charInfos)
		if ((void*)join_vtx_p == &ch.second.modelWeights)
		{
			for (auto& nodeweights : ch.second.modelWeights)
				weightFuncs->DeInit(nodeweights.second.weights, nodeweights.first);
			return;
		}
	ec_join_vertex_end.Original(join_vtx_p);
}

std::pair<string, string> nodeKeys[] = {
	{ "RightHandPosition", "RightHandDirection" },
	{ "LeftHandPosition", "LeftHandDirection" },
	{ "RightFootPosition", "RightFootDirection" },
	{ "LeftFootPosition", "LeftFootDirection" },
	{ "User0Position", "User0Direction" },
	{ "User1Position", "User1Direction" }
};

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
					mdl->getweightinfo()
					});
			} while (FindNextFileA(hFind, &data) != 0);
			FindClose(hFind);
			const IniFile* mdlini = new IniFile(mdlpath + "models.ini");
			const IniGroup* mdlgrp = mdlini->getGroup("");
			for (auto i = mdlgrp->cbegin(); i != mdlgrp->cend(); ++i)
				if (labels.find(i->second) != labels.cend())
					objectsArray[std::stol(i->first)] = (NJS_OBJECT*)labels[i->second];
			for (auto i = mdlini->cbegin(); i != mdlini->cend(); ++i)
			{
				auto node = labels.find(i->first);
				if (node == labels.end())
					continue;
				auto weights = charinf.second.modelWeights.find(static_cast<NJS_OBJECT*>(node->second));
				if (weights == charinf.second.modelWeights.end())
					continue;
				auto nodeidx = &weights->second.rightHandNode;
				auto dirptr = &weights->second.rightHandDir;
				for (auto& keys : nodeKeys)
				{
					if (i->second->hasKeyNonEmpty(keys.first))
					{
						*nodeidx = GetNodeIndex(weights->first, static_cast<NJS_OBJECT*>(labels[i->second->getString(keys.first)]));
						auto dir = i->second->getString(keys.second, "X");
						if (dir == "Y")
							*dirptr = 1;
						else if (dir == "Z")
							*dirptr = 2;
					}
					++nodeidx;
					++dirptr;
				}
			}
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
		if (charInfos[Characters_Tails].modelWeights.size() > 0)
		{
			NJS_OBJECT** objectsArray = (NJS_OBJECT**)GetProcAddress(hmod, "___MILES_OBJECTS");
			auto& weights = charInfos[Characters_Tails].modelWeights;
			auto lsrweights = weights.find(objectsArray[65]);
			auto lslweights = weights.find(objectsArray[64]);
			if (lsrweights != weights.end() || lslweights != weights.end())
			{
				auto rootweights = weights.find(objectsArray[0]);
				if (rootweights != weights.end())
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
				rootweights = weights.find(objectsArray[1]);
				if (rootweights != weights.end())
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
		if (charInfos[Characters_Knuckles].modelWeights.size() > 0)
			WriteData((char*)0x4726A0, (char)0xC3); // disable upgrades
		WriteData<2>((void*)0x49BE22, 0x90u); // enable welds for sonic's spin model
		ProcessVertexWelds_h.Hook(ProcessVertexWelds_Check);
		EPJoinVertexes_h.Hook(EPJoinVertexes_Check);
		mr_join_vertex_init.Hook(mr_join_vertex_init_Check);
		mr_join_vertex_exec.Hook(mr_join_vertex_exec_Check);
		mr_join_vertex_end.Hook(mr_join_vertex_end_Check);
		ec_join_vertex_end.Hook(ec_join_vertex_end_Check);

		initWeightsMultiplayer();
	}

	__declspec(dllexport) ModInfo SADXModInfo = { ModLoaderVer };
}