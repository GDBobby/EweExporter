#include "SkeletalAnimation.h"

#include <exception>
#include <stdexcept>



#define CUSTOM_ROLL_INDICES true

template <typename T, typename... Rest>
void hashCombine(std::size_t& seed, const T& v, const Rest&... rest) {
	seed ^= std::hash<T>{}(v)+0x9e3779b9 + (seed << 6) + (seed >> 2);
	(hashCombine(seed, rest), ...);
};

template<>
struct std::hash<glm::vec2> {
	size_t operator()(glm::vec2 const& bv2) const {
		size_t seed = 0;
		hashCombine(seed, bv2.x, bv2.y);
		return seed;
	}
};

template<>
struct std::hash<glm::vec3> {
	size_t operator()(glm::vec3 const& bv3) const {
		size_t seed = 0;
		hashCombine(seed, bv3.x, bv3.y, bv3.z);
		return seed;
	}
};

template <>
struct std::hash<boneVertex> {
	size_t operator()(boneVertex const& vertex) const {
		size_t seed = 0;
		hashCombine(seed, vertex.position, vertex.normal, vertex.uv, vertex.tangent,
			vertex.m_BoneIDs[0], vertex.m_BoneIDs[1], vertex.m_BoneIDs[2], vertex.m_BoneIDs[3],
			vertex.m_Weights[0], vertex.m_Weights[1], vertex.m_Weights[2], vertex.m_Weights[3]
		);
		return seed;
	}
};


template <>
struct std::hash<boneVertexNoTangent> {
	size_t operator()(boneVertexNoTangent const& vertex) const {
		size_t seed = 0;
		hashCombine(seed, vertex.position, vertex.normal, vertex.uv,
			vertex.m_BoneIDs[0], vertex.m_BoneIDs[1], vertex.m_BoneIDs[2], vertex.m_BoneIDs[3],
			vertex.m_Weights[0], vertex.m_Weights[1], vertex.m_Weights[2], vertex.m_Weights[3]
		);
		return seed;
	}
};
template <>
struct std::hash<Vertex> {
	size_t operator()(Vertex const& vertex) const {
		size_t seed = 0;
		hashCombine(seed, vertex.position, vertex.normal, vertex.uv, vertex.tangent);
		return seed;
	}
};
template <>
struct std::hash<VertexNT> {
	size_t operator()(VertexNT const& vertex) const {
		size_t seed = 0;
		hashCombine(seed, vertex.position, vertex.normal, vertex.uv);
		return seed;
	}
};







#define APS_60 false // force the animation to be at 60 fps

#define USING_TANGENT_SPACE true

SkeletalAnimation::SkeletalAnimation(const aiScene* scene, int animationIter, std::map<std::string, BoneInfo>& boneInfoMap, int& boneCount) {
	if (animationIter > scene->mNumAnimations) {
		std::cout << "?? animation iter too high" << std::endl;
	}
#if APS_60
	m_Duration = scene->mAnimations[animationIter]->mDuration * 60.0 / 250.0;
#else
	m_Duration = scene->mAnimations[animationIter]->mDuration;
#endif
	m_TicksPerSecond = scene->mAnimations[animationIter]->mTicksPerSecond;

	ReadHeirarchyData(m_RootNode, scene->mRootNode);
	ReadMissingBones(scene->mAnimations[animationIter], boneInfoMap, boneCount);
}
Bone* SkeletalAnimation::FindBone(const std::string& name) {
	auto iter = std::find_if(m_Bones.begin(), m_Bones.end(),
		[&](const Bone& Bone) {
			return Bone.GetBoneName() == name;
		}
	);
	if (iter == m_Bones.end()) return nullptr;
	else return &(*iter);
}

void SkeletalAnimation::ReadMissingBones(const aiAnimation* animation, std::map<std::string, BoneInfo>& boneInfoMap, int& boneCount) {
	int size = animation->mNumChannels;
	//std::cout << "mNumChannels : " << animation->mNumChannels << std::endl;

	//auto& boneInfoMap = skeleton.GetBoneInfoMap();//getting m_BoneInfoMap from Model class
	//int& boneCount = skeleton.GetBoneCount(); //getting the m_BoneCounter from Model class

	//reading channels(bones engaged in an animation and their keyframes)
	for (int i = 0; i < size; i++) {
		auto channel = animation->mChannels[i];
		std::string boneName = channel->mNodeName.data;

		if (boneInfoMap.find(boneName) == boneInfoMap.end())
		{
			boneInfoMap[boneName].id = boneCount;
			boneCount++;
		}
		m_Bones.push_back(Bone(channel->mNodeName.data,
			boneInfoMap[channel->mNodeName.data].id, channel));
	}

	m_BoneInfoMap = boneInfoMap;
}
void SkeletalAnimation::ReadHeirarchyData(AssimpNodeData& dest, const aiNode* src) {
	assert(src);

	dest.name = src->mName.data;
	//dest.transformation = AssimpGLMHelpers::ConvertMatrixToGLMFormat(src->mTransformation);
	dest.transformation[0][0] = src->mTransformation.a1; dest.transformation[1][0] = src->mTransformation.a2; dest.transformation[2][0] = src->mTransformation.a3; dest.transformation[3][0] = src->mTransformation.a4;
	dest.transformation[0][1] = src->mTransformation.b1; dest.transformation[1][1] = src->mTransformation.b2; dest.transformation[2][1] = src->mTransformation.b3; dest.transformation[3][1] = src->mTransformation.b4;
	dest.transformation[0][2] = src->mTransformation.c1; dest.transformation[1][2] = src->mTransformation.c2; dest.transformation[2][2] = src->mTransformation.c3; dest.transformation[3][2] = src->mTransformation.c4;
	dest.transformation[0][3] = src->mTransformation.d1; dest.transformation[1][3] = src->mTransformation.d2; dest.transformation[2][3] = src->mTransformation.d3; dest.transformation[3][3] = src->mTransformation.d4;
	dest.childrenCount = src->mNumChildren;

	for (int i = 0; i < src->mNumChildren; i++)
	{
		AssimpNodeData newData;
		ReadHeirarchyData(newData, src->mChildren[i]);
		dest.children.push_back(newData);
	}
}

#define EXTRA_MATERIALS_PATH "C:\\Projects\\BOUGHTMODELS\\peoplePack\\uploads_files_4410196_Blender_Ellen\\Blender_Ellen\\textures\\mesh\\"


/*animator*/
Animator::Animator(SkeletalAnimation* animation) {
	m_CurrentTime = 0.0;
	m_CurrentAnimation = animation;
	//m_FinalBoneMatrices.resize(100, glm::mat4(1.0f)); ?? why not
	uint32_t boneSize = animation->getBoneSize();
	std::cout << "bone id map size : " << boneSize << std::endl;

	m_FinalBoneMatrices.reserve(boneSize);
	for (int i = 0; i < boneSize; i++) { m_FinalBoneMatrices.push_back(glm::mat4(1.0f)); }
}
Animator::Animator() {
	m_CurrentTime = 0.0;
	m_CurrentAnimation = nullptr;
}
void Animator::init(SkeletalAnimation* idleAnimation) {
	m_CurrentTime = 0.0;
	m_CurrentAnimation = idleAnimation;

	uint32_t boneSize = idleAnimation->getBoneSize();
	m_FinalBoneMatrices.reserve(boneSize);
	for (int i = 0; i < boneSize; i++) { m_FinalBoneMatrices.push_back(glm::mat4(1.0f)); }
}
void Animator::UpdateAnimation(float dt) {
	m_DeltaTime = dt;
	//std::cout << "inside update animation " << std::endl;
	if (m_CurrentAnimation) {
		m_CurrentTime += m_CurrentAnimation->GetTicksPerSecond() * dt;
		m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->GetDuration());
		//std::cout << "pre updateanimation calc bone transform" << std::endl;
		CalculateBoneTransform(&m_CurrentAnimation->GetRootNode());
		//std::cout << "post updateanimation calc bone transform" << std::endl;
	}
}
bool Animator::SetAnimation(int animStep) {

	m_CurrentTime = m_CurrentAnimation->GetDuration() * animStep / (m_CurrentAnimation->GetDuration() + 1);
	if (m_CurrentTime > m_CurrentAnimation->GetDuration()) {
		return false;
	}
	CalculateBoneTransform(&m_CurrentAnimation->GetRootNode());
}

void Animator::CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform/* = glm::mat4(1.0f)*/) {
	//std::cout << "inside calculate bone transform" << std::endl;
	std::string nodeName = node->name;
	//printf("calculating bone transform, node name : %s \n", node->name.c_str());
	glm::mat4 nodeTransform = node->transformation;

	Bone* Bone = m_CurrentAnimation->FindBone(nodeName);

	if (Bone) {
		Bone->Update(m_CurrentTime);
		nodeTransform = Bone->GetLocalTransform();
	}

	glm::mat4 globalTransformation = parentTransform * nodeTransform;

	auto boneInfoMap = m_CurrentAnimation->GetBoneIDMap();
	if (boneInfoMap.find(nodeName) != boneInfoMap.end()) {
		int index = boneInfoMap[nodeName].id;
		glm::mat4 offset = boneInfoMap[nodeName].offset;
		m_FinalBoneMatrices[index] = globalTransformation * offset;
	}

	//when doing a different bone structure, need to rename this bone, no current method to support dynamic armature changes
	if (node->name == "weapon_r") {
		handBone = boneInfoMap[nodeName].id;
	}
	else if (node->name == "hand_R") {
		handBone = boneInfoMap[nodeName].id;
	}
	else if (node->name == "Wrist.R") {
		handBone = boneInfoMap[nodeName].id;
		//std::cout << "handbone id : " << handBone << std::endl;
	}

	for (int i = 0; i < node->childrenCount; i++) { CalculateBoneTransform(&node->children[i], globalTransformation); }

	//std::cout << "end of calculate bone transform" << std::endl;
}

SkeletonHandler::SkeletonHandler(std::string filePath) {
	if (filePath.find("ellen") != filePath.npos) {
		printf("found ellen \n");
		usefulBone.resize(101, false); /* THIS HAS TO BE SET MANUALLY BECAUSE BONE COUNT ISN'T PROCESSED UNTIL ITS TOO lATE*/
	}

	if (filePath.find("skeleGuy") != filePath.npos) {
		printf("found skeleGuy \n");
		usefulBone.resize(79, false);
	}
	else if (filePath.find("BaseHuman") != filePath.npos) {
		printf("found basehuman \n");
		usefulBone.resize(62, false);
	}
	else if (filePath.find("ellMesh") != filePath.npos) {
		printf("found ellMesh \n");
		usefulBone.resize(101, false);
	}
	else if (filePath.find("ellBASE") != filePath.npos) {
		printf("found ellBASE \n");
		usefulBone.resize(101, false);
	}
	else if (filePath.find("katana") != filePath.npos) {
		usefulBone.resize(1, false);
	}
	else if (filePath.find("spear") != filePath.npos) {
		usefulBone.resize(1, false);
	}
	else if (filePath.find("shield") != filePath.npos) {
		usefulBone.resize(1, false);
	}
	else if (filePath.find("Corruption") != filePath.npos) {
		printf("found Corruption \n");
		usefulBone.resize(62, false);
	}
	else if (filePath.find("PlayerMan") != filePath.npos) {
		printf("found PlayerMan \n");
		usefulBone.resize(62, false);
	}
	else if (filePath.find("skeleMon") != filePath.npos) {
		printf("found skelemon \n");
		usefulBone.resize(68, false);
	}
	else if (filePath.find("LichKing") != filePath.npos) {
		printf("found lich king \n");
		usefulBone.resize(11, false);
	}
	else if (filePath.find("deerMonster") != filePath.npos) {
		printf("found deerman \n");
		usefulBone.resize(21, false);
	}
	else if (filePath.find("DevilMan") != filePath.npos) {
		printf("found devilman \n");
		usefulBone.resize(52, false);
	}
	else if (filePath.find("charmer") != filePath.npos) {
		usefulBone.resize(35, false);
	}
	else if (filePath.find("carrot") != filePath.npos) {
		usefulBone.resize(33, false);
	}
	else {
		usefulBone.resize(66, false);
		printf("default bone count %d : %s \n", usefulBone.size(), filePath.c_str());
	}
	skelePath = filePath;
	//aiScene* scene; need const to improt readfile
	printf("reading file in skele handler : %s \n", filePath.c_str());
	Assimp::Importer importer;

	//printf("After debugger, loading fiile \n");

	scene = importer.ReadFile(filePath, aiProcess_FindInvalidData | aiProcess_ValidateDataStructure | aiProcess_FindDegenerates);
	printf("generating smooth normals \n");
	//importer.ApplyPostProcessing(aiProcess_GenSmoothNormals);
	printf("calculating tanggent space - a waste on the models that arent oign to use it \n");
	//importer.ApplyPostProcessing(aiProcess_CalcTangentSpace);
	importer.ApplyPostProcessing(aiProcess_OptimizeGraph | aiProcess_OptimizeMeshes);

	if (!scene) {
		printf("ASSIMP::ERROR - file:%s - error:%s", filePath.c_str(), importer.GetErrorString());
		return;
	}
	if (!scene->mRootNode) {
		printf("error : %s - does not have root node \n", filePath.c_str());
	}
	if (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
		printf("ai scene flag incomplete : %s \n", filePath.c_str());
	}
	//}
	/*
	catch (const std::runtime_error& error) {
		//printf("wtf \n");
		printf("wtf \n");
		printf("%s \n", error.what());
		std::cerr << "Error importing file: " << error.what() << '\n';
		printf("improt error string : %s \n", importer.GetErrorString());
		//printf("import error :%s \n", import.GetException()->what());
	}
	*/
	printf("immediately after scene \n");
	if (scene == NULL) {
		
		printf("error logged \n");
	}
	printf("number of texures : %d \n", scene->mNumTextures);
	for (int i = 0; i < scene->mNumTextures; i++) {
		char buffer[5];
		
		if (scene->mTextures[i]->CheckFormat(buffer)) {
			buffer[3] = '\0';
			printf("file name, type - %s:%s \n", scene->mTextures[i]->mFilename.C_Str(), buffer);
		}
		else {
			printf("format type not found, name:%s \n", scene->mTextures[i]->mFilename.C_Str());
		}
		//std::string bufferString = buffer;
		//textureSaving();
	}
	printf("numebr of meshes : %d \n", scene->mNumMeshes);
	for (int i = 0; i < scene->mNumMeshes; i++) {
		printf("mesh name[%d] : %s \n", i, scene->mMeshes[i]->mName.C_Str());
	}
	for (int i = 0; i < scene->mNumMaterials; i++) {
		printf("material name[%d] : %s \n", i, scene->mMaterials[i]->GetName().C_Str());
	}
	printf("number of materials : %d \n", scene->mNumMaterials);
	animationCount = scene->mNumAnimations;
	processNode(scene->mRootNode, scene);

	//reimplement skeletalAnimation
	
	if (animationCount == 0) {
		printf("returning on 0 animations in skeletonHandler constructor \n");
		if (!(meshes.size() == meshNames.size()) && (meshesNT.size() == meshNTNames.size())) {
			printf("size mismatch! assert error \n");
		}
		assert((meshes.size() == meshNames.size()) && (meshesNT.size() == meshNTNames.size()));
		return;
	}
	printf("init animationCount = %d \n", animationCount);
	//animationNames.resize(animationCount);
	animations.reserve(animationCount);
	for (int i = 0; i < animationCount; i++) {
		printf("animaiton pre-duplicate triimming : %s \n", scene->mAnimations[i]->mName.C_Str());
	}
	int idleAnimIter = -1;
	for (int i = 0; i < animationCount; i++) {
		//i dont know why, but scene is losing all its values after this construction function, so store this value for later
		animationNames.push_back(scene->mAnimations[i]->mName.C_Str());
		if (animationNames.back() == "idle.002") { animationNames.pop_back(); continue; }
		animationNames.back() = animationNames.back().substr(0, animationNames.back().find_first_of("."));
		if(animationNames.back() == "idle") { idleAnimIter = i; }

		//if (animationNames.back() == "Walking") { continue; }
		//if (animationNames.back() == "landing") { continue; }
		//if (animationNames.back() == "idle") { continue; }
		//if (animationNames.back() == "Jumping") { continue; }
		bool foundAMatch = false;
		/*
		for (int j = 0; j < animationNames.size() - 1; j++) {
			if (animationNames.back().find(animationNames[j]) != animationNames.back().npos) {
				printf("comparison - %s : %s \n", animationNames[j].c_str(), animationNames.back().c_str());
				foundAMatch = true;
			}
			//foundAMatch |= animationNames.back().find(animationNames[j]) != animationNames.back().npos;
		}
		if (foundAMatch) {
			printf("found a match, popping back : %s \n", animationNames.back().c_str());
			animationNames.pop_back();
		}
		else {
		*/
			animations.emplace_back(scene, i, m_BoneInfoMap, m_BoneCounter);
		//}
	}
	printf("after animation names \n");
	
	//POSSIBLE ISSUE WITH THIS METHOD, if similar names then it'll delete despite not being duplicate
		//if this becomes serious, then do a data comparison before deleting
		//or jsut dont name unique animations the same thing in blender

	//uint16_t uniqueAnimCounter = 0;
	
	for (int i = 0; i < animationNames.size(); i++) {
		animations.emplace_back(scene, i, m_BoneInfoMap, m_BoneCounter);
	}
	printf("after animation emplacement \n");

	animationCount = animationNames.size();
	printf("after emplacing anims, animation counts - %d:%d \n", animationCount, animationNames.size());
	currentAnimationIterator = 0;
	if (idleAnimIter >= 0) {
		animator.init(&animations[idleAnimIter]);
	}
	else {
		animator.init(&animations[0]);
	}
}
void SkeletonHandler::switchAnimation(int animationIter) {
	if ((animationIter >= animationNames.size())) {
		std::cout << "switch animation iter too high, doing a modulus temporarily" << std::endl;
		if (animationCount == 0) { printf("0animation count return \n"); return; }

		animationIter %= animationNames.size();
	}
	currentAnimationIterator = animationIter;
	//get the old index
	animator.PlayAnimation(&animations[animationIter]);
	//printf("animation iter from switch %d \n", animationIter);
}
void SkeletonHandler::UpdateCurrentAnimation(float dt) {
	animator.UpdateAnimation(dt);
}
void SkeletonHandler::processNode(aiNode* node, const aiScene* scene) {
	// process all the node's meshes (if any)
	//printf("processing node : %s \n", node->mName.C_Str());
	for (unsigned int i = 0; i < node->mNumMeshes; i++) {
		printf("processing a mesh : %s \n", scene->mMeshes[node->mMeshes[i]]->mName.C_Str());
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
#if USING_TANGENT_SPACE
		if ((skelePath.find("katana") != skelePath.npos) || (skelePath.find("spear") != skelePath.npos) || (skelePath.find("shield") != skelePath.npos)) {
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			//std::string tempNameBuffer = material->GetName().C_Str();
			//tempNameBuffer = tempNameBuffer.substr(0, tempNameBuffer.find_first_of("."));
			//meshNames.push_back(tempNameBuffer);
			meshNames.push_back(material->GetName().C_Str());
			meshes.push_back(processMesh<boneVertex>(mesh, scene, meshNames.back()));
		}
		else if (scene->HasMaterials()) {
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			if (material != nullptr) {
				//printf("material name : %s \n", material->GetName().C_Str());
				if (animationCount != 0) {
					if (material->GetTextureCount(aiTextureType::aiTextureType_NORMALS) > 0) {
						meshNames.push_back(material->GetName().C_Str());
						meshes.push_back(processMesh<boneVertex>(mesh, scene, meshNames.back()));
					}
					else {
#if FORCE_TANGENTS

						std::string tempNameBuffer = material->GetName().C_Str();
						tempNameBuffer = tempNameBuffer.substr(0, tempNameBuffer.find_first_of("."));
						meshNames.push_back(tempNameBuffer);
						//meshNames.push_back(material->GetName().C_Str());
						meshes.push_back(processMesh<boneVertex>(mesh, scene, meshNames.back()));
#else
						meshNTNames.push_back(material->GetName().C_Str());
						meshesNT.push_back(processMesh<boneVertexNoTangent>(mesh, scene, meshNTNames.back()));
#endif
					}
				}
				else {
					if (material->GetTextureCount(aiTextureType::aiTextureType_NORMALS) > 0) {
						meshSimpleNames.push_back(material->GetName().C_Str());
						meshesSimple.push_back(processMesh<Vertex>(mesh, scene, meshSimpleNames.back()));
					}
					else {
#if FORCE_TANGENTS
						meshSimpleNames.push_back(material->GetName().C_Str());
						meshesSimple.push_back(processMesh<Vertex>(mesh, scene, meshSimpleNames.back()));
#else
						meshNTSimpleNames.push_back(material->GetName().C_Str());
						meshesNTSimple.push_back(processMesh<VertexNT>(mesh, scene, meshNTSimpleNames.back()));
#endif
					}
				}
			}
			else {
				printf("mesh does not have a material! ERROR \n");
				//std throw
				//meshNames.push_back(mesh->mName.C_Str());
				//meshes.push_back(processMesh(mesh, scene, meshNames.back()));
			}
		}
		else {
			printf("INVALID MESH \n");
			throw std::exception("invalid mesh");
		}
#else
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		if (material != nullptr) {
			meshNTNames.push_back(material->GetName().C_Str());
		}
		else {
			meshNTNames.push_back(mesh->mName.C_Str());
		}
		meshesNT.push_back(processMeshNT(mesh, scene, meshNTNames.back()));
#endif
	}
	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		processNode(node->mChildren[i], scene);
	}
}

template <typename T>
std::pair<std::vector<T>, std::vector<uint32_t>> SkeletonHandler::processMesh(aiMesh* mesh, const aiScene* scene, std::string meshName) {
	// data to fill

	std::vector<T> vertices;
	std::vector<uint32_t> indices;
	//std::vector<Texture> textures;

	if (scene->HasMaterials()) {
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		if (material != nullptr) {
			printf("material name : %s \n", material->GetName().C_Str());
			//uint8_t knownTextureCount = 0;
			for (int t = 0; t < 21; t++) {

				if (material->GetTextureCount((aiTextureType)t) > 0) {
					
					printf("\t material textures:%s count :%d \n", aiTextureTypeToString((aiTextureType)t), material->GetTextureCount((aiTextureType)t));
					//knownTextureCount += material->GetTextureCount((aiTextureType)t);
				}
			}
			
			//uint8_t unknownTextureCount = 0;
#if false
			for (int i = 0; i < material->mNumProperties; i++) {

				printf("\t property values, dataLength : mIndex : mSemantic : mType - %d : %d : %d : %d \n",
					material->mProperties[i]->mDataLength,
					material->mProperties[i]->mIndex,
					material->mProperties[i]->mSemantic,
					material->mProperties[i]->mType
				);

			}
#endif
			/*
				if (material->mProperties[i]->mType == aiPTI_String) {
					unknownTextureCount++;
					printf("\t property[%d] : %s \n", i, material->mProperties[i]->mKey.C_Str());
					
#define METHOD_AI false
#if METHOD_AI
					aiString outputString;
					aiGetMaterialString(material, material->mProperties[i]->mKey.C_Str(), material->mProperties[i]->mType, material->mProperties[i]->mIndex, &outputString);
					printf("\t\t pls : %s \n", outputString.C_Str());
#endif
#define METHOD_CHAR false
#if METHOD_CHAR
					char buffer[256];
					size_t bufferSize = std::min((unsigned int)255, material->mProperties[i]->mDataLength);
					memcpy(buffer, material->mProperties[i]->mData, bufferSize);

					char oldValue = buffer[bufferSize];
					buffer[bufferSize] = '\0';
					printf("\t data in pls, bufferSize 0 - %s : %d \n", buffer, bufferSize);

					buffer[bufferSize] = oldValue;
					buffer[bufferSize - 1] = '\0';
					printf("\t data in pls, bufferSize-1 - %s : %d \n", buffer, bufferSize);

					//just cause
					buffer[bufferSize - 1] = oldValue;
					buffer[bufferSize + 1] = '\0';
					printf("\t data in pls, bufferSize+1 - %s : %d \n", buffer, bufferSize);
#endif

				}
			}


			std::string unknownMaterialPath = EXTRA_MATERIALS_PATH;
			unknownMaterialPath += meshName + "\\" + meshName + "\\";
			unknownMaterialPath += material->GetName().C_Str();

			printf("individualTexture path : %s \n", )
			//printf("unknownMaterialPath : %s \n", unknownMaterialPath.c_str());
			uint8_t foundMaterialCount = 0;
			if (std::filesystem::exists(unknownMaterialPath)) {
				for (const auto& entry : std::filesystem::directory_iterator(unknownMaterialPath)) {
					//printf("texture index:name -  %d : %s \n", foundMaterialCount, entry.path().string().c_str());
					foundMaterialCount++;
				}
			}

			if (unknownTextureCount != (foundMaterialCount + knownTextureCount)) {
				printf("unknown material Count, found material Count, knownTextureCount - %d : %d \n", unknownTextureCount, foundMaterialCount,  knownTextureCount);
			}
			else {
				printf("all textures accoutned for \n");
			}
			*/
		}
		else {
			printf("mesh does not have a material! ERROR \n");
			//std throw
		}


		//aiTextureType_UNKNOWN
		//printf("\n");
	}
	// walk through each of the mesh's vertices
	for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
		T vertex;
		if ((skelePath.find("katana") != skelePath.npos) || (skelePath.find("spear") != skelePath.npos)) {
			katanaBoneDefault(vertex);
		}
		else if (skelePath.find("shield") != skelePath.npos) {
			shieldBoneDefault(vertex);
		}
		else {
			SetVertexBoneDataToDefault(vertex);
		}

		memcpy(&vertex.position, &mesh->mVertices[i], sizeof(float) * 3);
		//vertex.position = mesh->mVertices[i];
		//vertex.position.x = mesh->mVertices[i].x;
		//vertex.position.y = mesh->mVertices[i].y;
		//vertex.position.z = mesh->mVertices[i].z;
		//vertex.normal = AssimpGLMHelpers::GetGLMVec(mesh->mNormals[i]);

		if (mesh->HasNormals()) {
			memcpy(&vertex.normal, &mesh->mNormals[i], sizeof(float) * 3);
			//vertex.normal = mesh->mNormals[i];
			//vertex.normal.x = mesh->mNormals[i].x;
			//vertex.normal.y = mesh->mNormals[i].y;
			//vertex.normal.z = mesh->mNormals[i].z;
			
		}
		else {
			//std::cout << "why no normals" << std::endl;
		}


		// texture coordinates
		if (mesh->mTextureCoords[0]) { // does the mesh contain texture coordinates?
			// a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
			// use models where a vertex can have multiple texture coordinates so we always take the first set (0).
			/*
			if (mesh->mTextureCoords.size() > 1) {
				printf("size of texture coords? \n");
			}
			*/

			vertex.uv.x = mesh->mTextureCoords[0][i].x;
			vertex.uv.y =  1.f - mesh->mTextureCoords[0][i].y;
		}
		else {
			vertex.uv.x = 0.f;
			vertex.uv.y = 0.f;
		}
		collectTangent(vertex, mesh, i);
		vertices.push_back(vertex);
	}
	// now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
	if ((skelePath.find("katana") != skelePath.npos) || (skelePath.find("spear") != skelePath.npos) || (skelePath.find("shield") != skelePath.npos)) {
		ExtractBoneWeightForVertices(vertices, mesh, scene);
	}
	else {
		ExtractBoneWeightForVertices(vertices, mesh, scene);
	}

#if CUSTOM_ROLL_INDICES
	printf("mesh doesn't have indices? \n");
	printf("rolling custom indexer? :%d \n", vertices.size());

	std::unordered_map<T, uint32_t> uniqueVertices{};

	std::vector<T> indexedVertices;

	for (int i = 0; i < vertices.size(); i++) {
		if (uniqueVertices.count(vertices[i]) == 0) {
			uniqueVertices[vertices[i]] = static_cast<uint32_t>(indexedVertices.size());
			indexedVertices.push_back(vertices[i]);
		}
		indices.push_back(uniqueVertices[vertices[i]]);
	}
	printf("custom indexing, sizes, vertices, indexedVertices, indices - %d:%d:%d \n", vertices.size(), indexedVertices.size(), indices.size());
	return { indexedVertices, indices };
	
#else
	uint32_t lastIndexValue = 0;
	for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++) {
			indices.push_back(face.mIndices[j]);
			if (face.mIndices[j] != lastIndexValue + 1) {
				printf("index isn't consecutive - %d:%d \n", face.mIndices[j], lastIndexValue);
			}
			lastIndexValue = face.mIndices[j];
			//printf("face index value : %d \n", face.mIndices[j]);
		}
	}
	
#endif

	printf("size of vertices:indices - %d:%d \n", vertices.size(), indices.size());
	return { vertices, indices };
}

void SkeletonHandler::ExtractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene) {
	return;
}
void SkeletonHandler::ExtractBoneWeightForVertices(std::vector<VertexNT>& vertices, aiMesh* mesh, const aiScene* scene) {
	return;
}
void SkeletonHandler::ExtractBoneWeightForVertices(std::vector<boneVertex>& vertices, aiMesh* mesh, const aiScene* scene) {
	ExtractBoneWeightForVertices(vertices, mesh, scene, true);
}
void SkeletonHandler::ExtractBoneWeightForVertices(std::vector<boneVertexNoTangent>& vertices, aiMesh* mesh, const aiScene* scene) {
	ExtractBoneWeightForVertices(vertices, mesh, scene, true);
}


//the flag is just to differentiate functions so that ic an template this
//i could just as easily remove the flag and rename the function
template<typename T>
void SkeletonHandler::ExtractBoneWeightForVertices(std::vector<T>& vertices, aiMesh* mesh, const aiScene* scene, bool flag) { 
	printf("extracting bones, mesh num boens : %d \n", mesh->mNumBones);
	for (int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
		int boneID = -1;
		std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
		if (m_BoneInfoMap.find(boneName) == m_BoneInfoMap.end()) {
			BoneInfo newBoneInfo;
			newBoneInfo.id = m_BoneCounter;
			//newBoneInfo.offset = AssimpGLMHelpers::ConvertMatrixToGLMFormat(mesh->mBones[boneIndex]->mOffsetMatrix);
			newBoneInfo.offset[0][0] = mesh->mBones[boneIndex]->mOffsetMatrix.a1; newBoneInfo.offset[1][0] = mesh->mBones[boneIndex]->mOffsetMatrix.a2; newBoneInfo.offset[2][0] = mesh->mBones[boneIndex]->mOffsetMatrix.a3; newBoneInfo.offset[3][0] = mesh->mBones[boneIndex]->mOffsetMatrix.a4;
			newBoneInfo.offset[0][1] = mesh->mBones[boneIndex]->mOffsetMatrix.b1; newBoneInfo.offset[1][1] = mesh->mBones[boneIndex]->mOffsetMatrix.b2; newBoneInfo.offset[2][1] = mesh->mBones[boneIndex]->mOffsetMatrix.b3; newBoneInfo.offset[3][1] = mesh->mBones[boneIndex]->mOffsetMatrix.b4;
			newBoneInfo.offset[0][2] = mesh->mBones[boneIndex]->mOffsetMatrix.c1; newBoneInfo.offset[1][2] = mesh->mBones[boneIndex]->mOffsetMatrix.c2; newBoneInfo.offset[2][2] = mesh->mBones[boneIndex]->mOffsetMatrix.c3; newBoneInfo.offset[3][2] = mesh->mBones[boneIndex]->mOffsetMatrix.c4;
			newBoneInfo.offset[0][3] = mesh->mBones[boneIndex]->mOffsetMatrix.d1; newBoneInfo.offset[1][3] = mesh->mBones[boneIndex]->mOffsetMatrix.d2; newBoneInfo.offset[2][3] = mesh->mBones[boneIndex]->mOffsetMatrix.d3; newBoneInfo.offset[3][3] = mesh->mBones[boneIndex]->mOffsetMatrix.d4;

			m_BoneInfoMap[boneName] = newBoneInfo;
			boneID = m_BoneCounter;
			m_BoneCounter++;
		}
		else {
			boneID = m_BoneInfoMap[boneName].id;
		}
		assert(boneID != -1);
		auto weights = mesh->mBones[boneIndex]->mWeights;
		int numWeights = mesh->mBones[boneIndex]->mNumWeights;

		for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex) {
			int vertexId = weights[weightIndex].mVertexId;
			float weight = weights[weightIndex].mWeight;
			if (vertexId > vertices.size()) {
				printf("vertexID finna fail assert - %d:%d \n", vertexId, vertices.size());
			}
			assert(vertexId <= vertices.size());
			if (boneID < -1) {
				printf("bone id is less than -1, value:meshName - %d:%s \n", boneID, mesh->mName.C_Str());
			}
			SetVertexBoneData(vertices[vertexId], boneID, weight);
		}
	}
}
template <typename T>
void SkeletonHandler::SetVertexBoneData(T& vertex, int boneID, float weight) {
	for (int i = 0; i < MAX_BONE_WEIGHTS; ++i) {
		if (vertex.m_BoneIDs[i] < 0) {
			vertex.m_Weights[i] = weight;
			vertex.m_BoneIDs[i] = boneID;
			if (boneID > usefulBone.size()) {
				printf("bone id greater than usefulBone size, values - %d:%d \n", boneID, usefulBone.size());
			}
			usefulBone[boneID] = true;
			break;
		}
	}
}

template <typename T>
void SkeletonHandler::SetVertexBoneDataToDefault(T& vertex) {
	for (int i = 0; i < MAX_BONE_WEIGHTS; i++) {
		vertex.m_BoneIDs[i] = -1;
		vertex.m_Weights[i] = 0.0f;
	}
}

void SkeletonHandler::katanaBoneDefault(boneVertex& vertex) {
	vertex.m_BoneIDs[0] = 10; //50 for skeleton, 39 for homie
	vertex.m_Weights[0] = 1.0f;
	for (int i = 1; i < MAX_BONE_WEIGHTS; i++) {
		vertex.m_BoneIDs[i] = 0;
		vertex.m_Weights[i] = 0.0f;
	}
}
void SkeletonHandler::shieldBoneDefault(boneVertex& vertex) {
	vertex.m_BoneIDs[0] = 38; //38 wrist.L
	vertex.m_Weights[0] = 1.0f;
	for (int i = 1; i < MAX_BONE_WEIGHTS; i++) {
		vertex.m_BoneIDs[i] = 0;
		vertex.m_Weights[i] = 0.0f;
	}
}
template <typename T>
void SkeletonHandler::katanaBoneDefault(T& vertex) {
	return;
}
template <typename T>
void SkeletonHandler::shieldBoneDefault(T& vertex) {
	return;
}
