#pragma once
#include "SkeletalAnimation.h"


#include <glm/gtc/type_ptr.hpp>


#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/utility.hpp>

/*
//XML
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
*/

//binary
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>



#define bobmat_size 16

class ExportData {
public:
    struct bobmat4 {
        float bmat4[bobmat_size];
        template<class Archive>
        void serialize(Archive& archive, const unsigned int version) {
            archive& bmat4;
        }
        void operator=(float* other) {
            memcpy(bmat4, other, sizeof(bmat4));
        }
        void operator=(glm::mat4& other) {
            memcpy(bmat4, glm::value_ptr(other), sizeof(bmat4));
        }
        bool operator==(bobmat4& other) {
            for (int i = 0; i < bobmat_size; i++) {
                if ((bmat4[i] - other.bmat4[i]) > .0001f) {
                    return false;
                }
            }
            return true;
        }
        bobmat4(float* other) {
            memcpy(bmat4, other, sizeof(bmat4));
        }
        bobmat4(glm::mat4& other) {
            memcpy(bmat4, glm::value_ptr(other), sizeof(bmat4));
        }
        bobmat4() {
            for (int i = 0; i < bobmat_size; i++) {
                bmat4[i] = 1.f;
            }
        }
    };
 
    struct boneEData {
        uint32_t boneID{69420};
        bobmat4 boneTransform{};

        boneEData(uint32_t id, glm::mat4 transform) : boneID{ id }, boneTransform{ transform } {
            //memcpy(boneTransform, glm::value_ptr(transform), sizeof(boneTransform));
        }
        boneEData() {}

        template<class Archive>
        void serialize(Archive& archive, const unsigned int version) {
            archive& boneID;
            archive& boneTransform;
        }
    };
    struct meshEData {
        std::string versionTracker = "";
        std::vector<std::pair<std::vector<boneVertex>, std::vector<uint32_t>>> meshes;

        meshEData(std::vector<boneVertex>& vertex, std::vector<uint32_t>& index) {
            meshes.push_back(std::make_pair(vertex, index));
        }
        meshEData() {}

        template<class Archive>
        void serialize(Archive& archive, const unsigned int version) {
            archive& versionTracker;
            archive& meshes;
        }
    };
    struct meshNTEData {
        std::string versionTracker = "";
        std::vector<std::pair<std::vector<boneVertexNoTangent>, std::vector<uint32_t>>> meshesNT;
        meshNTEData(std::vector<boneVertexNoTangent>& vertex, std::vector<uint32_t>& index) {
            meshesNT.push_back(std::make_pair(vertex, index));
        }
        meshNTEData() {}

        template<class Archive>
        void serialize(Archive& archive, const unsigned int version) {
            archive& versionTracker;
            archive& meshesNT;
        }
    };
    struct meshSimpleData {
        std::vector<std::pair<std::vector<Vertex>, std::vector<uint32_t>>> meshesSimple;
        meshSimpleData(std::vector<Vertex>& vertex, std::vector<uint32_t>& index) {
            meshesSimple.push_back(std::make_pair(vertex, index));
        }
        meshSimpleData() {}
        template<class Archive>
        void serialize(Archive& archive, const unsigned int version) {
            archive& meshesSimple;
        }
    };    
    struct meshNTSimpleData {
        std::vector<std::pair<std::vector<VertexNT>, std::vector<uint32_t>>> meshesNTSimple;
        meshNTSimpleData(std::vector<VertexNT>& vertex, std::vector<uint32_t>& index) {
            meshesNTSimple.push_back(std::make_pair(vertex, index));
        }
        meshNTSimpleData() {}
        template<class Archive>
        void serialize(Archive& archive, const unsigned int version) {
            archive& meshesNTSimple;
        }
    };
    struct AnimData {
        std::string versionTracker = "";
        std::vector<bobmat4> defaultBoneValues; //T-POSE or something, for when an animation doesn't cover everything

        int32_t handBone = -1;

        //animation
        //1 duration will have 1 set of bones, 1 bone will hold a vector of matrix transforms by length of duration

        std::vector< //animation count
            std::vector< //animation duration
            std::vector< //bone count
            boneEData>>> //{bone id, bone transform}, bone ID will keep track of which bone as i clear useless bones. i could also use a map, might be better
            animations;

        template<class Archive>
        void serialize(Archive& archive, const unsigned int version) {
            archive& versionTracker;
            archive& defaultBoneValues;
            archive& animations;
            archive& handBone;
        }
    };
    struct FullAnimData {
        std::string versionTracker = "";
        int32_t handBone = -1;

        std::vector< //each animation
            std::vector< //animation frame duration
            std::vector< //boneCount
            bobmat4>>> animations;

        template<class Archive>
        void serialize(Archive& archive, const unsigned int version) {
            archive& versionTracker;
            archive& animations;
            archive& handBone;
        }
    };
    struct NameExportData {
        std::string versionTracker = "";
        std::vector<std::string> meshNames;
        std::vector<std::string> meshNTNames;
        std::vector<std::string> meshSimpleNames;
        std::vector<std::string> meshNTSimpleNames;

        template<class Archive>
        void serialize(Archive& archive, const unsigned int version) {
            archive& versionTracker;
            archive& meshNTNames;
            archive& meshNames;
            archive& meshSimpleNames;
            archive& meshNTSimpleNames;
        }
    };

    meshEData meshExport;
    meshNTEData meshNTExport;    
    meshSimpleData meshSimpleExport;
    meshNTSimpleData meshNTSimpleExport;
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

};

