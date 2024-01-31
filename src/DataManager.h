#pragma once

#include "ExportData.h"

#include "ThreadPool.h"

#include <filesystem>
#include <memory>

//0 - partial - the old type, only the bone values that change are affected
//1 - full - all bones are recorded per animation, regardless of whether they change or not
#define ANIM_TYPE 1

//template <typename T>
struct ThreadImportStruct {
	std::vector<MeshData<boneVertex>> meshes{};
	std::vector<MeshData<boneVertexNoTangent>> meshesNT{};
	std::vector<std::string> meshNames;
	std::vector<std::string> meshNTNames;
	uint32_t boneCount;
	std::vector<std::string> boneVectorForPrinting;
	std::vector<bool> usefulBones;


	static uint32_t activeThreadCount;

	//not currently accounting for bones??

	void threadImport(std::string threadPath);
	void print() {
		printf("size - meshes, meshNames - meshesNT, meshNTNames ~ %d:%d - %d:%d \n", meshes.size(), meshNames.size(), meshesNT.size(), meshNTNames.size());

		uint32_t usefulBoneCount = 0;
		for (int i = 0; i < usefulBones.size(); i++) {
			usefulBoneCount += usefulBones[i];
		}
		printf("boneCount, usefulBoneCount - %d:%d \n", boneCount, usefulBoneCount);

		std::vector<std::pair<std::vector<boneVertex>, std::vector<uint32_t>>> meshes;
		std::vector<std::pair<std::vector<boneVertexNoTangent>, std::vector<uint32_t>>> meshesNT;
		std::vector<std::string> meshNames;
		std::vector<std::string> meshNTNames;
		uint32_t boneCount;
		std::vector<std::string> boneVectorForPrinting;
		std::vector<bool> usefulBones;
	}
};



class DataManager {
	public:	
	ExportData exportData;
	std::unique_ptr<SkeletonHandler> skeleton;

	std::string filePath;
	std::string importPath;
	uint32_t boneCount = 0;
	std::vector<std::string> boneVectorForPrinting;
	std::vector<std::pair<uint32_t, std::string>> removedBoneNames;
	std::vector<std::string> animationNames;
	std::vector<bool> batchUsefulBones;

	DataManager(std::string animPath, std::string exportLocation, int creationFlag /*literally could be anything, just using this to signify anim only*/) {
		printf("BATCH CONSTRUCTOR \n");
		boneCount = 101;
		batchUsefulBones.resize(boneCount, false);

		filePath = exportLocation;
		skeleton = std::make_unique<SkeletonHandler>(animPath);
		printf("after construction of skeleton \n");

#if ANIM_TYPE == 0
		exportData.animExport.animations.resize(skeleton->animationNames.size(), {});
		printf("skeleton animation name size : %d \n", skeleton->animationNames.size());
#else
		exportData.fullAnim.animations.resize(skeleton->animationNames.size(), {});
#endif


		auto boneMap = skeleton->GetBoneInfoMap();
		printf("data manager batch constructor, boneMap size : %d \n", boneMap.size());
		boneVectorForPrinting.resize(boneMap.size());
		for (auto iter = boneMap.begin(); iter != boneMap.end(); iter++) {
			boneVectorForPrinting[iter->second.id] = iter->first;
		}
	}
	static void batchImporter();
	DataManager(std::string fileLocation, std::string exportLocation);

	//run these functions in order, or run FullProcess() instead
	void fullProcess() {
		printf("initializing data \n");
		initializeTheData();
		if (skeleton->animationCount != 0) {
#if ANIM_TYPE == 0
			printf("trimming animations \n");
			trimAnimations();
			printf("cleaning animations \n");
			cleanAnimations();
#else

#endif

			printf("adjusting bones \n");
			adjustBones();
		}
		printf("logging \n");
		Logging();
		printf("archiving \n");
		fileWrite();
		//fileImportAndCompare();
	}
	//void meshProcessing(); //not currently active, would pretend like no animations are imported and just process the mesh, nto sure if possible
	//void animProcessing(); //would just process animations

	void checkBoneSequence();
	void initializeTheData();
	void adjustBones();
	

	void trimAnimations();
	void cleanAnimations();

	//void setDefaultBones();
	void setDefaultBones(std::vector<glm::mat4>& defaultFinal);

	void Logging();
	void fileWrite();
	//void fileImportAndCompare();


private:
	std::vector<int32_t> cleanWeightlessBones();

	void printGLM4(std::vector<glm::mat4>& printed, std::string titleOf);
	void printGLM4(glm::mat4 printed, std::string titleOf);
	void printAnimationBoneSet(std::vector<ExportData::boneEData>& printed, std::string titleOf);

	void checkWonkyBones();
	//void printBobMat4(std::vector<ExportData::bobmat4> printed, std::string titleOf);
};

/*
NEED TO TRIM BONES WHILE CLEANING, IT'S OFFSETTING MY BONE INDEXES

CHECK FOR WEIGHTLESS BONES, that have no weights, remove them
then check for unchanging bones per animation, then remove those from the animation
*/