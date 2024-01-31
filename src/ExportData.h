#pragma once
#include "SkeletalAnimation.h"


//#include <boost/serialization/string.hpp>
//#include <boost/serialization/vector.hpp>
//#include <boost/serialization/utility.hpp>
//
//
////binary
//#include <boost/archive/binary_oarchive.hpp>
//#include <boost/archive/binary_iarchive.hpp>

class ExportData {
public:
    struct boneEData {
        uint32_t boneID{69420};
        glm::mat4 boneTransform{};

        boneEData(uint32_t id, glm::mat4 transform) : boneID{ id }, boneTransform{ transform } {
            //memcpy(boneTransform, glm::value_ptr(transform), sizeof(boneTransform));
        }
        boneEData() {}

        void writeToFile(std::ofstream& outFile) const {
            Writing::UIntToFile(outFile, &boneID);
            Writing::GLMMat4ToFile(outFile, boneTransform);
        }
        void writeToFileSwapEndian(std::ofstream& outFile) const {
            Writing::UIntToFileSwapEndian(outFile, &boneID);
            Writing::GLMMat4ToFileSwapEndian(outFile, boneTransform);
        }
    };
    template<typename V_Type>
    struct TemplateMeshData {
        std::string versionTracker = "";
        std::vector<MeshData<V_Type>> meshes;

        TemplateMeshData(std::vector<V_Type>& vertex, std::vector<uint32_t>& index) {
            meshes.emplace_back(vertex, index);
        }
        TemplateMeshData() : meshes{} {}

        void writeToFile(std::ofstream& outFile, bool endian) const {
            if (endian) {
                for (const auto& mesh : meshes) {
                    mesh.writeToFile(outFile);
                }
            }
            else {
                for (const auto& mesh : meshes) {
                    mesh.writeToFileSwapEndian(outFile);
                }
            }
        }
    };

    struct AnimData {
        std::string versionTracker = "";
        std::vector<glm::mat4> defaultBoneValues; //T-POSE or something, for when an animation doesn't cover everything

        int32_t handBone = -1;

        //animation
        //1 duration will have 1 set of bones, 1 bone will hold a vector of matrix transforms by length of duration

        std::vector< //animation count
            std::vector< //animation duration
            std::vector< //bone count
            boneEData>>> //{bone id, bone transform}, bone ID will keep track of which bone as i clear useless bones. i could also use a map, might be better
            animations;

        void writeToFile(std::ofstream& outFile, bool endian) const {
            uint64_t size = defaultBoneValues.size();
            if (endian) {
                Writing::UInt64ToFile(outFile, &size);

                for (auto const& defaultBone : defaultBoneValues) {
                    Writing::GLMMat4ToFile(outFile, defaultBone);
                }

                size = animations.size(); //animationCount
                Writing::UInt64ToFile(outFile, &size);

                for (auto const& animationDuration : animations) {
                    size = animationDuration.size();
                    Writing::UInt64ToFile(outFile, &size);
                    for (auto const& boneCount : animationDuration) {
                        size = boneCount.size();
                        Writing::UInt64ToFile(outFile, &size);
                        for (auto const& boneData : boneCount) {
                            boneData.writeToFile(outFile);
                        }
                    }
                }
                Writing::IntToFile(outFile, &handBone);
            }
            else {
                Writing::UInt64ToFileSwapEndian(outFile, &size);
                for (auto const& defaultBone : defaultBoneValues) {
                    Writing::GLMMat4ToFileSwapEndian(outFile, defaultBone);
                }
                size = animations.size(); //animationCount
                Writing::UInt64ToFileSwapEndian(outFile, &size);

                for (auto const& animationDuration : animations) {
                    size = animationDuration.size();
                    Writing::UInt64ToFileSwapEndian(outFile, &size);
                    for (auto const& boneCount : animationDuration) {
                        size = boneCount.size();
                        Writing::UInt64ToFileSwapEndian(outFile, &size);
                        for (auto const& boneData : boneCount) {
                            boneData.writeToFileSwapEndian(outFile);
                        }
                    }
                }
                Writing::IntToFileSwapEndian(outFile, &handBone);
            }


        }
    };
    struct FullAnimData {
        std::string versionTracker = "";
        int32_t handBone = -1;

        std::vector< //each animation
            std::vector< //animation frame duration
            std::vector< //boneCount
            glm::mat4>>> animations;

        void writeToFile(std::ofstream& outFile, bool endian) const {
            if (endian) {
                uint64_t size = animations.size(); //animationCount
                Writing::UInt64ToFile(outFile, &size);
                for (auto const& animationDuration : animations) {
                    size = animationDuration.size();
                    Writing::UInt64ToFile(outFile, &size);
                    for (auto const& boneCount : animationDuration) {
                        size = boneCount.size();
                        Writing::UInt64ToFile(outFile, &size);
                        for (auto const& boneData : boneCount) {
                            Writing::GLMMat4ToFile(outFile, boneData);
                        }
                    }
                }
                Writing::IntToFile(outFile, &handBone);
            }
            else {
                uint64_t size = animations.size(); //animationCount
                Writing::UInt64ToFileSwapEndian(outFile, &size);
                for (auto const& animationDuration : animations) {
                    size = animationDuration.size();
                    Writing::UInt64ToFileSwapEndian(outFile, &size);
                    for (auto const& boneCount : animationDuration) {
                        size = boneCount.size();
                        Writing::UInt64ToFileSwapEndian(outFile, &size);
                        for (auto const& boneData : boneCount) {
                            Writing::GLMMat4ToFileSwapEndian(outFile, boneData);
                        }
                    }
                }
                Writing::IntToFileSwapEndian(outFile, &handBone);
            }

        }
    };
    struct NameExportData {
        std::string versionTracker = "";
        std::vector<std::string> meshNames;
        std::vector<std::string> meshNTNames;
        std::vector<std::string> meshSimpleNames;
        std::vector<std::string> meshNTSimpleNames;

        void writeToFile(std::ofstream& outFile) {
            for (auto const& meshName : meshNames) {
                outFile.write(meshName.c_str(), meshName.length());
            }
            for (auto const& meshName : meshNTNames) {
                outFile.write(meshName.c_str(), meshName.length());
            }
            for (auto const& meshName : meshSimpleNames) {
                outFile.write(meshName.c_str(), meshName.length());
            }
            for (auto const& meshName : meshNTSimpleNames) {
                outFile.write(meshName.c_str(), meshName.length());
            }
        }
    };

    TemplateMeshData<boneVertex> meshExport{};
    TemplateMeshData<boneVertexNoTangent> meshNTExport{};
    TemplateMeshData<Vertex> meshSimpleExport{};
    TemplateMeshData<VertexNT> meshNTSimpleExport{};

    AnimData animExport;
    FullAnimData fullAnim;
    NameExportData nameExport;

    void setVersionTracker(std::string version) {
        meshExport.versionTracker = version;
        meshNTExport.versionTracker = version;
        animExport.versionTracker = version;
        nameExport.versionTracker = version;
        fullAnim.versionTracker = version;
    }

    void writeToFile(std::string fileName) {
        uint32_t testValue = 1;
        bool endian = *reinterpret_cast<uint8_t*>(&testValue) == 1;
        endian = false;
        printf("endianness : %d \n", endian);

        if(meshExport.meshes.size() > 0) {
            std::ofstream outFile{ fileName + "_mesh.ewe" };
            if (!outFile.is_open()) {
                outFile.open(fileName + "_mesh.ewe");
                if (!outFile.is_open()) {
                    throw std::runtime_error("failed to open mesh file \n");
                }
            }
            meshExport.writeToFile(outFile, endian);
        }
        if (meshNTExport.meshes.size() > 0) {
            std::ofstream outFile{ fileName + "_meshNT.ewe" };
            if (!outFile.is_open()) {
                outFile.open(fileName + "_meshNT.ewe");
                if (!outFile.is_open()) {
                    throw std::runtime_error("failed to open mesh file \n");
                }
            }
            meshNTExport.writeToFile(outFile, endian);
        }
        if (meshSimpleExport.meshes.size() > 0) {
            std::ofstream outFile{ fileName + "_meshSimple.ewe" };
            if (!outFile.is_open()) {
                outFile.open(fileName + "_meshSimple.ewe");
                if (!outFile.is_open()) {
                    throw std::runtime_error("failed to open mesh file \n");
                }
            }

            meshSimpleExport.writeToFile(outFile, endian);
        }
        if (meshSimpleExport.meshes.size() > 0) {
            std::ofstream outFile{ fileName + "_meshNTSimple.ewe" };
            if (!outFile.is_open()) {
                outFile.open(fileName + "_meshNTSimple.ewe");
                if (!outFile.is_open()) {
                    throw std::runtime_error("failed to open mesh file \n");
                }
            }

            meshNTSimpleExport.writeToFile(outFile, endian);
        }
        if (animExport.animations.size() > 0) {
            std::ofstream outFile{ fileName + "_anim.ewe" };
            if (!outFile.is_open()) {
                outFile.open(fileName + "_anim.ewe");
                if (!outFile.is_open()) {
                    throw std::runtime_error("failed to open mesh file \n");
                }
            }

            animExport.writeToFile(outFile, endian);
        }

        if (fullAnim.animations.size() > 0) {
            std::ofstream outFile{ fileName + "_fullAnim.ewe" };
            if (!outFile.is_open()) {
                outFile.open(fileName + "_fullAnim.ewe");
                if (!outFile.is_open()) {
                    throw std::runtime_error("failed to open mesh file \n");
                }
            }

            fullAnim.writeToFile(outFile, endian);
        }

        {
            std::ofstream outFile{ fileName + "_names.ewe" };
            if (!outFile.is_open()) {
                outFile.open(fileName + "_names.ewe");
                if (!outFile.is_open()) {
                    throw std::runtime_error("failed to open mesh file \n");
                }
            }
            nameExport.writeToFile(outFile);
        }
    }

};

