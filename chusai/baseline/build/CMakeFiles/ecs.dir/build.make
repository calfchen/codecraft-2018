# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 2.8

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
CMAKE_SOURCE_DIR = /home/chenxl/2018/demo/ecs

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/chenxl/2018/demo/build

# Include any dependencies generated for this target.
include CMakeFiles/ecs.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/ecs.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/ecs.dir/flags.make

CMakeFiles/ecs.dir/io.cpp.o: CMakeFiles/ecs.dir/flags.make
CMakeFiles/ecs.dir/io.cpp.o: /home/chenxl/2018/demo/ecs/io.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/chenxl/2018/demo/build/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object CMakeFiles/ecs.dir/io.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/ecs.dir/io.cpp.o -c /home/chenxl/2018/demo/ecs/io.cpp

CMakeFiles/ecs.dir/io.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/ecs.dir/io.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/chenxl/2018/demo/ecs/io.cpp > CMakeFiles/ecs.dir/io.cpp.i

CMakeFiles/ecs.dir/io.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/ecs.dir/io.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/chenxl/2018/demo/ecs/io.cpp -o CMakeFiles/ecs.dir/io.cpp.s

CMakeFiles/ecs.dir/io.cpp.o.requires:
.PHONY : CMakeFiles/ecs.dir/io.cpp.o.requires

CMakeFiles/ecs.dir/io.cpp.o.provides: CMakeFiles/ecs.dir/io.cpp.o.requires
	$(MAKE) -f CMakeFiles/ecs.dir/build.make CMakeFiles/ecs.dir/io.cpp.o.provides.build
.PHONY : CMakeFiles/ecs.dir/io.cpp.o.provides

CMakeFiles/ecs.dir/io.cpp.o.provides.build: CMakeFiles/ecs.dir/io.cpp.o

CMakeFiles/ecs.dir/ecs.cpp.o: CMakeFiles/ecs.dir/flags.make
CMakeFiles/ecs.dir/ecs.cpp.o: /home/chenxl/2018/demo/ecs/ecs.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/chenxl/2018/demo/build/CMakeFiles $(CMAKE_PROGRESS_2)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object CMakeFiles/ecs.dir/ecs.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/ecs.dir/ecs.cpp.o -c /home/chenxl/2018/demo/ecs/ecs.cpp

CMakeFiles/ecs.dir/ecs.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/ecs.dir/ecs.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/chenxl/2018/demo/ecs/ecs.cpp > CMakeFiles/ecs.dir/ecs.cpp.i

CMakeFiles/ecs.dir/ecs.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/ecs.dir/ecs.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/chenxl/2018/demo/ecs/ecs.cpp -o CMakeFiles/ecs.dir/ecs.cpp.s

CMakeFiles/ecs.dir/ecs.cpp.o.requires:
.PHONY : CMakeFiles/ecs.dir/ecs.cpp.o.requires

CMakeFiles/ecs.dir/ecs.cpp.o.provides: CMakeFiles/ecs.dir/ecs.cpp.o.requires
	$(MAKE) -f CMakeFiles/ecs.dir/build.make CMakeFiles/ecs.dir/ecs.cpp.o.provides.build
.PHONY : CMakeFiles/ecs.dir/ecs.cpp.o.provides

CMakeFiles/ecs.dir/ecs.cpp.o.provides.build: CMakeFiles/ecs.dir/ecs.cpp.o

CMakeFiles/ecs.dir/devcloud.cpp.o: CMakeFiles/ecs.dir/flags.make
CMakeFiles/ecs.dir/devcloud.cpp.o: /home/chenxl/2018/demo/ecs/devcloud.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/chenxl/2018/demo/build/CMakeFiles $(CMAKE_PROGRESS_3)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object CMakeFiles/ecs.dir/devcloud.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/ecs.dir/devcloud.cpp.o -c /home/chenxl/2018/demo/ecs/devcloud.cpp

CMakeFiles/ecs.dir/devcloud.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/ecs.dir/devcloud.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/chenxl/2018/demo/ecs/devcloud.cpp > CMakeFiles/ecs.dir/devcloud.cpp.i

CMakeFiles/ecs.dir/devcloud.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/ecs.dir/devcloud.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/chenxl/2018/demo/ecs/devcloud.cpp -o CMakeFiles/ecs.dir/devcloud.cpp.s

CMakeFiles/ecs.dir/devcloud.cpp.o.requires:
.PHONY : CMakeFiles/ecs.dir/devcloud.cpp.o.requires

CMakeFiles/ecs.dir/devcloud.cpp.o.provides: CMakeFiles/ecs.dir/devcloud.cpp.o.requires
	$(MAKE) -f CMakeFiles/ecs.dir/build.make CMakeFiles/ecs.dir/devcloud.cpp.o.provides.build
.PHONY : CMakeFiles/ecs.dir/devcloud.cpp.o.provides

CMakeFiles/ecs.dir/devcloud.cpp.o.provides.build: CMakeFiles/ecs.dir/devcloud.cpp.o

CMakeFiles/ecs.dir/predict.cpp.o: CMakeFiles/ecs.dir/flags.make
CMakeFiles/ecs.dir/predict.cpp.o: /home/chenxl/2018/demo/ecs/predict.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/chenxl/2018/demo/build/CMakeFiles $(CMAKE_PROGRESS_4)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object CMakeFiles/ecs.dir/predict.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/ecs.dir/predict.cpp.o -c /home/chenxl/2018/demo/ecs/predict.cpp

CMakeFiles/ecs.dir/predict.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/ecs.dir/predict.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/chenxl/2018/demo/ecs/predict.cpp > CMakeFiles/ecs.dir/predict.cpp.i

CMakeFiles/ecs.dir/predict.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/ecs.dir/predict.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/chenxl/2018/demo/ecs/predict.cpp -o CMakeFiles/ecs.dir/predict.cpp.s

CMakeFiles/ecs.dir/predict.cpp.o.requires:
.PHONY : CMakeFiles/ecs.dir/predict.cpp.o.requires

CMakeFiles/ecs.dir/predict.cpp.o.provides: CMakeFiles/ecs.dir/predict.cpp.o.requires
	$(MAKE) -f CMakeFiles/ecs.dir/build.make CMakeFiles/ecs.dir/predict.cpp.o.provides.build
.PHONY : CMakeFiles/ecs.dir/predict.cpp.o.provides

CMakeFiles/ecs.dir/predict.cpp.o.provides.build: CMakeFiles/ecs.dir/predict.cpp.o

# Object files for target ecs
ecs_OBJECTS = \
"CMakeFiles/ecs.dir/io.cpp.o" \
"CMakeFiles/ecs.dir/ecs.cpp.o" \
"CMakeFiles/ecs.dir/devcloud.cpp.o" \
"CMakeFiles/ecs.dir/predict.cpp.o"

# External object files for target ecs
ecs_EXTERNAL_OBJECTS =

/home/chenxl/2018/demo/bin/ecs: CMakeFiles/ecs.dir/io.cpp.o
/home/chenxl/2018/demo/bin/ecs: CMakeFiles/ecs.dir/ecs.cpp.o
/home/chenxl/2018/demo/bin/ecs: CMakeFiles/ecs.dir/devcloud.cpp.o
/home/chenxl/2018/demo/bin/ecs: CMakeFiles/ecs.dir/predict.cpp.o
/home/chenxl/2018/demo/bin/ecs: CMakeFiles/ecs.dir/build.make
/home/chenxl/2018/demo/bin/ecs: CMakeFiles/ecs.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX executable /home/chenxl/2018/demo/bin/ecs"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/ecs.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/ecs.dir/build: /home/chenxl/2018/demo/bin/ecs
.PHONY : CMakeFiles/ecs.dir/build

CMakeFiles/ecs.dir/requires: CMakeFiles/ecs.dir/io.cpp.o.requires
CMakeFiles/ecs.dir/requires: CMakeFiles/ecs.dir/ecs.cpp.o.requires
CMakeFiles/ecs.dir/requires: CMakeFiles/ecs.dir/devcloud.cpp.o.requires
CMakeFiles/ecs.dir/requires: CMakeFiles/ecs.dir/predict.cpp.o.requires
.PHONY : CMakeFiles/ecs.dir/requires

CMakeFiles/ecs.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/ecs.dir/cmake_clean.cmake
.PHONY : CMakeFiles/ecs.dir/clean

CMakeFiles/ecs.dir/depend:
	cd /home/chenxl/2018/demo/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/chenxl/2018/demo/ecs /home/chenxl/2018/demo/ecs /home/chenxl/2018/demo/build /home/chenxl/2018/demo/build /home/chenxl/2018/demo/build/CMakeFiles/ecs.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/ecs.dir/depend

