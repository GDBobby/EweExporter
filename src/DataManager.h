#pragma once

#include "ExportData.h"

#include "ThreadPool.h"

#include <filesystem>
#include <memory>

//0 - partial - the old type, only the bone values that change are affected
//1 - full - all bones are recorded per animation, regardless of whether they change or not
#define ANIM_TYPE 0

//template <typename T>
struct ThreadImportStruct {
	std::vector<std::pair<std::vector<boneVertex>, std::vector<uint32_t>>> meshes;
	std::vector<std::pair<std::vector<boneVertexNoTangent>, std::vector<uint32_t>>> meshesNT;
	std::vector<std::string> meshNames;
	std::vector<std::string> meshNTNames;
	uint32_t boneCount;
	std::vector<std::string> boneVectorForPrinting;
	std::vector<bool> usefulBones;


	static uint32_t activeThreadCount;

	//not currently accounting for bones??

	void threadImport(std::string threadPath) {
		activeThreadCount++;
		//Assimp::Importer importer;
		boneCount = 101;

		//std::unique_ptr<SkeletonHandler> skeleton = std::make_unique<SkeletonHandler>(threadPath);
		SkeletonHandler skeleton{ threadPath };
		printf("after construction of skeleton \n");

		meshNames = skeleton.meshNames;
		meshNTNames = skeleton.meshNTNames;

		meshes.reserve(skeleton.meshes.size());
		for (int i = 0; i < skeleton.meshes.size(); i++) {
			meshes.emplace_back(skeleton.meshes[i].first, skeleton.meshes[i].second);
		}

		meshesNT.reserve(skeleton.meshesNT.size());
		for (int i = 0; i < skeleton.meshesNT.size(); i++) {
			meshesNT.emplace_back(skeleton.meshesNT[i].first, skeleton.meshesNT[i].second);
		}
		if (meshNames.size() != meshes.size()) {
			printf("size mismatch between names of meshes \n");
		}
		if (meshNTNames.size() != meshesNT.size()) {
			printf("size mistmatch between meshesNT \n");
		}


		auto boneMap = skeleton.GetBoneInfoMap();
		boneVectorForPrinting.resize(boneMap.size());
		for (auto iter = boneMap.begin(); iter != boneMap.end(); iter++) {
			boneVectorForPrinting[iter->second.id] = iter->first;
			//printf("bone id : name ~ %d : %s \n", iter->second.id, iter->first.c_str());
		}
		usefulBones.resize(boneCount, false);
		for (int i = 0; i < skeleton.usefulBone.size(); i++) {
			usefulBones[i] = skeleton.usefulBone[i];
			if (skeleton.usefulBone[i]) {
				printf("skeleton in threadIMport has a useful bone \n");
			}
		}
		/*
		for (int i = 0; i < boneVectorForPrinting.size(); i++) {
			printf("bone id : name ~ %d :% s \n", i, boneVectorForPrinting[i].c_str());
		}
		*/
		//usefulBones.resize(skeleton->GetBoneCount(), false);
		activeThreadCount--;
		printf("thread improt finished : %s - threadsRemaining : %d \n", threadPath.c_str(), activeThreadCount);
		//return; //shitty 
	}
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
	static void batchImporter() {
		/*
		Assimp::LogStream* logStreamPls = Assimp::LogStream::createDefaultStream(aiDefaultLogStream_STDOUT);
		if (logStreamPls == nullptr) {
			printf("LOG STREAM FAILED TO CREATE \n");
		}
		else {
			printf("log stream created \n");
		}
		if (Assimp::DefaultLogger::get() == nullptr) {
			printf("default logger is null \n");
		}

		Assimp::DefaultLogger::create();
		if (Assimp::DefaultLogger::get()->attachStream(logStreamPls, Assimp::Logger::LogSeverity::VERBOSE) &&
			Assimp::DefaultLogger::get()->attachStream(logStreamPls, Assimp::Logger::LogSeverity::DEBUGGING &&
				Assimp::DefaultLogger::get()->attachStream(logStreamPls, Assimp::Logger::LogSeverity::NORMAL)
			)
			) {
			printf("log stream attached to default logger \n");
		}
		else {
			printf("DEFAULT LOG STREAM WAS NOT ATTACHED \n");
		}
		*/
		ThreadImportStruct threadImport;
		threadImport.threadImport("model\\batch\\__0.fbx");
		threadImport.print();

		return;

		printf("thread concurrency : %d \n", std::thread::hardware_concurrency());
		ThreadImportStruct::activeThreadCount = 0;
		std::vector<ThreadImportStruct> threadImportStructs;
		uint32_t fileCount = 0;
		for (const auto& entry : std::filesystem::directory_iterator("model\\batch\\")) {
			//threadImportStructs.push_back({});
			//printf("testing individual imports : %d \n", fileCount);
			//threadImportStructs.back().threadImport(entry.path().generic_string());

			fileCount++;
		}
		//return; //temp testing
		threadImportStructs.resize(fileCount);
		ThreadPool threadPool(std::max(fileCount, std::thread::hardware_concurrency() - 1));



		uint32_t fileIterator = 0;
		std::string animPath = "";

		for (const auto& entry : std::filesystem::directory_iterator("model\\batch\\")) {
			std::string tempAnimString = entry.path().generic_string();
			if (tempAnimString.find("anim") != tempAnimString.npos) {
				if (animPath.length() > 3) { printf("anim path was already initialized? \n"); }
				animPath = tempAnimString;
				threadImportStructs.pop_back();
				continue;
			}
			threadPool.enqueueVoid(&ThreadImportStruct::threadImport, &threadImportStructs[fileIterator], entry.path().generic_string());
			fileIterator++;
		}


		threadPool.waitForCompletion();
		printf("after finished waiting for thread pool \n");

		DataManager dataManager(animPath, "tester", 1);



		uint32_t meshCount = 0;
		uint32_t meshNTCount = 0;
		uint32_t meshNameCount = 0;
		uint32_t meshNTNameCount = 0;
		for (int i = 0; i < threadImportStructs.size(); i++) {
			meshCount += threadImportStructs[i].meshes.size();
			meshNTCount += threadImportStructs[i].meshesNT.size();
			meshNameCount += threadImportStructs[i].meshNames.size();
			meshNTNameCount += threadImportStructs[i].meshNames.size();
		}
		dataManager.exportData.meshExport.meshes.reserve(meshCount);
		dataManager.exportData.meshNTExport.meshesNT.reserve(meshNTCount);
		dataManager.exportData.setVersionTracker("3.0.0");
		dataManager.exportData.nameExport.meshNames.reserve(meshNameCount);
		dataManager.exportData.nameExport.meshNTNames.reserve(meshNTNameCount);

		for (int i = 0; i < threadImportStructs.size(); i++) {
			if (threadImportStructs[i].meshes.size() != threadImportStructs[i].meshNames.size()) {
				printf("ERROR : mesh to name size mismatch - %d:%d \n", threadImportStructs[i].meshes.size(), threadImportStructs[i].meshNames.size());
				assert(threadImportStructs[i].meshes.size() == threadImportStructs[i].meshNames.size());
			}
			if (threadImportStructs[i].meshesNT.size() != threadImportStructs[i].meshNTNames.size()) {
				printf("ERROR : meshNT to name size mismatch - %d:%d \n", threadImportStructs[i].meshesNT.size(), threadImportStructs[i].meshNTNames.size());
				assert(threadImportStructs[i].meshesNT.size() == threadImportStructs[i].meshNTNames.size());
			}
			for (int j = 0; j < threadImportStructs[i].meshes.size(); j++) {
				dataManager.exportData.meshExport.meshes.push_back(threadImportStructs[i].meshes[j]);
				dataManager.exportData.nameExport.meshNames.push_back(threadImportStructs[i].meshNames[j]);
			}
			for (int j = 0; j < threadImportStructs[i].meshesNT.size(); j++) {
				dataManager.exportData.meshNTExport.meshesNT.push_back(threadImportStructs[i].meshesNT[j]);
				dataManager.exportData.nameExport.meshNTNames.push_back(threadImportStructs[i].meshNTNames[j]);
			}
			if (dataManager.skeleton->usefulBone.size() != threadImportStructs[i].usefulBones.size()) {
				printf("useful bone size mismatch - %d:%d \n", dataManager.skeleton->usefulBone.size(), threadImportStructs[i].usefulBones.size());
			}
			assert(dataManager.skeleton->usefulBone.size() == threadImportStructs[i].usefulBones.size());
			for (int j = 0; j < threadImportStructs[i].usefulBones.size(); j++) {
				if (threadImportStructs[i].usefulBones[j]) {
					printf("threadIMprotStruct has a useful bone \n");
				}
				dataManager.batchUsefulBones[j] = dataManager.batchUsefulBones[j] || threadImportStructs[i].usefulBones[j];
			}
		}
		printf("mesh,name - NT - %d:%d - %d:%d \n", dataManager.exportData.meshExport.meshes.size(), dataManager.exportData.nameExport.meshNames.size(),
			dataManager.exportData.meshNTExport.meshesNT.size(), dataManager.exportData.nameExport.meshNTNames.size()
		);
		printf("full processing data manager \n");
		dataManager.fullProcess();

	}
	DataManager(std::string fileLocation, std::string exportLocation) :importPath{ fileLocation } {
		//Assimp::Importer importer;
		if (fileLocation.find("ellen") != fileLocation.npos) {
			printf("found ellen \n");
			printf("yo wtf \n");
			boneCount = 101; /* THIS HAS TO BE SET MANUALLY BECAUSE BONE COUNT ISN'T PROCESSED UNTIL ITS TOO lATE*/
		}
		else if (fileLocation.find("skeleGuy") != fileLocation.npos) {
			printf("found skeleGuy \n");
			boneCount = 79;
		}
		else if (fileLocation.find("BaseHuman") != fileLocation.npos) {
			printf("found basehuman \n");
			boneCount = 62;
		}
		else if (fileLocation.find("ellMesh") != fileLocation.npos) {
			printf("found ellMesh \n");
			boneCount = 101;
		}
		else if (fileLocation.find("ellBASE") != fileLocation.npos) {
			printf("found ellBASE \n");
			boneCount = 101;
		}
		else if (fileLocation.find("katana") != fileLocation.npos) {
			boneCount = 1;
		}
		else if (fileLocation.find("spear") != fileLocation.npos) {
			boneCount = 1;
		}
		else if (fileLocation.find("spear") != fileLocation.npos) {
			boneCount = 1;
		}
		else if (fileLocation.find("Corruption") != fileLocation.npos) {
			printf("found Corrupton : DataManager \n");
			boneCount = 62;
		}
		else if (fileLocation.find("PlayerMan") != fileLocation.npos) {
			printf("found PlayerMan : DataManager \n");
			boneCount = 62;
		}
		else if (fileLocation.find("skeleMon") != fileLocation.npos) {
			printf("found skelemon \n");
			boneCount = 68;
		}
		else if (fileLocation.find("LichKing") != fileLocation.npos) {
			printf("found lich king \n");
			boneCount = 11;
		}
		else if (fileLocation.find("deerMonster") != fileLocation.npos) {
			printf("found deerman \n");
			boneCount = 21;
		}
		else if (fileLocation.find("DevilMan") != fileLocation.npos) {
			printf("found devilman \n");
			boneCount = 52;
		}
		else {
			boneCount = 66;
			printf("DataManager :: COULD NOT IDENTIFY SOURCE FILE, NO USEFUL BONES SET : %s \n", fileLocation.c_str());
		}

		filePath = exportLocation;
		skeleton = std::make_unique<SkeletonHandler>(fileLocation);
		printf("after construction of skeleton \n");
		//exportData.meshNames = skeleton->meshNames;

		exportData.nameExport.meshNames = skeleton->meshNames;
		exportData.nameExport.meshNTNames = skeleton->meshNTNames;
		exportData.nameExport.meshSimpleNames = skeleton->meshSimpleNames;
		exportData.nameExport.meshNTSimpleNames = skeleton->meshNTSimpleNames;
		//exportData.meshNTNames = skeleton->meshNTNames;

		exportData.meshExport.meshes.reserve(skeleton->meshes.size());
		for (int i = 0; i < skeleton->meshes.size(); i++) {
			exportData.meshExport.meshes.emplace_back(skeleton->meshes[i].first, skeleton->meshes[i].second);
		}

		exportData.meshNTExport.meshesNT.reserve(skeleton->meshesNT.size());
		for (int i = 0; i < skeleton->meshesNT.size(); i++) {
			exportData.meshNTExport.meshesNT.emplace_back(skeleton->meshesNT[i].first, skeleton->meshesNT[i].second);
		}

		exportData.meshSimpleExport.meshesSimple.reserve(skeleton->meshesSimple.size());
		for (int i = 0; i < skeleton->meshesSimple.size(); i++) {
			exportData.meshSimpleExport.meshesSimple.emplace_back(skeleton->meshesSimple[i].first, skeleton->meshesSimple[i].second);
		}

		exportData.meshNTSimpleExport.meshesNTSimple.reserve(skeleton->meshesNTSimple.size());
		for (int i = 0; i < skeleton->meshesNTSimple.size(); i++) {
			exportData.meshNTSimpleExport.meshesNTSimple.emplace_back(skeleton->meshesNTSimple[i].first, skeleton->meshesNTSimple[i].second);
		}
		/*
		for (int i = 0; i < skeleton->meshesNT.size(); i++) {
			exportData.meshNTExport.meshesNT.emplace_back(skeleton->meshesNT[i].first, skeleton->meshesNT[i].second);
		}
		*/
		if (exportData.nameExport.meshNames.size() != exportData.meshExport.meshes.size()) {
			printf("size mismatch between names of meshes \n");
		}
		if (exportData.nameExport.meshNTNames.size() != exportData.meshNTExport.meshesNT.size()) {
			printf("size mistmatch between meshesNT \n");
		}

#if ANIM_TYPE == 0
		exportData.animExport.animations.resize(skeleton->animationNames.size(), {});
		printf("skeleton animation name size : %d \n", skeleton->animationNames.size());
#else
		exportData.fullAnim.animations.resize(skeleton->animationNames.size(), {});
#endif

		auto boneMap = skeleton->GetBoneInfoMap();
		boneVectorForPrinting.resize(boneMap.size());
		for (auto iter = boneMap.begin(); iter != boneMap.end(); iter++) {
			boneVectorForPrinting[iter->second.id] = iter->first;
			//printf("bone id : name ~ %d : %s \n", iter->second.id, iter->first.c_str());
		}
		/*
		for (int i = 0; i < boneVectorForPrinting.size(); i++) {
			printf("bone id : name ~ %d :% s \n", i, boneVectorForPrinting[i].c_str());
		}
		*/
		//usefulBones.resize(skeleton->GetBoneCount(), false);
	}

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