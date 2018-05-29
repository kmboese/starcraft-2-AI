include_directories("../include")
include_directories("common")

# Setup MSVC parallelized builds
add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/MP>)

# Included for project
include_directories(SYSTEM "${PROJECT_BINARY_DIR}/generated")
include_directories(SYSTEM "${CMAKE_SOURCE_DIR}/contrib/SDL-mirror/include")

# Function to generate pathfinder project with extra libraries
function (pathfinder_project_extra project_name source_code extra_libs)
    message("Adding new pathfinder project ${project_name}, sources: ${source_code}, extra_libs: ${extra_libs}")
    file (GLOB pathfinder_project_sources ${source_code} "common/*.h" "common/*.cc")
    add_executable(${project_name} ${pathfinder_project_sources})

    set_target_properties(${project_name} PROPERTIES FOLDER pathfinder)

	if (MSVC)
        message("NODEFAULTLIB command reached")
		#set_target_properties(${project_name} PROPERTIES LINK_FLAGS "/NODEFAULTLIB:libcmt;libconcrt")
        set_target_properties(${project_name} PROPERTIES LINK_FLAGS "/NODEFAULTLIB:libcmt")
	endif ()

	if (MSCV AND (NOT (MSVC_VERSION LESS 1910)))
		target_link_libraries(${project_name} legacy_stdio_definitions)
	endif ()

    target_link_libraries(${project_name} sc2api sc2lib sc2utils ${extra_libs})
endfunction ()

# Function to generate a pathfinder project
function (pathfinder_project project_name source_code)
    pathfinder_project_extra(${project_name} ${source_code} "")
endfunction ()

# pathfinder_project(project "project.cc")

if (NOT APPLE)
    pathfinder_project_extra(project "project.cc" sc2renderer)
endif ()