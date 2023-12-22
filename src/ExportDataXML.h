#pragma once
#include "SkeletalAnimation.h"
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/vector.hpp>

//XML
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>


//binary
//#include <boost/archive/binary_oarchive.hpp>
//#include <boost/archive/binary_iarchive.hpp>
//#include <boost/serialization/binary_object.hpp>



class ExportData {


public:
    struct bobmat4 {
        float yo[4][4];

        bobmat4(glm::mat4 glm4) {
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    yo[i][j] = glm4[i][j];
                }
            }
        }
        bobmat4(float initFloat = 1.f) {
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    yo[i][j] = initFloat;
                }
            }
        }

        template<class Archive>
        void serialize(Archive& archive, const unsigned int version) {
            archive& (yo);
        }
        inline bool operator==(bobmat4& other) {
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    if ((this->yo[i][j] - other.yo[i][j]) > .0001f) {
                        //printf("this, other %.2f:%.2f \n", this->yo[i][j], other.yo[i][j]);
                        return false;
                    }
                }
            }
            return true;
        }
        void print() {
            std::string printString = "printing matrix values? \n";
            std::string printLine = " % .1f, % .1f, % .1f, % .1f, \n";

            printf(printString.c_str());
            printf(printLine.c_str(), yo[0][0], yo[0][1], yo[0][2], yo[0][3]);
            printf(printLine.c_str(), yo[1][0], yo[1][1], yo[1][2], yo[1][3]);
            printf(printLine.c_str(), yo[2][0], yo[2][1], yo[2][2], yo[2][3]);
            printf(printLine.c_str(), yo[3][0], yo[3][1], yo[3][2], yo[3][3]);
        }

    };

    struct boneEData {
        uint32_t boneID{69420};
        bobmat4 boneTransform{ 1.f };

        boneEData(uint32_t id, glm::mat4 transform) : boneID{ id }, boneTransform{ transform }{}
        boneEData() {}

        template<class Archive>
        void serialize(Archive& archive, const unsigned int version) {
            archive& BOOST_SERIALIZATION_NVP(boneID);
            archive& BOOST_SERIALIZATION_NVP(boneTransform);
        }

    };
    struct meshEData {
        std::vector<boneVertex> vertices;
        std::vector<uint32_t> indices;

        meshEData(std::vector<boneVertex>& vertex, std::vector<uint32_t>& index) {
            vertices = vertex;
            indices = index;
        }
        meshEData() {}

        template<class Archive>
        void serialize(Archive& archive, const unsigned int version) {
            archive& BOOST_SERIALIZATION_NVP(vertices);
            archive& BOOST_SERIALIZATION_NVP(indices);
        }
    };


	std::vector <meshEData> meshes;
    
    //animation
    //1 duration will have 1 set of bones, 1 bone will hold a vector of matrix transforms by length of duration

	std::vector< //animation count
        std::vector< //animation duration
            std::vector< //bone count
                boneEData>>> //{bone id, bone transform}, bone ID will keep track of which bone as i clear useless bones. i could also use a map, might be better
                animations;
    //doing it like this ^ has an entire animation frames bone final matrices sequential

    //doing it like this \/ has 1 bones data sequential, when pulling a final bone matrix ill have to pull from non-sequential areas
    /*
    std::vector< //animations count, inside of here is all the information for one animation
        std::vector< //bone count, i think it makes the most sense to put bones here, then i can delete the entire bone with 1 function
            std::vector<
                bobmat4>>> //this is the animation duration
                animationData;
    */
    template<class Archive>
    void serialize(Archive& archive, const unsigned int version) {
        ///read/write from/to archive
        archive& BOOST_SERIALIZATION_NVP(meshes);
        archive& BOOST_SERIALIZATION_NVP(animations);
    }
    
};

