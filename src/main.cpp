
#define ASSIMP_FBX true //can not have assimp_fbx true and gltf_glb true
#define GLTF_GLB false
#if GLTF_GLB
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "gltfSkeleton.h"
#endif

#if ASSIMP_FBX
#include "DataManager.h"
#endif


#include <cstdlib>
#include <thread>
#include <utility>
#include <fstream>
#include <filesystem>



static bool threadFinished = false;
/*
void timerFunction(std::string trackingString) {
	uint32_t tracker = 0;
	while (!threadFinished) {
		using namespace std::literals::chrono_literals;
		
		if (tracker % 10 == 0) {
			std::string trackerString = trackingString;
			trackerString += " %d seconds \n";
			printf(trackerString.c_str(), tracker / 10);
		}
		tracker++;
		std::this_thread::sleep_for(100ms);
	}
	if (threadFinished) {
		float trackerFloat = tracker;
		trackerFloat /= 10;
		std::string trackerString = trackingString;
		trackerString += " %.1f seconds \n";
		printf(trackerString.c_str(), trackerFloat);
	}
}
*/

//std::vector<uint32_t> relevantAnimations{ 0, 3, 6, 8, 9, 10 };

int main() {
#if FORCE_TANGENTS
	printf("FORCING TANGENT \n");
#endif


#if GLTF_GLB
	GLTFSkeleton skeleton("model/ELLENREDO.glb");

	return 0;
#endif
#if ASSIMP_FBX
	std::string threadString = "init skeleton";
	auto startTime = std::chrono::high_resolution_clock::now();
	std::string animationPath = "../inModels/treeThree.fbx";
	std::string exportLocation = "../output/treeThree";
	//printf("loading model : %s \n", animationPath.c_str());
	printf("current path : %s \n", std::filesystem::current_path().string().c_str());


	/*leaf importing*/
	/*
	DataManager dataManager("model/leaf.fbx", "leaf");
	dataManager.fileWrite();
	return 0;

	//DataManager::batchImporter();
	return 0;
	*/
	DataManager dataManager(animationPath, exportLocation);
	dataManager.fullProcess();
	printf("immediately after full process \n");
	threadFinished = true;
	auto timeDiff = std::chrono::duration<float, std::chrono::seconds::period>(std::chrono::high_resolution_clock::now() - startTime).count();
	//return 1;
	printf("total run time : %f.2", timeDiff);
	std::ofstream file;
	file.open("runTime.log", std::ios::app); //app for append
	printf("file was opened \n");
	if (file.is_open()) {
		//std::string trackingString = "1.0.0.2 removes the import and compare. time should go down as a result \n";
		//file << trackingString << '\n';
		// DO NOT HARD CODE NOTES< JUST ADD THEM TO THE FILE

		std::string outString = "1.0.1.1 \n";
		outString += "Total run time : ";
		outString += std::to_string(timeDiff) + '\n';
		//std::filesystem::path fileSystemPath = exportLocation;
		//outString += "file size : " + std::to_string(std::filesystem::file_size(fileSystemPath)) + '\n';

		file << outString;
		file.close();
		printf("log file written \n");
	}
	else {
		printf("failed ot open log file \n");
	}
	
	return 1;
#endif
}





