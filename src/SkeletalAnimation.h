#pragma once

#include "Bone.h"
#include "Vertices.h"

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>

//#include <boost/serialization/nvp.hpp>

#include <map>
#include <vector>
#include <iostream>
#include <typeinfo>
#include <unordered_map>

#define FORCE_TANGENTS true

//#include <iomanip>

struct BoneInfo {
	/*id is index in finalBoneMatrices*/
	int id;
	/*offset matrix transforms vertex from model space to bone space*/
	glm::mat4 offset;
	BoneInfo() {}
	BoneInfo(int id) : id{ id } {}

};
struct AssimpNodeData {
	glm::mat4 transformation;
	std::string name;
	int childrenCount;
	std::vector<AssimpNodeData> children;
};

class SkeletalAnimation
{
public:
	SkeletalAnimation() = default;

	SkeletalAnimation(const aiScene* scene, int animationIter, std::unordered_map<std::string, BoneInfo>& boneInfoMap, int& boneCount);

	~SkeletalAnimation() {}

	Bone* FindBone(const std::string& name);

	inline float GetTicksPerSecond() { return m_TicksPerSecond; }
	inline float GetDuration() { return m_Duration; }
	inline const AssimpNodeData& GetRootNode() { return m_RootNode; }
	inline const std::unordered_map<std::string, BoneInfo>& GetBoneIDMap() { return m_BoneInfoMap; }
	inline uint32_t getBoneIDMapSize() { return m_BoneInfoMap.size(); }
	inline const int& getBoneSize() { return m_Bones.size(); }



private:
	void ReadMissingBones(const aiAnimation* animation, std::unordered_map<std::string, BoneInfo>& boneInfoMap, int& boneCount);

	void ReadHeirarchyData(AssimpNodeData& dest, const aiNode* src);

	float m_Duration;
	int m_TicksPerSecond;
	std::vector<Bone> m_Bones;
	AssimpNodeData m_RootNode;
	std::unordered_map<std::string, BoneInfo> m_BoneInfoMap;
};

class Animator {
public:
	Animator(SkeletalAnimation* animation);
	Animator();

	void UpdateAnimation(float dt);
	void updateAnimation(float dt, int iterator) {
		m_DeltaTime = dt;
		//m_CurrentTime->GetTicksPerSecond()* dt;
	}

	void PlayAnimation(SkeletalAnimation* pAnimation) {
		m_CurrentAnimation = pAnimation;
		m_CurrentTime = 0.0f;
	}

	bool SetAnimation(int animStep);
	void init(SkeletalAnimation* idleAnimation);
	void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 const& parentTransform);

	std::vector<glm::mat4>& GetFinalBoneMatrices() { return m_FinalBoneMatrices; }
	int32_t getHandBone() { return handBone; }

private:
	int32_t handBone = -1;
	std::vector<glm::mat4> m_FinalBoneMatrices;
	SkeletalAnimation* m_CurrentAnimation;
	float m_CurrentTime;
	float m_DeltaTime;
};

class SkeletonHandler {

public:
	SkeletonHandler(std::string filePath);
	void switchAnimation(int animationIter);
	void UpdateCurrentAnimation(float dt);

	auto& GetBoneInfoMap() { return m_BoneInfoMap; }
	int& GetBoneCount() { return m_BoneCounter; }

	std::vector<MeshData<boneVertex>> meshes{};
	std::vector<MeshData<boneVertexNoTangent>> meshesNT{};
	std::vector<MeshData<Vertex>> meshesSimple{};
	std::vector<MeshData<VertexNT>> meshesNTSimple{};

	//std::vector <std::pair<std::vector<boneVertex>, std::vector<uint32_t>>> meshes;

	std::vector<glm::mat4>& GetFinalBoneMatrices() { 
		if (animator.getHandBone() != -1) {
			handBone = animator.getHandBone();
		}
		return animator.GetFinalBoneMatrices(); 
	}
	int32_t getHandBone() { return handBone; }

	uint32_t getCurrentAnimationIter() { return currentAnimationIterator; }
	SkeletalAnimation& currentAnimation() { return animations[currentAnimationIterator]; }
	void setCurrentAnimation(int iter) { animator.SetAnimation(iter); }
	std::string getAnimName(int iter) { 
		return animationNames[iter];
	}

	std::vector<bool> usefulBone;
	uint32_t animationCount = 0;
	std::vector<std::string> animationNames;

	std::vector<std::string> meshNTNames;
	std::vector<std::string> meshNames;
	std::vector<std::string> meshSimpleNames;
	std::vector<std::string> meshNTSimpleNames;

private:
	std::vector<uint32_t> baseAnimIndices;
	std::vector<bool> duplicateAnims;
	std::string skelePath;

	aiString* aiBuffer;
	int32_t handBone = -1;
	std::vector<SkeletalAnimation> animations;
	uint32_t currentAnimationIterator = 0;
	const aiScene* scene;
	
	Animator animator;
	

	std::unordered_map<std::string, BoneInfo> m_BoneInfoMap; //
	int m_BoneCounter = 0;

	void processNode(aiNode* node, const aiScene* scene);
	//void processTextures(); //need to do this

	void collectTangent(boneVertex& vertex, aiMesh* mesh, uint32_t index) {
		if (mesh->HasTangentsAndBitangents()) {
			memcpy(&vertex.tangent, &mesh->mTangents[index], sizeof(float) * 3);
			//vertex.tangent = mesh->mTangents[index];
			//vertex.tangent.x = mesh->mTangents[index].x;
			//vertex.tangent.y = mesh->mTangents[index].y;
			//vertex.tangent.z = mesh->mTangents[index].z;
		}
	}
	void collectTangent(Vertex& vertex, aiMesh* mesh, uint32_t index) {
		if (mesh->HasTangentsAndBitangents()) {
			memcpy(&vertex.tangent, &mesh->mTangents[index], sizeof(float) * 3);
			vertex.tangent.x = mesh->mTangents[index].x;
			vertex.tangent.y = mesh->mTangents[index].y;
			vertex.tangent.z = mesh->mTangents[index].z;
		}
	}

	void collectTangent(boneVertexNoTangent& vertex, aiMesh* mesh, uint32_t index) {
		return;
	}
	template <typename T>
	void collectTangent(T vertex, aiMesh* mesh, uint32_t index) {
		return;
	}

	template <typename T>
	std::pair<std::vector<T>, std::vector<uint32_t>> processMesh(aiMesh* mesh, const aiScene* scene, std::string meshName);
	//std::pair<std::vector<boneVertexNoTangent>, std::vector<uint32_t>> processMeshNT(aiMesh* mesh, const aiScene* scene, std::string meshName);

	template<typename T>
	void ExtractBoneWeightForVertices(std::vector<T>& vertices, aiMesh* mesh, const aiScene* scene, bool flag);
	template<typename T>
	void SetVertexBoneData(T& vertex, int boneID, float weight);
	template <typename T>
	void SetVertexBoneDataToDefault(T& vertex);

	void SetVertexBoneDataToDefault(Vertex& vertex) { }
	void SetVertexBoneDataToDefault(VertexNT& vertex) {}

	void ExtractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene);
	void ExtractBoneWeightForVertices(std::vector<VertexNT>& vertices, aiMesh* mesh, const aiScene* scene);
	void ExtractBoneWeightForVertices(std::vector<boneVertex>& vertices, aiMesh* mesh, const aiScene* scene);
	void ExtractBoneWeightForVertices(std::vector<boneVertexNoTangent>& vertices, aiMesh* mesh, const aiScene* scene);

	void katanaBoneDefault(boneVertex& vertex);
	void shieldBoneDefault(boneVertex& vertex);

	template <typename T>
	void katanaBoneDefault(T& vertex);

	template <typename T>
	void shieldBoneDefault(T& vertex);

};

