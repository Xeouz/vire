# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

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
CMAKE_SOURCE_DIR = /home/dev0/Programming/Virelang

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/dev0/Programming/Virelang/build

# Include any dependencies generated for this target.
include CMakeFiles/vire-parser.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/vire-parser.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/vire-parser.dir/flags.make

CMakeFiles/vire-parser.dir/src/Vire/AST-Parse/parser.cpp.o: CMakeFiles/vire-parser.dir/flags.make
CMakeFiles/vire-parser.dir/src/Vire/AST-Parse/parser.cpp.o: ../src/Vire/AST-Parse/parser.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/dev0/Programming/Virelang/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/vire-parser.dir/src/Vire/AST-Parse/parser.cpp.o"
	/home/dev0/Programming/llvm-project/build/bin/clang++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/vire-parser.dir/src/Vire/AST-Parse/parser.cpp.o -c /home/dev0/Programming/Virelang/src/Vire/AST-Parse/parser.cpp

CMakeFiles/vire-parser.dir/src/Vire/AST-Parse/parser.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/vire-parser.dir/src/Vire/AST-Parse/parser.cpp.i"
	/home/dev0/Programming/llvm-project/build/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/dev0/Programming/Virelang/src/Vire/AST-Parse/parser.cpp > CMakeFiles/vire-parser.dir/src/Vire/AST-Parse/parser.cpp.i

CMakeFiles/vire-parser.dir/src/Vire/AST-Parse/parser.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/vire-parser.dir/src/Vire/AST-Parse/parser.cpp.s"
	/home/dev0/Programming/llvm-project/build/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/dev0/Programming/Virelang/src/Vire/AST-Parse/parser.cpp -o CMakeFiles/vire-parser.dir/src/Vire/AST-Parse/parser.cpp.s

# Object files for target vire-parser
vire__parser_OBJECTS = \
"CMakeFiles/vire-parser.dir/src/Vire/AST-Parse/parser.cpp.o"

# External object files for target vire-parser
vire__parser_EXTERNAL_OBJECTS =

libvire-parser.a: CMakeFiles/vire-parser.dir/src/Vire/AST-Parse/parser.cpp.o
libvire-parser.a: CMakeFiles/vire-parser.dir/build.make
libvire-parser.a: CMakeFiles/vire-parser.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/dev0/Programming/Virelang/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX static library libvire-parser.a"
	$(CMAKE_COMMAND) -P CMakeFiles/vire-parser.dir/cmake_clean_target.cmake
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/vire-parser.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/vire-parser.dir/build: libvire-parser.a

.PHONY : CMakeFiles/vire-parser.dir/build

CMakeFiles/vire-parser.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/vire-parser.dir/cmake_clean.cmake
.PHONY : CMakeFiles/vire-parser.dir/clean

CMakeFiles/vire-parser.dir/depend:
	cd /home/dev0/Programming/Virelang/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/dev0/Programming/Virelang /home/dev0/Programming/Virelang /home/dev0/Programming/Virelang/build /home/dev0/Programming/Virelang/build /home/dev0/Programming/Virelang/build/CMakeFiles/vire-parser.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/vire-parser.dir/depend

