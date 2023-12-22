
/* not fully implemented
#pragma once



// #define TINYGLTF_NOEXCEPTION // optional. disable exception handling.
#include "external/tiny_gltf.h"

#include <filesystem>




//2 ways to proceed
//get glb supported in EW engine, and THEN do animations
//or
//do animations, THEN get glb supported



class GLTFSkeleton {
	public:
		void readNode(tinygltf::Node& node) {
			printf("node name, size - %s :%d \n", node.name.c_str(), node.children.size());
			/*
			for (int i = 0; i < node.children.size(); i++) {
				readNode(node.children[i]);
			}
			*
		}

	GLTFSkeleton(std::string filePath) {
		tinygltf::Model model;
		tinygltf::TinyGLTF loader;
		std::string err;
		std::string warn;

		if (!std::filesystem::exists(filePath)) {
			printf("file does not exist : %s \n", filePath.c_str());
		}
		printf("loding path : %s \n", filePath.c_str());
		bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, filePath);

		if (!warn.empty()) {
			printf("Warn: %s\n", warn.c_str());
		}

		if (!err.empty()) {
			printf("Err: %s\n", err.c_str());
		}

		if (!ret) {
			printf("Failed to parse glTF\n");
			//throw C++ exception
		}

		printf("gltf scene size : %d \n", model.scenes.size());
		printf("gltf animation size : %d \n", model.animations.size());
		printf("accessor size : %d \n \n", model.accessors.size());
		//printf("asset size : %d \n", model.asset.size());
		printf("\n material size : %d \n", model.materials.size());
		printf("\t texture size : %d \n", model.textures.size());
		printf("\t sampler size : %d \n", model.samplers.size());
		printf("\t image size : %d \n \n", model.images.size());

		printf("gltf node size : %d \n", model.nodes.size());
		for (int i = 0; i < model.nodes.size(); i++) {
			printf("node name, child count - %s : %d \n", model.nodes[i].name.c_str(), model.nodes[i].children.size());
		}


		printf("gltf skin size : %d \n", model.skins.size());
		printf("gltf mesh size : %d \n", model.meshes.size());

		for (int i = 0; i < model.meshes.size(); i++) {
			printf("gltf mesh %d name : %s \n", i, model.meshes[i].name.c_str());
			printf("\t mesh primitive size : %d \n", model.meshes[i].primitives.size());
			for (int j = 0; j < model.meshes[i].primitives.size(); j++) {
				for (auto iter = model.meshes[i].primitives[j].attributes.begin(); iter != model.meshes[i].primitives[j].attributes.end(); iter++) {
					printf("\t \t primiitives first:second - %s:%d \n", iter->first.c_str(), iter->second);
				}
				printf("\n");
			}
			printf("\t mesh weights size : %d \n", model.meshes[i].weights.size());
			for (int j = 0; j < model.meshes[i].weights.size(); j++) {
				printf("mesh weight %d : %f \n", j, model.meshes[i].weights[j]);
			}
			printf("\t mesh targets size : %d \n", model.meshes[i].extensions.size());

		}
		printf("\n \n");
	}


};

*/