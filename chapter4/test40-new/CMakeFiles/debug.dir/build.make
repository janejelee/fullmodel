# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.5

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /scratch/janelee/deal.II-v9.0.0/fullmodel/chapter4/test40-new

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /scratch/janelee/deal.II-v9.0.0/fullmodel/chapter4/test40-new

# Utility rule file for debug.

# Include the progress variables for this target.
include CMakeFiles/debug.dir/progress.make

CMakeFiles/debug:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/scratch/janelee/deal.II-v9.0.0/fullmodel/chapter4/test40-new/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Switch CMAKE_BUILD_TYPE to Debug"
	/usr/bin/cmake -DCMAKE_BUILD_TYPE=Debug /scratch/janelee/deal.II-v9.0.0/fullmodel/chapter4/test40-new
	/usr/bin/cmake --build /scratch/janelee/deal.II-v9.0.0/fullmodel/chapter4/test40-new --target all

debug: CMakeFiles/debug
debug: CMakeFiles/debug.dir/build.make

.PHONY : debug

# Rule to build all files generated by this target.
CMakeFiles/debug.dir/build: debug

.PHONY : CMakeFiles/debug.dir/build

CMakeFiles/debug.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/debug.dir/cmake_clean.cmake
.PHONY : CMakeFiles/debug.dir/clean

CMakeFiles/debug.dir/depend:
	cd /scratch/janelee/deal.II-v9.0.0/fullmodel/chapter4/test40-new && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /scratch/janelee/deal.II-v9.0.0/fullmodel/chapter4/test40-new /scratch/janelee/deal.II-v9.0.0/fullmodel/chapter4/test40-new /scratch/janelee/deal.II-v9.0.0/fullmodel/chapter4/test40-new /scratch/janelee/deal.II-v9.0.0/fullmodel/chapter4/test40-new /scratch/janelee/deal.II-v9.0.0/fullmodel/chapter4/test40-new/CMakeFiles/debug.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/debug.dir/depend

