cmake_minimum_required(VERSION 3.19)
project(chipmunk2d)

add_library(${PROJECT_NAME} STATIC
        src/chipmunk.c
        src/cpArbiter.c
        src/cpArray.c
        src/cpBBTree.c
        src/cpBody.c
        src/cpCollision.c
        src/cpConstraint.c
        src/cpDampedRotarySpring.c
        src/cpDampedSpring.c
        src/cpGearJoint.c
        src/cpGrooveJoint.c
        src/cpHashSet.c
        src/cpHastySpace.c
        src/cpMarch.c
        src/cpPinJoint.c
        src/cpPivotJoint.c
        src/cpPolyShape.c
        src/cpPolyline.c
        src/cpRatchetJoint.c
        src/cpRobust.c
        src/cpRotaryLimitJoint.c
        src/cpShape.c
        src/cpSimpleMotor.c
        src/cpSlideJoint.c
        src/cpSpace.c
        src/cpSpaceComponent.c
        src/cpSpaceDebug.c
        src/cpSpaceHash.c
        src/cpSpaceQuery.c
        src/cpSpaceStep.c
        src/cpSpatialIndex.c
        src/cpSweep1D.c
 )

target_include_directories(${PROJECT_NAME} PUBLIC ./include)
target_include_directories(${PROJECT_NAME} PUBLIC ./src)

set_target_properties(${PROJECT_NAME} PROPERTIES C_STANDARD 11)

add_library(ekx::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

if (EKX_BUILD_EXTERNAL_TESTS)
    add_subdirectory(test)
endif ()
