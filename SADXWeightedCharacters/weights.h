#pragma once

using std::map;

struct ModelWeightInfo
{
	WeightInfo* weights;
	int rightHandNode = -1;
	int leftHandNode = -1;
	int rightFootNode = -1;
	int leftFootNode = -1;
	int user0Node = -1;
	int user1Node = -1;
	int rightHandDir;
	int leftHandDir;
	int rightFootDir;
	int leftFootDir;
	int user0Dir;
	int user1Dir;
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

extern map<int, CharInfo> charInfos;
void __cdecl ProcessVertexWelds_Check_(EntityData1* a1, EntityData2* a2, CharObj2* a3);
void initWeightsMultiplayer();