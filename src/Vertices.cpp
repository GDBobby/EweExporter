#include "Vertices.h"


namespace Writing {
	void GLM3ToFile(std::ofstream& outFile, glm::vec3 const& vec) {
		outFile.write((char*)&vec.x, sizeof(float));
		outFile.write((char*)&vec.y, sizeof(float));
		outFile.write((char*)&vec.z, sizeof(float));
	}
	void GLM3ToFileSwapEndian(std::ofstream& outFile, glm::vec3 const& vec) {
		uint32_t value;
		memcpy(&value, &vec.x, sizeof(float));
		value = ((value & 0xFF) << 24) | (((value >> 8) & 0xFF) << 16) | (((value >> 16) & 0xFF) << 8) | ((value >> 24) & 0xFF);
		outFile.write((char*)&value, sizeof(float));
		memcpy(&value, &vec.y, sizeof(float));
		value = ((value & 0xFF) << 24) | (((value >> 8) & 0xFF) << 16) | (((value >> 16) & 0xFF) << 8) | ((value >> 24) & 0xFF);
		outFile.write((char*)&value, sizeof(float));
		memcpy(&value, &vec.z, sizeof(float));
		value = ((value & 0xFF) << 24) | (((value >> 8) & 0xFF) << 16) | (((value >> 16) & 0xFF) << 8) | ((value >> 24) & 0xFF);
		outFile.write((char*)&value, sizeof(float));
	}

	void GLM2ToFile(std::ofstream& outFile, glm::vec2 const& vec) {
		outFile.write((char*)&vec.x, sizeof(float));
		outFile.write((char*)&vec.y, sizeof(float));
	}

	void GLM2ToFileSwapEndian(std::ofstream& outFile, glm::vec2 const& vec) {
		uint32_t value;
		memcpy(&value, &vec.x, sizeof(float));
		value = ((value & 0xFF) << 24) | (((value >> 8) & 0xFF) << 16) | (((value >> 16) & 0xFF) << 8) | ((value >> 24) & 0xFF);
		outFile.write((char*)&value, sizeof(float));
		memcpy(&value, &vec.y, sizeof(float));
		value = ((value & 0xFF) << 24) | (((value >> 8) & 0xFF) << 16) | (((value >> 16) & 0xFF) << 8) | ((value >> 24) & 0xFF);
		outFile.write((char*)&value, sizeof(float));
	}

	void IntToFile(std::ofstream& outFile, int const* value) {
		outFile.write((char*)value, sizeof(int));
	}
	void UIntToFile(std::ofstream& outFile, uint32_t const* value) {
		outFile.write((char*)value, sizeof(uint32_t));
	}
	void UInt64ToFile(std::ofstream& outFile, uint64_t const* value) {
		outFile.write((char*)value, sizeof(uint64_t));
	}
	void UInt64ToFileSwapEndian(std::ofstream& outFile, uint64_t const* value) {
		uint64_t outValue = *value;
		outValue = ((outValue & 0xFF) << 56) | (((outValue >> 8) & 0xFF) << 48) |
			(((outValue >> 16) & 0xFF) << 40) | (((outValue >> 24) & 0xFF) << 32) |
			(((outValue >> 32) & 0xFF) << 24) | (((outValue >> 40) & 0xFF) << 16) |
			(((outValue >> 48) & 0xFF) << 8) | ((outValue >> 56) & 0xFF);
		outFile.write((char*)&outValue, sizeof(uint64_t));
	}

	void UIntToFileSwapEndian(std::ofstream& outFile, uint32_t const* value) {
		uint32_t outValue = *value;
		outValue = ((outValue & 0xFF) << 24) | (((outValue >> 8) & 0xFF) << 16) | (((outValue >> 16) & 0xFF) << 8) | ((outValue >> 24) & 0xFF);
		outFile.write((char*)&outValue, sizeof(uint32_t));
	}
	void FloatToFile(std::ofstream& outFile, float const* value) {
		outFile.write((char*)value, sizeof(float));
	}

	void IntToFileSwapEndian(std::ofstream& outFile, int const* value) {
		uint32_t outValue;
		memcpy(&outValue, value, sizeof(int));
		outValue = ((outValue & 0xFF) << 24) | (((outValue >> 8) & 0xFF) << 16) | (((outValue >> 16) & 0xFF) << 8) | ((outValue >> 24) & 0xFF);
		outFile.write((char*)&outValue, sizeof(int));
	}
	void FloatToFileSwapEndian(std::ofstream& outFile, float const* value) {
		uint32_t outValue;
		memcpy(&outValue, value, sizeof(float));
		outValue = ((outValue & 0xFF) << 24) | (((outValue >> 8) & 0xFF) << 16) | (((outValue >> 16) & 0xFF) << 8) | ((outValue >> 24) & 0xFF);
		outFile.write((char*)&outValue, sizeof(float));
	}

	void GLMMat4ToFile(std::ofstream& outFile, glm::mat4 const& mat) {
		uint32_t value;
		for (int x = 0; x < 4; x++) {
			for (int y = 0; y < 4; y++) {
				memcpy(&value, &mat[x][y], sizeof(float));
				outFile.write((char*)&value, sizeof(float));
			}
		}
	}
	void GLMMat4ToFileSwapEndian(std::ofstream& outFile, glm::mat4 const& mat) {
		uint32_t value;
		for (int x = 0; x < 4; x++) {
			for (int y = 0; y < 4; y++) {
				memcpy(&value, &mat[x][y], sizeof(float));
				value = ((value & 0xFF) << 24) | (((value >> 8) & 0xFF) << 16) | (((value >> 16) & 0xFF) << 8) | ((value >> 24) & 0xFF);
				outFile.write((char*)&value, sizeof(float));
			}
		}
	}
}