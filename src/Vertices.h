#pragma once
#pragma once

#include "Bone.h" //bone isnt really relevant here, but it includes glm

#include <fstream>

#define MAX_BONE_INFLUENCE 4
#define MAX_BONE_WEIGHTS 4

namespace Writing {
	void GLM3ToFile(std::ofstream& outFile, glm::vec3 const& vec);
	void GLM3ToFileSwapEndian(std::ofstream& outFile, glm::vec3 const& vec);
	void GLM2ToFile(std::ofstream& outFile, glm::vec2 const& vec);
	void GLM2ToFileSwapEndian(std::ofstream& outFile, glm::vec2 const& vec);

	void IntToFile(std::ofstream& outFile, int const* value);
	void IntToFileSwapEndian(std::ofstream& outFile, int const* value);
	void UIntToFile(std::ofstream& outFile, uint32_t const* value);
	void UIntToFileSwapEndian(std::ofstream& outFile, uint32_t const* value);

	void UInt64ToFile(std::ofstream& outFile, uint64_t const* value);
	void UInt64ToFileSwapEndian(std::ofstream& outFile, uint64_t const* value);

	void FloatToFile(std::ofstream& outFile, float const* value);
	void FloatToFileSwapEndian(std::ofstream& outFile, float const* value);

	void GLMMat4ToFile(std::ofstream& outFile, glm::mat4 const& mat);
	void GLMMat4ToFileSwapEndian(std::ofstream& outFile, glm::mat4 const& mat);
}

struct boneVertexNoTangent {
	glm::vec3 position{};
	glm::vec3 normal{};
	glm::vec2 uv{};
	int m_BoneIDs[MAX_BONE_INFLUENCE];
	float m_Weights[MAX_BONE_INFLUENCE];
	boneVertexNoTangent() {
		for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
			m_BoneIDs[i] = -1;
			m_Weights[i] = 0.f;
		}
	}

	bool operator==(const boneVertexNoTangent& other) const {
		if (!(position == other.position)) {
			//printf("position fail bonevertex \n");
			return false;
		}
		if (!(normal == other.normal)) {
			//printf("normal fail bonevertex \n");
			return false;
		}
		if (!(uv == other.uv)) {
			//printf("uv fail, bonevertex \n");
			return false;
		}
		for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
			if (m_BoneIDs[i] != other.m_BoneIDs[i]) {
				//printf("bone id fail, bone vertex : %d \n", i);
				return false;
			}
			if (m_Weights[i] != other.m_Weights[i]) {

				//printf("bone weight fail, bone vertex : %d \n", i);
				return false;
			}
		}
		return true;
	}

	void writeToFile(std::ofstream& outFile) const {
		Writing::GLM3ToFile(outFile, position);
		Writing::GLM3ToFile(outFile, normal);
		Writing::GLM2ToFile(outFile, uv);

		for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
			Writing::IntToFile(outFile, &m_BoneIDs[i]);
		}
		for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
			Writing::FloatToFile(outFile, &m_Weights[i]);
		}
	}

	void writeToFileSwapEndian(std::ofstream& outFile) const {
		Writing::GLM3ToFileSwapEndian(outFile, position);
		Writing::GLM3ToFileSwapEndian(outFile, normal);
		Writing::GLM2ToFileSwapEndian(outFile, uv);

		for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
			Writing::IntToFileSwapEndian(outFile, &m_BoneIDs[i]);
		}
		for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
			Writing::FloatToFileSwapEndian(outFile, &m_Weights[i]);
		}
	}

};
struct boneVertex {
	glm::vec3 position{};
	glm::vec3 normal{};
	glm::vec2 uv{};
	glm::vec3 tangent{};

	int m_BoneIDs[MAX_BONE_INFLUENCE];
	float m_Weights[MAX_BONE_INFLUENCE];

	boneVertex() {
		for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
			m_BoneIDs[i] = -1;
			m_Weights[i] = 0.f;
		}
	}
	void writeToFile(std::ofstream& outFile) const {
		Writing::GLM3ToFile(outFile, position);
		Writing::GLM3ToFile(outFile, normal);
		Writing::GLM2ToFile(outFile, uv);
		Writing::GLM3ToFile(outFile, tangent);

		for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
			Writing::IntToFile(outFile, &m_BoneIDs[i]);
		}
		for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
			Writing::FloatToFile(outFile, &m_Weights[i]);
		}
	}
	void writeToFileSwapEndian(std::ofstream& outFile) const {

		Writing::GLM3ToFileSwapEndian(outFile, position);
		Writing::GLM3ToFileSwapEndian(outFile, normal);
		Writing::GLM2ToFileSwapEndian(outFile, uv);
		Writing::GLM3ToFileSwapEndian(outFile, tangent);

		for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
			Writing::IntToFileSwapEndian(outFile, &m_BoneIDs[i]);
		}
		for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
			Writing::FloatToFileSwapEndian(outFile, &m_Weights[i]);
		}
	}

	bool operator==(const boneVertex& other) const {
		if (!(this->position == other.position)) {
			//printf("position fail bonevertex \n");
			return false;
		}
		if (!(this->normal == other.normal)) {
			//printf("normal fail bonevertex \n");
			return false;
		}
		if (!(this->uv == other.uv)) {
			//printf("uv fail, bonevertex \n");
			return false;
		}
		if (!(this->tangent == other.tangent)) {
			//printf("tangent fail, bonevertex \n");
			return false;
		}
		for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
			if (this->m_BoneIDs[i] != other.m_BoneIDs[i]) {
				//printf("bone id fail, bone vertex : %d \n", i);
				return false;
			}
			if (this->m_Weights[i] != other.m_Weights[i]) {

				//printf("bone weight fail, bone vertex : %d \n", i);
				return false;
			}
		}
		return true;
	}
};

struct Vertex {
	glm::vec3 position{};
	glm::vec3 normal{};
	glm::vec2 uv{};
	glm::vec3 tangent{};

	bool operator==(const Vertex& other) const {
		if (!(this->position == other.position)) {
			//printf("position fail bonevertex \n");
			return false;
		}
		if (!(this->normal == other.normal)) {
			//printf("normal fail bonevertex \n");
			return false;
		}
		if (!(this->uv == other.uv)) {
			//printf("uv fail, bonevertex \n");
			return false;
		}
		if (!(this->tangent == other.tangent)) {
			//printf("tangent fail, bonevertex \n");
			return false;
		}
		return true;
	}
	void writeToFile(std::ofstream& outFile) const {
		Writing::GLM3ToFile(outFile, position);
		Writing::GLM3ToFile(outFile, normal);
		Writing::GLM2ToFile(outFile, uv);
		Writing::GLM3ToFile(outFile, tangent);
	}

	void writeToFileSwapEndian(std::ofstream& outFile) const {

		Writing::GLM3ToFileSwapEndian(outFile, position);
		Writing::GLM3ToFileSwapEndian(outFile, normal);
		Writing::GLM2ToFileSwapEndian(outFile, uv);
		Writing::GLM3ToFileSwapEndian(outFile, tangent);
	}
};
struct VertexNT {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 uv;

	bool operator==(const VertexNT& other) const {
		if (!(this->position == other.position)) {
			//printf("position fail bonevertex \n");
			return false;
		}
		if (!(this->normal == other.normal)) {
			//printf("normal fail bonevertex \n");
			return false;
		}
		if (!(this->uv == other.uv)) {
			//printf("uv fail, bonevertex \n");
			return false;
		}
		return true;
	}
	void writeToFile(std::ofstream& outFile) const {
		Writing::GLM3ToFile(outFile, position);
		Writing::GLM3ToFile(outFile, normal);
		Writing::GLM2ToFile(outFile, uv);
	}
	void writeToFileSwapEndian(std::ofstream& outFile) const {
		Writing::GLM3ToFileSwapEndian(outFile, position);
		Writing::GLM3ToFileSwapEndian(outFile, normal);
		Writing::GLM2ToFileSwapEndian(outFile, uv);
	}
};

template <typename V_Type>
struct MeshData {
	std::vector<V_Type> vertices;
	std::vector<uint32_t> indices;

	MeshData() {}
	MeshData(std::vector<V_Type> const& vertices, std::vector<uint32_t> const& indices) : vertices{ vertices }, indices{ indices } {}
	MeshData(std::pair<std::vector<V_Type>, std::vector<uint32_t>> const& pairData) : vertices{ pairData.first }, indices{ pairData.second } {}

	void writeToFile(std::ofstream& outFile) const {
		for (auto& vertex : vertices) {
			vertex.writeToFile(outFile);
		}
		for (auto& index : indices) {
			Writing::UIntToFile(outFile, &index);
		}
	}
	void writeToFileSwapEndian(std::ofstream& outFile) const {

		for (auto& vertex : vertices) {
			vertex.writeToFileSwapEndian(outFile);
		}

		for (auto& index : indices) {
			Writing::UIntToFileSwapEndian(outFile, &index);
		}
	}
};