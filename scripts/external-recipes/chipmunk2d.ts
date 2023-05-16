import * as path from "path";
import { rm } from "../../lib/utils/utils.js";
import { downloadFiles } from "../../lib/utils/download.js";
import { resolveEkxPath } from "../../lib/utils/dirs.js";

const destPath = resolveEkxPath("external/chipmunk2d");

async function clean() {
    await rm(path.join(destPath, "include"));
    await rm(path.join(destPath, "src"));
}

async function fetch() {
    await downloadFiles({
        srcBaseUrl: "https://github.com/slembcke/Chipmunk2D/raw/master",
        destPath: destPath,
        fileList: [
            // Source files
            "src/chipmunk.c",
            "src/cpArbiter.c",
            "src/cpArray.c",
            "src/cpBBTree.c",
            "src/cpBody.c",
            "src/cpCollision.c",
            "src/cpConstraint.c",
            "src/cpDampedRotarySpring.c",
            "src/cpDampedSpring.c",
            "src/cpGearJoint.c",
            "src/cpGrooveJoint.c",
            "src/cpHashSet.c",
            "src/cpHastySpace.c",
            "src/cpMarch.c",
            "src/cpPinJoint.c",
            "src/cpPivotJoint.c",
            "src/cpPolyShape.c",
            "src/cpPolyline.c",
            "src/cpRatchetJoint.c",
            "src/cpRobust.c",
            "src/cpRotaryLimitJoint.c",
            "src/cpShape.c",
            "src/cpSimpleMotor.c",
            "src/cpSlideJoint.c",
            "src/cpSpace.c",
            "src/cpSpaceComponent.c",
            "src/cpSpaceDebug.c",
            "src/cpSpaceHash.c",
            "src/cpSpaceQuery.c",
            "src/cpSpaceStep.c",
            "src/cpSpatialIndex.c",
            "src/cpSweep1D.c",
            "src/prime.h",

            // Headers
            "include/chipmunk/chipmunk.h",
            "include/chipmunk/chipmunk_ffi.h",
            "include/chipmunk/chipmunk_private.h",
            "include/chipmunk/chipmunk_structs.h",
            "include/chipmunk/chipmunk_types.h",
            "include/chipmunk/chipmunk_unsafe.h",
            "include/chipmunk/cpArbiter.h",
            "include/chipmunk/cpBB.h",
            "include/chipmunk/cpBody.h",
            "include/chipmunk/cpConstraint.h",
            "include/chipmunk/cpDampedRotarySpring.h",
            "include/chipmunk/cpDampedSpring.h",
            "include/chipmunk/cpGearJoint.h",
            "include/chipmunk/cpGrooveJoint.h",
            "include/chipmunk/cpHastySpace.h",
            "include/chipmunk/cpMarch.h",
            "include/chipmunk/cpPinJoint.h",
            "include/chipmunk/cpPivotJoint.h",
            "include/chipmunk/cpPolyShape.h",
            "include/chipmunk/cpPolyline.h",
            "include/chipmunk/cpRatchetJoint.h",
            "include/chipmunk/cpRobust.h",
            "include/chipmunk/cpRotaryLimitJoint.h",
            "include/chipmunk/cpShape.h",
            "include/chipmunk/cpSimpleMotor.h",
            "include/chipmunk/cpSlideJoint.h",
            "include/chipmunk/cpSpace.h",
            "include/chipmunk/cpSpatialIndex.h",
            "include/chipmunk/cpTransform.h",
            "include/chipmunk/cpVect.h",
        ],
    });
}

export default { clean, fetch };