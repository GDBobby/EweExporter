include(./.env.cmake OPTIONAL RESULT_VARIABLE LOCAL_ENV)
message(STATUS "Local .env.cmake: ${LOCAL_ENV}")

cmake_minimum_required(VERSION 3.23)

project(EweExporter VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 2. Set ASSIMP_SDK_PATH in .env.cmake to target specific boost version
if (DEFINED ASSIMP_SDK_PATH)
  set(ASSIMP_INCLUDE_DIR "${ASSIMP_SDK_PATH}/include") # 1.1 Make sure this include path is correct
  set(ASSIMP_LIB "${ASSIMP_SDK_PATH}/lib") # 1.2 Make sure lib path is correct
  set(ASSIMP_FOUND "True")
else()
  message(STATUS "ASSIMP path undefined in .env")
  find_package(Assimp REQUIRED) # throws error if could not find boost
  message(STATUS "Found ASSIMP: $ENV{ASSIMP_SDK}")
endif()
if (NOT ASSIMP_FOUND)
	message(FATAL_ERROR "Could not find ASSIMP library!")
else()
	message(STATUS "Using ASSIMP lib at: ${ASSIMP_LIB}")
endif()

if (NOT GLM_PATH)
  message(STATUS "GLM_PATH not specified in .env.cmake, using external/glm")
  set(GLM_PATH external/include)
endif()


file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src/*.*)

add_executable(${PROJECT_NAME} ${SOURCES})

message(STATUS "including directories")
target_include_directories(${PROJECT_NAME} PUBLIC
	${ASSIMP_INCLUDE_DIR}
    ${GLM_PATH}
)
message(STATUS "linking directories")
target_link_directories(${PROJECT_NAME} PUBLIC
	${ASSIMP_LIB}
)

message(STATUS "linking libs")
target_link_libraries(${PROJECT_NAME}
	$<$<CONFIG:Debug>:assimp-vc143-mtd>
	$<$<CONFIG:Release>:assimp-vc143-mt>
)
	
# Create source groups to maintain file structure in Visual Studio
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})