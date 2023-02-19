# CBuild
An easy-to-use Windows build system for C written in C++.
CBuild is simple to setup and will automatically keep track of any source files that needs to be rebuilt.
Supported compilers: `(gcc, avr-gcc)`

## Usage
In order to use CBuild, simply download the latest release and include it in your `PATH` environment variable (optional, but recommended).  

Next, create a `.cbuild` file in the root directory of your project and include the following:  
```
//Basic example of using CBuild.
set_compiler "gcc";           //Specifies what C compiler to use. (defaults to gcc)
set_project_name "Tutorial";  //Specifies the project name.
add_src_dirs "src;           //Specifies one or multiple directories of source files.
set_obj_output "obj";         //Specifies the output of directory the compiled obj files.
set_build_output "bin";       //Specifies the directory of the final binary/library.
set_build_name "tutorial";    //Specifies the name of the final binary/library.
```
Refer to the [Command List](https://github.com/Zekronz/CBuild#command-list) for a list of all commands.  
In order to build your project, open a command prompt in the same directory as your `.cbuild` file and run `cbuild 'name_of_build_file'`.

## Build Flags
```
-fr/force_rebuild   - Forces CBuild to rebuild every source file.
-release            - Compiles in release mode (defaults to debug mode).
-pcmds              - Prints out the compiler's build commands.
```

## Command List
```
set_compiler "name"                           - What C compiler to use. (default: gcc, supports: gcc, avr-gcc)  
set_compiler_dir "dir"                        - Directory of compiler binaries. (default: gcc, supports: gcc, avr-gcc)  
set_project_name "name"                       - Project name. (only used internally by CBuild)  
set_build_type "type"                         - Build type. (default: binary, supports: binary, static_lib)  
set_build_output "dir"                        - Output directory of build.  
set_obj_output "dir"                          - Directory of compiled obj files.  
set_precompiled_header/set_pch "header_file"  - Optional precompiled header file. CBuild will scan the source directories to find the header file if you don't specify the path. 
set_run_binary true/false                     - Whether or not to run the executable after building.  
add_src_dirs "dir1" "dir2" ...                - Add one or more directories of source files.  
add_src_files "file1" "file2" ...             - Add one of more source files. 
add_incl_dirs "dir1" "dir2" ...               - Add one or more include directories.  
add_lib_dirs "dir1" "dir2" ...                - Add one or more library directories.  
add_static_libs "lib1" "lib2" ...             - Add one or more static libraries.
set_avr_mcu "mcu"                             - The AVR microcontroller that is being used.
set_atmel_studio_dir "dir"                    - Directory of Atmel Studio binaries.
```

## Work in Progress
- Support for more compilers (clang is currently in the works).
- Support for dynamic libraries.

## Credits
CBuild uses [spdlog](https://github.com/gabime/spdlog) for logging.