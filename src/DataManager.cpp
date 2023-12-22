#include "DataManager.h"
#include <fstream>

#include <thread>
uint32_t ThreadImportStruct::activeThreadCount;

//printf("after removedBoneNames \n");
/*
std::vector<glm::mat4> tempChecker = skeleton->GetFinalBoneMatrices();
for (int i = 0; i < 5; i++) {
	std::string printString = "printing matrix values? \n";
	std::string printLine = " % .1f, % .1f, % .1f, % .1f, \n";
	printf(printString.c_str());
	printf(printLine.c_str(), tempChecker[0][0], tempChecker[1][0], tempChecker[2][0], tempChecker[3][0]);
	printf(printLine.c_str(), tempChecker[0][1], tempChecker[1][1], tempChecker[2][1], tempChecker[3][1]);
	printf(printLine.c_str(), tempChecker[0][2], tempChecker[1][2], tempChecker[2][2], tempChecker[3][2]);
	printf(printLine.c_str(), tempChecker[0][3], tempChecker[1][3], tempChecker[2][3], tempChecker[3][3]);
}
*/

void DataManager::initializeTheData() {
	std::vector<int32_t> weightlessBones;
	weightlessBones = cleanWeightlessBones();
	removedBoneNames.resize(weightlessBones.size());
	//printf("before removedBoneNames \n");
	for (int i = 0; i < weightlessBones.size(); i++) {
		removedBoneNames[i].first = weightlessBones[i];
		removedBoneNames[i].second = boneVectorForPrinting[weightlessBones[i]];
	}

	//printf("initial animation names \n");
	//for (int i = 0; i < skeleton->animationNames.size(); i++) {
		//printf("%d : %s \n", i, skeleton->animationNames[i].c_str());
	//}
	printf("initial mesh names \n");
	for (int i = 0; i < exportData.nameExport.meshNames.size(); i++) {
		printf("%d : %s \n", i, exportData.nameExport.meshNames[i].c_str());
	}
	printf("initial mesh NT names \n");
	for (int i = 0; i < exportData.nameExport.meshNTNames.size(); i++) {
		printf(":%d : %s \n", i, exportData.nameExport.meshNTNames[i].c_str());
	}

#if ANIM_TYPE == 0
	std::vector<std::vector<std::vector<ExportData::boneEData>>>& animRef = exportData.animExport.animations;
#else
	std::vector<std::vector<std::vector<ExportData::bobmat4>>>& animRef = exportData.fullAnim.animations;
#endif

	for (int i = 0; i < skeleton->animationNames.size(); i++) { //per animation loop

		skeleton->switchAnimation(i);
		printf("i : name : bone count, %d : %s : %d \n", i, skeleton->animationNames[i].c_str(), skeleton->currentAnimation().getBoneSize());
		//skeleton->currentAnimation().getBoneSize();
		//float animTimeStep = (skeleton->currentAnimation().GetDuration() + 1); //useless?
		animRef[i].resize(skeleton->currentAnimation().GetDuration() + 1, {});
		printf("anim[%d] duration : %.2f \n", i, skeleton->currentAnimation().GetDuration() + 1);

		std::vector<glm::mat4> boneHolder(skeleton->GetBoneCount()); //setting the size now, all values will be changed every animation frame

		for (int j = 0; j < animRef[i].size(); j++) { //per each frame of the animation loop, this should hold the final bone matrix, trim useless bones
			skeleton->setCurrentAnimation(j); //this should set the time of the current animation to the step we want to record
			//now lets check bones and track if its changed. if it hasn't changed, don't mark it useful

			boneHolder = skeleton->GetFinalBoneMatrices();

			
			for (int z = weightlessBones.size() - 1; z >= 0; z--) {
				boneHolder.erase(boneHolder.begin() + weightlessBones[z]);
				if (j == 0 && skeleton->animationNames[i] == "Armature|idle") {
					//cleaning bone names
					boneVectorForPrinting.erase(boneVectorForPrinting.begin() + weightlessBones[z]);
				}
				else if(j == 0 && skeleton->animationNames[i] == "Armature|Idle") {
					//cleaning bone names
					boneVectorForPrinting.erase(boneVectorForPrinting.begin() + weightlessBones[z]);
				}
			}
#if ANIM_TYPE == 0
			if (j == 0 && (skeleton->animationNames[i].find("idle") != skeleton->animationNames[i].npos)) {
				printf("set default bones : %s \n", skeleton->animationNames[i].c_str());
				setDefaultBones(boneHolder);
			}
			else if (j == 0 && (skeleton->animationNames[i].find("Idle") != skeleton->animationNames[i].npos)) {
				printf("set default bones capital IDLE: %s \n", skeleton->animationNames[i].c_str());
				setDefaultBones(boneHolder);
			}
#endif

			animRef[i][j].reserve(boneHolder.size());

			for (int k = 0; k < boneHolder.size(); k++) {
#if ANIM_TYPE == 0
				animRef[i][j].emplace_back(k, boneHolder[k]);
#else
				animRef[i][j].emplace_back(boneHolder[k]);
#endif
			}
		}
	}
	printf("skeleton hand bone value : %d \n", skeleton->getHandBone());
	if (skeleton->getHandBone() != -1) {
#if ANIM_TYPE == 0
		exportData.animExport.handBone = skeleton->getHandBone();
		for (int i = 0; i < weightlessBones.size(); i++) {
			if (exportData.animExport.handBone > weightlessBones[i]) {
				printf("bumping up hand bone index : %d \n", exportData.animExport.handBone);
				exportData.animExport.handBone--;
			}
		}
#else
		exportData.fullAnim.handBone = skeleton->getHandBone();
		for (int i = 0; i < weightlessBones.size(); i++) {
			if (exportData.fullAnim.handBone > weightlessBones[i]) {
				printf("bumping up hand bone index : %d \n", exportData.fullAnim.handBone);
				exportData.fullAnim.handBone--;
			}
		}
#endif
	}
}

void DataManager::checkBoneSequence() {
	//this just checks if the bones are in order
	std::vector<int> foundBoneIDs;
	for (int j = 0; j < exportData.meshExport.meshes.size(); j++) {
		for (int k = 0; k < exportData.meshExport.meshes[j].first.size(); k++) {
			for (int x = 0; x < MAX_BONE_INFLUENCE; x++) {
				bool found = false;
				for (int i = 0; i < foundBoneIDs.size(); i++) {
					if (exportData.meshExport.meshes[j].first[k].m_BoneIDs[x] == foundBoneIDs[i]) {
						found = true;
					}
				}
				if (!found) {
					foundBoneIDs.push_back(exportData.meshExport.meshes[j].first[k].m_BoneIDs[x]);
				}
			}
		}
	}
	
	for (int k = 0; k < boneCount; k++) {
		bool found = false;
		for (int i = 0; i < foundBoneIDs.size();) {
			if (k == foundBoneIDs[i]) {
				found = true;
				//printf("found bone ID : %d \n", foundBoneIDs[i]);
				foundBoneIDs.erase(foundBoneIDs.begin() + i);
			}
			else {
				i++;
			}
		}
		if (!found) {
			printf("didnt find bone id : %d \n", k);
		}
	}
	std::vector<int> foundBoneIDsNT;
	for (int j = 0; j < exportData.meshNTExport.meshesNT.size(); j++) {
		for (int k = 0; k < exportData.meshNTExport.meshesNT[j].first.size(); k++) {
			for (int x = 0; x < MAX_BONE_INFLUENCE; x++) {
				bool found = false;
				for (int i = 0; i < foundBoneIDsNT.size(); i++) {
					if (exportData.meshNTExport.meshesNT[j].first[k].m_BoneIDs[x] == foundBoneIDsNT[i]) {
						found = true;
					}
				}
				if (!found) {
					foundBoneIDsNT.push_back(exportData.meshNTExport.meshesNT[j].first[k].m_BoneIDs[x]);
				}
			}
		}
	}
	for (int k = 0; k < boneCount; k++) {
		bool found = false;
		for (int i = 0; i < foundBoneIDsNT.size();) {
			if (k == foundBoneIDsNT[i]) {
				found = true;
				//printf("found bone ID : %d \n", foundBoneIDs[i]);
				foundBoneIDsNT.erase(foundBoneIDsNT.begin() + i);
				//break;
			}
			else {
				i++;
			}
		}
		if (!found) {
			printf("didnt find bone id : %d \n", k);
		}
	}
}

void DataManager::checkWonkyBones() {
	for (int j = 0; j < exportData.meshExport.meshes.size(); j++) {
		for (int k = 0; k < exportData.meshExport.meshes[j].first.size(); k++) {
			bool hasAPositive = false;
			for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
				hasAPositive |= exportData.meshExport.meshes[j].first[k].m_BoneIDs[i] >= 0;
				if (exportData.meshExport.meshes[j].first[k].m_BoneIDs[i] < -1) {
					printf("MESH[%d] AT VERTEX[%d] AT BONE[%d] HAS A WONKY NUMBER : %d \n", j, k, i, exportData.meshExport.meshes[j].first[k].m_BoneIDs[i]);
				}
			}
			/*
			if (
				((exportData.meshExport.meshes[j].first[k].m_BoneIDs[0] == exportData.meshExport.meshes[j].first[k].m_BoneIDs[1]) && (exportData.meshExport.meshes[j].first[k].m_BoneIDs[0] != -1)) ||
				(exportData.meshExport.meshes[j].first[k].m_BoneIDs[0] == exportData.meshExport.meshes[j].first[k].m_BoneIDs[2]) && (exportData.meshExport.meshes[j].first[k].m_BoneIDs[0] != -1) ||
				(exportData.meshExport.meshes[j].first[k].m_BoneIDs[0] == exportData.meshExport.meshes[j].first[k].m_BoneIDs[3]) && (exportData.meshExport.meshes[j].first[k].m_BoneIDs[0] != -1) ||

				(exportData.meshExport.meshes[j].first[k].m_BoneIDs[1] == exportData.meshExport.meshes[j].first[k].m_BoneIDs[2]) && (exportData.meshExport.meshes[j].first[k].m_BoneIDs[1] != -1) ||
				(exportData.meshExport.meshes[j].first[k].m_BoneIDs[1] == exportData.meshExport.meshes[j].first[k].m_BoneIDs[3]) && (exportData.meshExport.meshes[j].first[k].m_BoneIDs[1] != -1) ||

				(exportData.meshExport.meshes[j].first[k].m_BoneIDs[2] == exportData.meshExport.meshes[j].first[k].m_BoneIDs[3]) && (exportData.meshExport.meshes[j].first[k].m_BoneIDs[2] != -1)) {
				*
				printf("importpath : %s \n", importPath.c_str());
				printf("this shit has a double weighted bone??? j:k::0:1:2:3 - %d:%d::%d:%d:%d:%d \n", j, k,
					exportData.meshExport.meshes[j].first[k].m_BoneIDs[0],
					exportData.meshExport.meshes[j].first[k].m_BoneIDs[1],
					exportData.meshExport.meshes[j].first[k].m_BoneIDs[2],
					exportData.meshExport.meshes[j].first[k].m_BoneIDs[3]
				);
				
				*
			}
			*/

			if (!hasAPositive) {
				printf("MESH[%d] AT VERTEX[%d] HAS NO POSITIVE BONE VALUE, WEIGHTLESS VERTEX \n", j, k);
				for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
					printf("%d  ", exportData.meshExport.meshes[j].first[k].m_BoneIDs[i]);
				}
				printf("\n");
			}
		}
	}
}

std::vector<int32_t> DataManager::cleanWeightlessBones() {
	std::vector<int32_t> cleanTheseBones; //cant think of a better name
	for (int i = 0; i < batchUsefulBones.size(); i++) {
		if (!batchUsefulBones[i]) {
			cleanTheseBones.push_back(i);
			printf("bone %d doesn't have any effect on vertices, can trash it \n", i);
		}
	}
	//printf("after clearing weightless bones \n");
	int32_t biggestBone = 0;
	for (int i = 0; i < cleanTheseBones.size(); i++) {
		//printf("bone values : %d \n", cleanTheseBones[i]); these values are in the final report
		if (cleanTheseBones[i] < biggestBone) {
			printf(" \n \n clean these bones is unsorted, need to sort \n \n \n");
		}
		biggestBone = cleanTheseBones[i];
	}
	
	
	printf("checking bone values before cleaning \n");
	checkWonkyBones();
	


	//shift boneVertex boneIDs forward from here
	for (int j = 0; j < exportData.meshExport.meshes.size(); j++) {
		for (int k = 0; k < exportData.meshExport.meshes[j].first.size(); k++) {
			for (int x = 0; x < MAX_BONE_INFLUENCE; x++) { //i kinda wanna change MAX_BONE_INFLUENCE to 2 or 3 but ill worry about that later
				//std::vector<unsigned int> cleanBonesCopy = cleanTheseBones;
				//uint32_t cleanedCount = 0;
				for (int i = 0; i < cleanTheseBones.size(); i++) { //im not sure if the ordering of these for loops matters or not
					if (exportData.meshExport.meshes[j].first[k].m_BoneIDs[x] > (cleanTheseBones[i] - i)) { //i need to do this to animation bone ids too
						//printf("bone id, cleanBoneValue - %d:%d \n", exportData.meshExport.meshes[j].vertices[k].m_BoneIDs[x], cleanTheseBones[i]);
						exportData.meshExport.meshes[j].first[k].m_BoneIDs[x]--;
						//cleanedCount++;
					}
				}
			}
		}
	}
	for (int j = 0; j < exportData.meshNTExport.meshesNT.size(); j++) {
		for (int k = 0; k < exportData.meshNTExport.meshesNT[j].first.size(); k++) {
			for (int x = 0; x < MAX_BONE_INFLUENCE; x++) { //i kinda wanna change MAX_BONE_INFLUENCE to 2 or 3 but ill worry about that later
				//std::vector<unsigned int> cleanBonesCopy = cleanTheseBones;
				//uint32_t cleanedCount = 0;
				for (int i = 0; i < cleanTheseBones.size(); i++) { //im not sure if the ordering of these for loops matters or not
					if (exportData.meshNTExport.meshesNT[j].first[k].m_BoneIDs[x] > (cleanTheseBones[i] - i)) { //i need to do this to animation bone ids too
						//printf("bone id, cleanBoneValue - %d:%d \n", exportData.meshExport.meshes[j].vertices[k].m_BoneIDs[x], cleanTheseBones[i]);
						exportData.meshNTExport.meshesNT[j].first[k].m_BoneIDs[x]--;
					}
				}
			}
		}
	}

	return cleanTheseBones;
}

void DataManager::trimAnimations() {
	std::vector<uint32_t> boneRemovalCount;
	boneRemovalCount.resize(exportData.animExport.animations[0][0].size(), 0);
	for (int i = 0; i < exportData.animExport.animations.size(); i++) { //per animation loop
		if (skeleton->animationNames[i].find("mainMenu") != skeleton->animationNames[i].npos) {
			printf("skipping the trim on mainMenu \n");
			continue;
		}
		/*
		if (skeleton->animationNames[i].find("Walking") != skeleton->animationNames[i].npos) {
			printf("skipping the trim on Walking \n");
			continue;
		}
		*/
		if (skeleton->animationNames[i].find("Crouch") != skeleton->animationNames[i].npos) {
			printf("skipping the trim on Crouch \n");
			continue;
		}
		std::vector<bool> usefulBones;
		
		

		usefulBones.resize(exportData.animExport.animations[i][0].size(), false);
		printf("size of animation[%d]:%d \n", i, exportData.animExport.animations[i][0].size());
		

		for (int j = 0; j < exportData.animExport.animations[i].size(); j++) {
			//now lets check bones and track if its changed. if it hasn't changed, don't mark it useful

			for (int k = 0; k < exportData.animExport.animations[i][j].size(); k++) {
				if (j > 0) {
					//if (!memcmp(exportData.animExport.animations[i][j - 1][k].boneTransform.bmat4, exportData.animExport.animations[i][j][k].boneTransform.bmat4, sizeof(exportData.animExport.animations[i][j - 1][k].boneTransform.bmat4))) {
					if (!(exportData.animExport.animations[i][j - 1][k].boneTransform == exportData.animExport.animations[i][j][k].boneTransform)) {
						usefulBones[k] = true; //if its not useful at the end, get rid of it
						//printf("\n \n printing a bone comparison %d : %d \n", exportData.animExport.animations[i][j - 1][k].boneID, exportData.animExport.animations[i][j][k].boneID);
						//exportData.animExport.animations[i][j - 1][k].boneTransform.print(); exportData.animExport.animations[i][j][k].boneTransform.print();
					}
				}
			}
		}
		uint32_t usefulBoneCount = 0;
		for (int j = 0; j < usefulBones.size(); j++) {
			usefulBoneCount += usefulBones[j];
		}
		printf("animation %d usefulbonecount : %d \n", i, usefulBoneCount);
		for (int j = 0; j < usefulBones.size(); j++) { //clear out useless bones
			if (!usefulBones[j]) {
				boneRemovalCount[j]++;
				for (int k = 0; k < exportData.animExport.animations[i].size(); k++) {
					for (int z = 0; z < exportData.animExport.animations[i][k].size(); z++) { //i have no idea why the first 2 bones aren't changing. maybe just bad animation
						if (exportData.animExport.animations[i][k][z].boneID == j) {
							//printf("erasing a useless bone, izj %d:%d:%d \n", i, z, j);
							if (k == 0) {
								//printf("useless bone, ijkz %d:%d:%d:%d ~ boneID : %d \n", i, j, k, z, exportData.animExport.animations[i][k][z].boneID);
							}
							exportData.animExport.animations[i][k].erase(exportData.animExport.animations[i][k].begin() + z);
							z--;
							break;
						}
					}
				}
			}
			else {
				
				//printf("bone %d is useful in animation %d \n", j, i);
			}
		}
	}
	//printf("animation count : %d \n", exportData.animExport.animations.size());
	//for (int i = 0; i < boneRemovalCount.size(); i++) {
		//printf("bone removal count : %d : %d \n", i, boneRemovalCount[i]);
	//}
	//printf("animation 7 bone id? %d \n", exportData.animExport.animations[7][0][0].boneID);
}
void DataManager::cleanAnimations() {
	//checking size of bone transform matrices after cleaning useless bones

	//int animationTracker = 0;
	printf("beginning of clean animations \n");
	for (int i = 0; i < exportData.animExport.animations.size(); i++) { //animation count


		if (exportData.animExport.animations[i][0].size() <= 2) {

			//the below for j loop can be commented out, just for testing purposes
			for (int j = 0; j < exportData.animExport.animations[i].size(); j++) {
				if (exportData.animExport.animations[i][j].size() != 0) {
					//printf("error cleaning bones at animation %d, frame %d, size of animation %d \n", i, j, exportData.animExport.animations[i][j].size());
					for (int k = 0; k < exportData.animExport.animations[i][j].size(); k++) {
						//exportData.animExport.animations[i][j][k].boneID;
						//skeleton->GetBoneInfoMap().at(exportData.animExport.animations[i][j][k].boneID).
						auto boneMap = skeleton->GetBoneInfoMap();
						for (auto iter = boneMap.begin(); iter != boneMap.end(); iter++) {
							if (iter->second.id == exportData.animExport.animations[i][j][k].boneID) {
								//printf("bone id:name? %d : %s \n", iter->second.id, iter->first.c_str());
							}
							//printf("bone id : name ~ %d : %s \n", iter->second.id, iter->first.c_str());
						}
					}
				}
			}
			printf("removing animation i %d \n", i);
			exportData.animExport.animations.erase(exportData.animExport.animations.begin() + i);
			printf("removing skele name at %s \n", skeleton->getAnimName(i).c_str());
			skeleton->animationNames.erase(skeleton->animationNames.begin() + i);
			i--;
		}
	}
	printf("end of animation clearing \n");
	
	uint32_t biggestBoneCount = 0;
	for (int i = 0; i < exportData.animExport.animations.size(); i++) {
		printf("bone count in animation %d ~ %d \n", i, exportData.animExport.animations[i][0].size());
		//this SHOULD be counting every single affected bone. if its a high tier bone being affected, the matrix will be more filled out. i.e. if walking moves the root, it will move the entire body. 
		//maybe do the root separately. or maybe apply lower body first, then upper body animations
		//DO ROOT SEPARATELY, THEN LOWER BODY SHOULDNT BE AFFECTING UPPER BODY
	}
}

void DataManager::adjustBones() {
	for (int i = 0; i < exportData.meshExport.meshes.size(); i++) {
		for (int j = 0; j < exportData.meshExport.meshes[i].first.size(); j++) {
#define BONE_WEIGHT_NORMALIZE false
#if BONE_WEIGHT_NORMALIZE
			//float boneWeightLength = 0;
#endif
			for (int k = 0; k < MAX_BONE_INFLUENCE; k++) {
				//normalize? i dont know if its an issue
#if BONE_WEIGHT_NORMALIZE
				if (exportData.meshExport.meshes[i].vertices[j].m_BoneIDs[k] >= 0) {
					boneWeightLength += exportData.meshExport.meshes[i].vertices[j].m_Weights[k] * exportData.meshExport.meshes[i].vertices[j].m_Weights[k];
				}
				else {
					exportData.meshExport.meshes[i].vertices[j].m_BoneIDs[k] = 0;
				}
#else
				if (exportData.meshExport.meshes[i].first[j].m_BoneIDs[k] < 0) {
					exportData.meshExport.meshes[i].first[j].m_BoneIDs[k] = 0;
				}
#endif
			}
#if BONE_WEIGHT_NORMALIZE
			boneWeightLength = sqrt(boneWeightLength);
			
			if (std::abs(boneWeightLength - 1.0f) > .001f) {
				printf("BONE WEIGHTS ARE NOT NORMALIZED %.5f \n \n", boneWeightLength);
				for (int k = 0; k < MAX_BONE_WEIGHTS; k++) {
					exportData.meshExport.meshes[i].vertices[j].m_Weights[k] /= boneWeightLength;
				}
			}
			
#endif
		}
	}

	for (int i = 0; i < exportData.meshNTExport.meshesNT.size(); i++) {
		for (int j = 0; j < exportData.meshNTExport.meshesNT[i].first.size(); j++) {
#define BONE_WEIGHT_NORMALIZE false
#if BONE_WEIGHT_NORMALIZE
			//float boneWeightLength = 0;
#endif
			for (int k = 0; k < MAX_BONE_INFLUENCE; k++) {
				//normalize? i dont know if its an issue
#if BONE_WEIGHT_NORMALIZE
				if (exportData.meshNTExport.meshesNT[i].vertices[j].m_BoneIDs[k] >= 0) {
					boneWeightLength += exportData.meshNTExport.meshesNT[i].vertices[j].m_Weights[k] * exportData.meshNTExport.meshesNT[i].vertices[j].m_Weights[k];
				}
				else {
					exportData.meshNTExport.meshesNT[i].vertices[j].m_BoneIDs[k] = 0;
				}
#else
				if (exportData.meshNTExport.meshesNT[i].first[j].m_BoneIDs[k] < 0) {
					exportData.meshNTExport.meshesNT[i].first[j].m_BoneIDs[k] = 0;
				}
#endif
			}
#if BONE_WEIGHT_NORMALIZE
			boneWeightLength = sqrt(boneWeightLength);

			if (std::abs(boneWeightLength - 1.0f) > .001f) {
				printf("BONE WEIGHTS ARE NOT NORMALIZED %.5f \n \n", boneWeightLength);
				for (int k = 0; k < MAX_BONE_WEIGHTS; k++) {
					exportData.meshNTExport.meshesNT[i].vertices[j].m_Weights[k] /= boneWeightLength;
				}
			}

#endif
		}
	}
}

void DataManager::Logging() {
	printf("remaining animation names, printing to file \n");
	std::string animPath = importPath;
	size_t extPos = animPath.find_first_of(".");
	animPath = animPath.substr(0, extPos);
	animPath += "_animNames.txt";
	std::ofstream animFile{ animPath, std::ios::trunc };

	for (int i = 0; i < skeleton->animationNames.size(); i++) {
		printf("%d : %s \n", i, skeleton->animationNames[i].c_str());
		animFile << skeleton->animationNames[i] << "\n";
	}
#if ANIM_TYPE == 0
	animFile << "handBone : " << exportData.animExport.handBone << "\n";
#else
	animFile << "full anim handBone : " << exportData.fullAnim.handBone << "\n";
#endif

	animFile << "-- mesh names -- \n";
	for (int i = 0; i < exportData.nameExport.meshNames.size(); i++) {
		animFile << i << ":" << exportData.nameExport.meshNames[i] << '\n';
	}
	animFile << "-- meshNT names -- \n";
	for (int i = 0; i < exportData.nameExport.meshNTNames.size(); i++) {
		animFile << i << ":" << exportData.nameExport.meshNTNames[i] << '\n';
	}

	animFile << "\n -- used bones -- \n";
	for (int i = 0; i < boneVectorForPrinting.size(); i++) {
		animFile << i << ":" << boneVectorForPrinting[i] << '\n';
	}
	animFile << "\n -- partial animation, duration, boneCount -- \n";
	for (int i = 0; i < exportData.animExport.animations.size(); i++) {
		animFile << i << ":" << exportData.animExport.animations[i].size() << ":" << exportData.animExport.animations[i][0].size() << '\n';
	}
	animFile << "\n -- full animation, duration, boneCount -- \n";
	for (int i = 0; i < exportData.fullAnim.animations.size(); i++) {
		animFile << i << ":" << exportData.fullAnim.animations[i].size() << ":" << exportData.fullAnim.animations[i][0].size() << '\n';
	}

	animFile << "\n \n -- trimmed bones -- \n";

	for (int i = 0; i < removedBoneNames.size(); i++) {
		animFile << removedBoneNames[i].first << ":" << removedBoneNames[i].second << "\n";
	}
	animFile.close();

	printf("partial default anim size? : %d \n", exportData.animExport.defaultBoneValues.size());
}

void DataManager::fileWrite() {
	/*
	for (int j = 0; j < exportData.meshExport.meshes.size(); j++) {
		for (int k = 0; k < exportData.meshExport.meshes[j].first.size(); k++) {
			bool hasAPositive = false;
			for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
				for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
					if (exportData.meshExport.meshes[j].first[k].m_BoneIDs[i] > exportData.animExport.defaultBoneValues.size()) {
						printf("MESH[%d] AT VERTEX[%d] AT BONE[%d] HAS A WONKY NUMBER GREATER THAN SIZE : %d \n", j, k, i, exportData.meshExport.meshes[j].first[k].m_BoneIDs[i]);
					}
				}
			}
		}
	}
	*/
	printf("DEFAULT BONE MATRIX SIZE : %d ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ \n", exportData.animExport.defaultBoneValues.size());

	//printBobMat4(exportData.defaultBoneValues, "default bones %d \n");

	//printAnimationBoneSet(exportData.animExport.animations.back()[0], "back set %d \n");


	/* FOR THREADING
		// Lambda expression begins
        [](float a, float b) {
            return (std::abs(a) < std::abs(b));
        } // end of lambda expressio
	*/

	exportData.setVersionTracker("2.0.0");

	
	if (exportData.meshExport.meshes.size() > 0) {
		printf("writing mesh count : %d \n", exportData.meshExport.meshes.size());
		std::ofstream outFile(filePath + "_mesh.ewe", std::ofstream::binary);
		if (outFile.is_open()) {
			boost::archive::binary_oarchive binary_output_archive(outFile, boost::archive::no_header);
			binary_output_archive& exportData.meshExport;
			printf("archived mesh file to : %s%s \n", filePath.c_str(), "_mesh.ewe");

			outFile.close();
		}
		else {
			printf("FAILED TO OPEN MESH FILE \n");
		}
	}
	else {
		printf("mesh size == 0? \n");
	}
	if (exportData.meshNTExport.meshesNT.size() > 0) {
		printf("wrinting mesh nt count : %d \n", exportData.meshNTExport.meshesNT.size());
		std::ofstream outFile(filePath + "_meshNT.ewe", std::ofstream::binary);
		outFile.open(filePath + "_meshNT.ewe", std::ofstream::binary);
		if (outFile.is_open()) {
			boost::archive::binary_oarchive boaNT(outFile, boost::archive::no_header);
			boaNT& exportData.meshNTExport;
			printf("archived mesh NT to file : %s%s \n", filePath.c_str(), "_meshNT.ewe");

			outFile.close();
		}
		else{
			printf("FAILED TO OPEN MESH NT FILE \n");
		}
	}
	else {
		printf("mesh nt size == 0? \n");
	}
	if (exportData.animExport.animations.size() == 0) {
		printf(" WHY IS EXPORT DATA : ANIMATIONS 0 \n");
	}
	else {
		printf("writing partial anim \n");
		std::ofstream outFile(filePath + "_anim.ewe", std::ofstream::binary);
		if (outFile.is_open()) {
			boost::archive::binary_oarchive boaAnim(outFile, boost::archive::no_header);
			boaAnim& exportData.animExport;
			printf("archived anim file to : %s%s \n", filePath.c_str(), "_anim.ewe");
			outFile.close();
		}
		else{
			printf("FAILED TO OPEN PARTIAL ANIM FILE \n");
		}
	}
	//before
	if (exportData.fullAnim.animations.size() == 0) {
		printf(" full anim : size 0 on filewrite \n");
	}
	else {
		printf("writing full anim \n");
		std::ofstream outFile(filePath + "_fullAnim.ewe", std::ofstream::binary);
		if (outFile.is_open()) {
			boost::archive::binary_oarchive boaFullAnim(outFile, boost::archive::no_header);
			boaFullAnim& exportData.fullAnim;
			printf("archived anim file to : %s%s \n", filePath.c_str(), "_fullAnim.ewe");
			outFile.close();
		}
		else {
			printf("FAILED TO OPEN FULL ANIM FILE \n");
		}
	}
	//after
	if (exportData.meshSimpleExport.meshesSimple.size() > 0) {
		printf("writing mesh count : %d \n", exportData.meshSimpleExport.meshesSimple.size());
		std::ofstream outFile(filePath + "_simpleMesh.ewe", std::ofstream::binary);
		if (outFile.is_open()) {
			boost::archive::binary_oarchive binary_output_archive(outFile, boost::archive::no_header);
			binary_output_archive& exportData.meshSimpleExport;
			printf("archived mesh file to : %s%s \n", filePath.c_str(), "_mesh.ewe");

			outFile.close();
		}
		else {
			printf("FAILED TO OPEN MESH FILE \n");
		}
	}
	else {
		printf("mesh simple size == 0? \n");
	}
	if (exportData.meshNTSimpleExport.meshesNTSimple.size() > 0) {
		printf("writing mesh count : %d \n", exportData.meshNTSimpleExport.meshesNTSimple.size());
		std::ofstream outFile(filePath + "_simpleNTMesh.ewe", std::ofstream::binary);
		if (outFile.is_open()) {
			boost::archive::binary_oarchive binary_output_archive(outFile, boost::archive::no_header);
			binary_output_archive& exportData.meshNTSimpleExport;
			printf("archived mesh file to : %s%s \n", filePath.c_str(), "_mesh.ewe");

			outFile.close();
		}
		else {
			printf("FAILED TO OPEN MESH FILE \n");
		}
	}
	else {
		printf("mesh size == 0? \n");
	}

	//NameExportData nameExport;
	if ((exportData.meshExport.meshes.size() > 0) || (exportData.meshNTExport.meshesNT.size() > 0) || (exportData.nameExport.meshSimpleNames.size() > 0) || (exportData.nameExport.meshNTSimpleNames.size() > 0)) {
		std::ofstream nameFile(filePath + "_Names.ewe", std::ofstream::binary);
		if (nameFile.is_open()) {
			boost::archive::binary_oarchive binary_output_archive(nameFile, boost::archive::no_header);
			binary_output_archive& exportData.nameExport;
			printf("archived mesh file to : %s%s \n", filePath.c_str(), "_Names.ewe");

			nameFile.close();
		}
		else {
			printf("FAILED TO OPEN MESH FILE \n");
		}
	}



	/* OLD METHOD
	boost::archive::binary_oarchive binary_output_archive(outFile, boost::archive::no_header);
	binary_output_archive& exportData;
	printf("archived file to %s \n", exportFile.c_str());
	outFile.close();
	*/
}
/*
void DataManager::fileImportAndCompare() {
	ExportData importData;
	std::ifstream inFile(filePath, std::ifstream::binary);
	boost::archive::binary_iarchive binary_input_archive(inFile, boost::archive::no_header);
	binary_input_archive& importData;
	inFile.close();

	printf("finished importing the data \n");

	bool matching = true;

	if (exportData.versionTracker == importData.versionTracker) {
		printf("same version confirmed : %s \n", importData.versionTracker.c_str());
	}
	else {
		matching = false;
		printf("differnt version! fatal error \n");
		printf("exported version : %s \n", exportData.versionTracker.c_str());
		printf("imported version : %s \n", importData.versionTracker.c_str());
	}
	if (matching) {
		
		for (int i = 0; i < exportData.meshExport.meshes.size(); i++) {
			for (int j = 0; j < exportData.meshExport.meshes[i].vertices.size(); j++) {
				matching = exportData.meshExport.meshes[i].vertices[j] == importData.meshes[i].vertices[j];
				if (!matching) {
					printf("not matching in vertices, i:j %d:%d \n", i, j);
					break;
				}
			}
			if (!matching) { break; }
			for (int j = 0; j < exportData.meshExport.meshes[i].indices.size(); j++) {
				matching = exportData.meshExport.meshes[i].indices[j] == importData.meshes[i].indices[j];
				if (!matching) {
					printf("indices not matching! i:j %d:%d \n", i, j);
					break;
				}
			}
			if (!matching) { break; }
		}
	}
	if (matching) {
		for (int i = 0; i < exportData.animExport.animations.size(); i++) {
			for (int j = 0; j < exportData.animExport.animations[i].size(); j++) {
				for (int k = 0; k < exportData.animExport.animations[i][j].size(); k++) {
					if (exportData.animExport.animations[i][j][k].boneID != importData.animations[i][j][k].boneID) {
						matching = false;
						printf("bone ids dont match, ijk ~ %d:%d:%d \n", i, j, k);
						i = 69420;
						j = 69420;
						k = 69420;
						break;
					}
					//if (!(exportData.animExport.animations[i][j][k].boneTransform == importData.animations[i][j][k].boneTransform)) {
					if(!memcmp(exportData.animExport.animations[i][j][k].boneTransform.bmat4, importData.animations[i][j][k].boneTransform.bmat4, sizeof(exportData.animExport.animations[i][j][k].boneTransform.bmat4))) {
						matching = false;
						printf("bone transform dont match, ijk %d:%d:%d \n", i, j, k);
						i = 69420;
						j = 69420;
						k = 69420;
						break;
					}
				}
			}
		}
	}
	if (!matching) {
		printf("failed to match import and output \n");
	}
	else {
		printf("meshes match! \n");
	}
}
*//*
void DataManager::setDefaultBones() {
	//size of defaltBoneValues, 
	exportData.defaultBoneValues.resize(exportData.animExport.animations.back()[0].size() * 16);

	for (int i = 0; i < exportData.defaultBoneValues.size(); i++) {
		exportData.defaultBoneValues[i] = exportData.animExport.animations.back()[0][i].boneTransform;
	}
	printf("finished default bones \n");
}
*/
void DataManager::setDefaultBones(std::vector<glm::mat4>& defaultFinal) {
	//printf("setting default bones?????? \n");
	exportData.animExport.defaultBoneValues.resize(defaultFinal.size());
	for (int i = 0; i < defaultFinal.size(); i++) {
		exportData.animExport.defaultBoneValues[i] = defaultFinal[i];
		//memcpy(&exportData.defaultBoneValues[i], glm::value_ptr(defaultFinal[i]), sizeof(float) * 16);
	}
}

void DataManager::printGLM4(std::vector<glm::mat4>& printed, std::string titleOf) {
	std::string printString = titleOf;
	printString += " index : % d \n";
	//std::string printString = "printing matrix values? bone id : %d \n";
	std::string printLine = " % .1f, % .1f, % .1f, % .1f, \n";
	for (int i = 0; i < printed.size(); i++) {
		printf(printString.c_str(), i);
		printf(printLine.c_str(), printed[0][0], printed[1][0], printed[2][0], printed[3][0]);
		printf(printLine.c_str(), printed[0][1], printed[1][1], printed[2][1], printed[3][1]);
		printf(printLine.c_str(), printed[0][2], printed[1][2], printed[2][2], printed[3][2]);
		printf(printLine.c_str(), printed[0][3], printed[1][3], printed[2][3], printed[3][3]);
		printf("\n");
	}
}

void DataManager::printGLM4(glm::mat4 printed, std::string titleOf) {
	std::string printString = titleOf;
	std::string printLine = " % .1f, % .1f, % .1f, % .1f, \n";
	printf(printString.c_str());
	printf(printLine.c_str(), printed[0][0], printed[1][0], printed[2][0], printed[3][0]);
	printf(printLine.c_str(), printed[0][1], printed[1][1], printed[2][1], printed[3][1]);
	printf(printLine.c_str(), printed[0][2], printed[1][2], printed[2][2], printed[3][2]);
	printf(printLine.c_str(), printed[0][3], printed[1][3], printed[2][3], printed[3][3]);
	printf("\n");
	
}

void DataManager::printAnimationBoneSet(std::vector<ExportData::boneEData>& printed, std::string titleOf) {
	std::string printString = titleOf;

	for (int i = 0; i < printed.size(); i++) {
		printf(printString.c_str(), i);
		//printed[i].boneTransform.print();
		printf("\n");
	}
}
/*
void DataManager::printBobMat4(std::vector<float> printed, std::string titleOf) {
	std::string printString = titleOf;

	for (int i = 0; i < printed.size(); i++) {
		printf(printString.c_str(), i);
		//printed[i].print();
		printf("\n");
	}
}
*/