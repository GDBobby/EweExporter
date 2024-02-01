#pragma once


#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/glm.hpp>
#include <assimp/scene.h>

#include <list>
#include <vector>


struct KeyPosition {
	glm::vec3 position;
	float timeStamp;
	KeyPosition() {}
	KeyPosition(aiVectorKey* key) : timeStamp{ static_cast<float>(key->mTime)} {
		memcpy(&position, &key->mValue, sizeof(float) * 3);

	}
};

struct KeyRotation {
	glm::quat orientation;
	float timeStamp;
	KeyRotation() {}
	KeyRotation(aiQuatKey* key) : orientation{ key->mValue.w, key->mValue.x, key->mValue.y, key->mValue.z }, timeStamp { static_cast<float>(key->mTime) } {
	}
};

struct KeyScale {
	glm::vec3 scale;
	float timeStamp;
	KeyScale(aiVectorKey* key) : timeStamp{ static_cast<float>(key->mTime) } {
		memcpy(&scale, &key->mValue, sizeof(float) * 3);
	}
};

class Bone {
public:
	Bone(aiString const& name, int ID, const aiNodeAnim* channel)
		:
		m_Name{ name.data },
		m_ID{ ID },
		m_LocalTransform{ 1.0f },
		m_NumPositions{channel->mNumPositionKeys},
		m_NumRotations{channel->mNumRotationKeys},
		m_NumScalings{channel->mNumScalingKeys}
	{

		for (int positionIndex = 0; positionIndex < m_NumPositions; ++positionIndex) {
			//aiVector3D aiPosition = channel->mPositionKeys[positionIndex].mValue;
			float timeStamp = channel->mPositionKeys[positionIndex].mTime;
			m_Positions.emplace_back(channel->mPositionKeys + positionIndex);
		}

		for (int rotationIndex = 0; rotationIndex < m_NumRotations; ++rotationIndex) {
			m_Rotations.emplace_back(channel->mRotationKeys + rotationIndex);
		}

		for (int keyIndex = 0; keyIndex < m_NumScalings; ++keyIndex) {
			m_Positions.emplace_back(channel->mScalingKeys + keyIndex);
		}
	}

	void Update(float animationTime) {
		glm::mat4 translation = InterpolatePosition(animationTime);
		glm::mat4 rotation = InterpolateRotation(animationTime);
		glm::mat4 scale = InterpolateScaling(animationTime);
		m_LocalTransform = translation * rotation * scale;
	}
	glm::mat4 GetLocalTransform() { return m_LocalTransform; }
	std::string const& GetBoneName() const { return m_Name; }
	int GetBoneID() { return m_ID; }



	int GetPositionIndex(float animationTime) {
		for (int index = 0; index < m_NumPositions - 1; ++index) {
			if (animationTime < m_Positions[index + 1].timeStamp) {
				return index;
			}
		}
		assert(0);
	}

	int GetRotationIndex(float animationTime) {
		for (int index = 0; index < m_NumRotations - 1; ++index) {
			if (animationTime < m_Rotations[index + 1].timeStamp) {
				return index;
			}
		}
		assert(0);
	}

	int GetScaleIndex(float animationTime) {
		for (int index = 0; index < m_NumScalings - 1; ++index) {
			if (animationTime < m_Scales[index + 1].timeStamp) {
				return index;
			}
		}
		assert(0);
	}


private:

	float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime) {
		float midWayLength = animationTime - lastTimeStamp;
		float framesDiff = nextTimeStamp - lastTimeStamp;
		float scaleFactor = midWayLength / framesDiff;
		return scaleFactor;
	}

	glm::mat4 InterpolatePosition(float animationTime) {
		if (1 == m_NumPositions) { return glm::translate(glm::mat4(1.0f), m_Positions[0].position); }

		int p0Index = GetPositionIndex(animationTime);
		int p1Index = p0Index + 1;
		float scaleFactor = GetScaleFactor(m_Positions[p0Index].timeStamp, m_Positions[p1Index].timeStamp, animationTime);
		glm::vec3 finalPosition = glm::mix(m_Positions[p0Index].position, m_Positions[p1Index].position, scaleFactor);
		return glm::translate(glm::mat4(1.0f), finalPosition);
	}

	glm::mat4 InterpolateRotation(float animationTime) {
		if (m_NumRotations == 1) {
			auto rotation = glm::normalize(m_Rotations[0].orientation);
			return glm::toMat4(rotation);
		}

		int p0Index = GetRotationIndex(animationTime);
		int p1Index = p0Index + 1;
		float scaleFactor = GetScaleFactor(m_Rotations[p0Index].timeStamp, m_Rotations[p1Index].timeStamp, animationTime);
		glm::quat finalRotation = glm::slerp(m_Rotations[p0Index].orientation, m_Rotations[p1Index].orientation, scaleFactor);
		finalRotation = glm::normalize(finalRotation);
		return glm::toMat4(finalRotation);

	}

	glm::mat4 InterpolateScaling(float animationTime) {
		if (m_NumScalings == 1) { return glm::scale(glm::mat4(1.0f), m_Scales[0].scale); }

		int p0Index = GetScaleIndex(animationTime);
		int p1Index = p0Index + 1;
		float scaleFactor = GetScaleFactor(m_Scales[p0Index].timeStamp, m_Scales[p1Index].timeStamp, animationTime);
		glm::vec3 finalScale = glm::mix(m_Scales[p0Index].scale, m_Scales[p1Index].scale, scaleFactor);
		return glm::scale(glm::mat4(1.0f), finalScale);
	}

	std::vector<KeyPosition> m_Positions{};
	std::vector<KeyRotation> m_Rotations{};
	std::vector<KeyScale> m_Scales{};
	uint32_t m_NumPositions;
	uint32_t m_NumRotations;
	uint32_t m_NumScalings;

	glm::mat4 m_LocalTransform;
	std::string m_Name;
	int m_ID;
};


