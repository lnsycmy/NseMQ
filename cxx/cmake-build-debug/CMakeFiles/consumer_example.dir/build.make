# CMAKE generated file: DO NOT EDIT!
# Generated by "NMake Makefiles" Generator, CMake Version 3.17

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


.SUFFIXES: .hpux_make_needs_suffix_list


# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE
NULL=nul
!ENDIF
SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = "D:\Development\JetBrains Clion\CLion 2020.2.1\bin\cmake\win\bin\cmake.exe"

# The command to remove a file.
RM = "D:\Development\JetBrains Clion\CLion 2020.2.1\bin\cmake\win\bin\cmake.exe" -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = E:\Project\NseMQ\cxx

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = E:\Project\NseMQ\cxx\cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles\consumer_example.dir\depend.make

# Include the progress variables for this target.
include CMakeFiles\consumer_example.dir\progress.make

# Include the compile flags for this target's objects.
include CMakeFiles\consumer_example.dir\flags.make

CMakeFiles\consumer_example.dir\examples\consumer_example.cpp.obj: CMakeFiles\consumer_example.dir\flags.make
CMakeFiles\consumer_example.dir\examples\consumer_example.cpp.obj: ..\examples\consumer_example.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=E:\Project\NseMQ\cxx\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/consumer_example.dir/examples/consumer_example.cpp.obj"
	"D:\Development\Visual Studio 2015\VC\bin\amd64\cl.exe" @<<
 /nologo /TP $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) /FoCMakeFiles\consumer_example.dir\examples\consumer_example.cpp.obj /FdCMakeFiles\consumer_example.dir\ /FS -c E:\Project\NseMQ\cxx\examples\consumer_example.cpp
<<

CMakeFiles\consumer_example.dir\examples\consumer_example.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/consumer_example.dir/examples/consumer_example.cpp.i"
	"D:\Development\Visual Studio 2015\VC\bin\amd64\cl.exe" > CMakeFiles\consumer_example.dir\examples\consumer_example.cpp.i @<<
 /nologo /TP $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E E:\Project\NseMQ\cxx\examples\consumer_example.cpp
<<

CMakeFiles\consumer_example.dir\examples\consumer_example.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/consumer_example.dir/examples/consumer_example.cpp.s"
	"D:\Development\Visual Studio 2015\VC\bin\amd64\cl.exe" @<<
 /nologo /TP $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) /FoNUL /FAs /FaCMakeFiles\consumer_example.dir\examples\consumer_example.cpp.s /c E:\Project\NseMQ\cxx\examples\consumer_example.cpp
<<

# Object files for target consumer_example
consumer_example_OBJECTS = \
"CMakeFiles\consumer_example.dir\examples\consumer_example.cpp.obj"

# External object files for target consumer_example
consumer_example_EXTERNAL_OBJECTS =

consumer_example.exe: CMakeFiles\consumer_example.dir\examples\consumer_example.cpp.obj
consumer_example.exe: CMakeFiles\consumer_example.dir\build.make
consumer_example.exe: NseMQ++.lib
consumer_example.exe: CMakeFiles\consumer_example.dir\objects1.rsp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=E:\Project\NseMQ\cxx\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable consumer_example.exe"
	"D:\Development\JetBrains Clion\CLion 2020.2.1\bin\cmake\win\bin\cmake.exe" -E vs_link_exe --intdir=CMakeFiles\consumer_example.dir --rc=C:\PROGRA~2\WI3CF2~1\8.1\bin\x64\rc.exe --mt=C:\PROGRA~2\WI3CF2~1\8.1\bin\x64\mt.exe --manifests  -- "D:\Development\Visual Studio 2015\VC\bin\amd64\link.exe" /nologo @CMakeFiles\consumer_example.dir\objects1.rsp @<<
 /out:consumer_example.exe /implib:consumer_example.lib /pdb:E:\Project\NseMQ\cxx\cmake-build-debug\consumer_example.pdb /version:0.0  /machine:x64 /debug /INCREMENTAL /subsystem:console   -LIBPATH:E:\Project\NseMQ\cxx\lib  -LIBPATH:D:\Program\boost1.72.0\boost_1_72_0\lib64-msvc-14.0  librdkafka.lib avrocpp.lib NseMQ++.lib librdkafka.lib avrocpp.lib kernel32.lib user32.lib gdi32.lib winspool.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comdlg32.lib advapi32.lib 
<<

# Rule to build all files generated by this target.
CMakeFiles\consumer_example.dir\build: consumer_example.exe

.PHONY : CMakeFiles\consumer_example.dir\build

CMakeFiles\consumer_example.dir\clean:
	$(CMAKE_COMMAND) -P CMakeFiles\consumer_example.dir\cmake_clean.cmake
.PHONY : CMakeFiles\consumer_example.dir\clean

CMakeFiles\consumer_example.dir\depend:
	$(CMAKE_COMMAND) -E cmake_depends "NMake Makefiles" E:\Project\NseMQ\cxx E:\Project\NseMQ\cxx E:\Project\NseMQ\cxx\cmake-build-debug E:\Project\NseMQ\cxx\cmake-build-debug E:\Project\NseMQ\cxx\cmake-build-debug\CMakeFiles\consumer_example.dir\DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles\consumer_example.dir\depend

