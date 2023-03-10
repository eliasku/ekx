import json
import os
import sys
import re

#from sys import print

# PBXPROJ: https://github.com/kronenthaler/mod-pbxproj/wiki
from pbxproj import XcodeProject, PBXGenericObject, XCBuildConfigurationFlags
from pbxproj.pbxextensions import FileOptions

config_file = open("ek-ios-build.json", "r")
config_data = json.loads(config_file.read())

print('Number of arguments: ' + str(len(sys.argv)) + ' arguments')
print('Argument List: ')
for arg in sys.argv:
    print(' * ' + arg)

proj_ios_name = sys.argv[1]
application_id = sys.argv[2]

def set_cpp_flags_for_files(project, files, flags):
    for file_path in files:
        print(file_path)
        # check absolute path
        for file in project.get_files_by_path(file_path, '<absolute>'):
            print(file)
            for build_file in project.get_build_files_for_file(file.get_id()):
                build_file.add_compiler_flags(flags)
        # check relative path
        rel_path = os.path.relpath(file_path, ".")
        for file in project.get_files_by_path(rel_path):
            for build_file in project.get_build_files_for_file(file.get_id()):
                build_file.add_compiler_flags(flags)

excludes = [
    "^build$",
    "^CMakeLists.txt$",
    "^.DS_Store$",
    "^.*\.md$",
    "^.*\.js$",
    "^.*\.glsl$",
    "^.*\.sh$"
]

mainTargetName = "app-ios"

project = XcodeProject.load(f"{mainTargetName}.xcodeproj/project.pbxproj")

project_target = project.get_target_by_name(mainTargetName)

# required by Crashlytics
project.set_flags("DEBUG_INFORMATION_FORMAT", "dwarf-with-dsym")

project.set_flags("GCC_ENABLE_CPP_EXCEPTIONS", "NO")
project.set_flags("GCC_ENABLE_CPP_RTTI", "NO")
project.set_flags("GCC_ENABLE_OBJC_EXCEPTIONS", "NO")
project.set_flags("GCC_OPTIMIZATION_LEVEL", "z", configuration_name='Release')
project.set_flags("LLVM_LTO", "YES", configuration_name='Release')

project_target.productName = proj_ios_name
project.set_flags("PRODUCT_BUNDLE_IDENTIFIER", application_id)
project.set_flags("IPHONEOS_DEPLOYMENT_TARGET", "12.0")

header_search_paths = ["$(inherited)"]
caps = []
compileDefines = ["$(inherited)"]

def apply_module_settings(decl, group):
    if "assets" in decl:
        for src in decl["assets"]:
            project.add_file("assets", parent=group, force=True)

    if "cpp" in decl:
        for src in decl["cpp"]:
            my_add_folder(src, parent=project.add_group("src", parent=group), excludes=excludes)
            header_search_paths.append(src)

    if "cpp_include" in decl:
        for src in decl["cpp_include"]:
            project.add_folder(src, parent=group, recursive=True, create_groups=False, excludes=excludes, file_options=FileOptions(create_build_files=False, ignore_unknown_type=True, embed_framework=False))
            header_search_paths.append(src)

    if "cpp_define" in decl:
        for define in decl["cpp_define"]:
            compileDefines.append("-D" + define.replace("\"", "\\\""))

    if "cpp_flags" in decl:
        for flags in decl["cpp_flags"]:
            if "files" in flags and "flags" in flags:
                set_cpp_flags_for_files(project, flags["files"], flags["flags"])

    if "xcode_framework" in decl:
        for framework in decl["xcode_framework"]:
            project.add_file("System/Library/Frameworks/" + framework + ".framework",
                             tree='SDKROOT', force=False,
                             file_options=FileOptions(weak=False, embed_framework=False))
    if "xcode_capability" in decl:
        for capability in decl["xcode_capability"]:
            caps.append(capability)

    if "xcode_file" in decl:
        for file in decl["xcode_file"]:
            project.add_file(file)

def my_add_folder(path, parent, excludes):
    file_options = FileOptions()

    if not os.path.isdir(path):
        return None

    # iterate over the objects in the directory
    for child in os.listdir(path):
        # exclude dirs or files matching any of the expressions
        if [pattern for pattern in excludes if re.match(pattern, child)]:
            continue

        full_path = os.path.join(path, child)
        if os.path.isfile(full_path):
            # check if the file exists already, if not add it
            project.add_file(full_path, parent, target_name=mainTargetName, force=False,
                             tree='<absolute>', file_options=file_options)
        else:
            new_parent = project.add_group(child, child, parent)
            my_add_folder(full_path, new_parent, excludes)

modules_group = project.add_group("modules")
for module in config_data["modules"]:
    moduleName = "global"
    if "name" in module:
        moduleName = module["name"]
    group = project.add_group(moduleName, parent=modules_group)
    apply_module_settings(module, group)

project.add_header_search_paths(header_search_paths)

if caps:
    sys_caps = PBXGenericObject()
    for cap in caps:
        sys_caps[cap] = PBXGenericObject()
        sys_caps[cap]["enabled"] = 1

    project.objects[project.rootObject].attributes.TargetAttributes[project_target.get_id()][
        'SystemCapabilities'] = sys_caps

project.add_other_ldflags("$(inherited)")
project.add_library_search_paths("$(inherited)")
project.add_other_cflags(compileDefines)

# after ios 12.2.0
project.add_search_paths(XCBuildConfigurationFlags.LD_RUNPATH_SEARCH_PATHS, "/usr/lib/swift", False)

project.save()
