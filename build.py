#!/usr/bin/python3

import sys
import subprocess
from io import TextIOBase

from halo import Halo

class colors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    OKPUR = '\u001b[35;1m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'
class options:
    verbose = False
    verbose_commands = True
    run_argument = "gen" # Types are: `exec` for Executable, `gen` for file generated by the Executable (.o file), `none` for Nothing
    build_arg = "--debug"
    clean_build = False
    keep_cache = True
    debug = False
commands = {
    "cxx-cmake": "cmake . -GNinja -Bbuild ",
    "wasm-cmake": "emcmake cmake ./wasm-lib -Wno-dev -GNinja -Bbuild -DZLIB_LIBRARY=/home/dev0/Programming/emsdk/upstream/emscripten/cache/sysroot/lib/wasm32-emscripten/libz.a -DZLIB_INCLUDE_DIR=/usr/include/ -DLLVM_DIR=/home/dev0/Programming/llvm-project/build-wasm/lib/cmake/llvm",
    "cxx-build": "ninja -Cbuild -j8",
    "wasm-build": "ninja -Cbuild -j8",
    "wasm-copy-wasm": "cp ./build/VIRELANG.wasm ./wasm-build/VIRELANG.wasm",
    "wasm-copy-js": "cp ./build/VIRELANG.js ./wasm-build/VIRELANG.js",
    "wasm-zip-wasm": "gzip -k --best -f ./VIRELANG.wasm",
    "cxx-run": "./VIRELANG",
    "cxx-run-gen": "clang++ res/test.cpp test.o -o test -no-pie",
    "cxx-run-gen-exec": "./test",
}
build_types = {
    "--release": "Release",
    "-r": "Release",

    "--debug": "Debug",
    "-d": "Debug",

    "--minsize": "MinSizeRel",
    "-minsz": "MinSizeRel",
}

def run_command(command_list, run_verbose=False, cwd="./"):
    if run_verbose:
        return subprocess.run(command_list, cwd=cwd)

    return subprocess.run(command_list, stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT, cwd=cwd)
def clean_cmake_cache(opts):
    if opts.keep_cache and not opts.clean_build:
        return;

    subprocess.run(["rm", "./build/CMakeCache.txt", "-f"])
    subprocess.run(["rm", "-r", "./build/CMakeFiles", "-f"])
    print(f"{colors.OKGREEN}Cleaned CMakeCache{colors.ENDC}")

def build_cxx(opts):
    ##########
    print(f"{colors.OKBLUE}{colors.BOLD}Building to native target...{colors.ENDC}\n---")

    ##########
    clean_cmake_cache(opts)
    
    ##########
    exec_command = commands["cxx-cmake"].split()
    exec_command.append("-DCMAKE_BUILD_TYPE="+build_types[opts.build_arg])

    cmd_text = "CMake command"
    if(opts.verbose_commands):
        cmd_text += ": \"" + commands["cxx-cmake"] + "\""
    
    stre=TextIOBase()
    with Halo(text=f"Executing {cmd_text}\n", spinner="arc", placement="right") as spinner:
        run_command(exec_command, run_verbose=opts.verbose)
    print(f"{colors.OKGREEN}Executed {cmd_text}{colors.ENDC}")

    ##########
    exec_command = commands["cxx-build"].split()

    cmd_text = "build command"
    if(opts.verbose_commands):
        cmd_text += ": \"" + commands["cxx-build"] + "\""

    with Halo(text=f"Executing {cmd_text}\n", spinner="arc", placement="right"):
        run_command(exec_command, run_verbose=opts.verbose)
    print(f"{colors.OKGREEN}Executed {cmd_text}{colors.ENDC}")
    print(f"{colors.OKGREEN}Build succeeded{colors.ENDC}")

    ##########
    if opts.run_argument == "none":
        return
    
    print(f"{colors.OKGREEN}Running executable{colors.ENDC}")
    
    cmd_text=commands["cxx-run"]
    if opts.debug:
        cmd_text="valgrind "+cmd_text
    run_command(cmd_text.split(), run_verbose=True, cwd="./build")
    if opts.run_argument == "gen":
        run_command(commands["cxx-run-gen"].split(), run_verbose=True, cwd="./build")
        run_command(commands["cxx-run-gen-exec"].split(), run_verbose=True, cwd="./build")
def build_wasm(opts):
    ##########
    print(f"{colors.OKBLUE}{colors.BOLD}Building to native target...{colors.ENDC}\n---")

    ##########
    clean_cmake_cache(opts)
    
    ##########
    exec_command = commands["wasm-cmake"].split()

    cmd_text = "CMake command"
    if(opts.verbose_commands):
        cmd_text += ": \"" + commands["wasm-cmake"] + "\""

    with Halo(text=f"Executing {cmd_text}\n", spinner="arc", placement="right"):
        run_command(exec_command, run_verbose=opts.verbose)
    print(f"{colors.OKGREEN}Executed {cmd_text}{colors.ENDC}")

    ##########
    exec_command = commands["wasm-build"].split()

    cmd_text = "build command"
    if(opts.verbose_commands):
        cmd_text += ": \"" + commands["wasm-build"]

    with Halo(text=f"Executing {cmd_text}\n", spinner="arc", placement="right"):
        run_command(exec_command, run_verbose=opts.verbose)
    print(f"{colors.OKGREEN}Executed {cmd_text}{colors.ENDC}")
    print(f"{colors.OKGREEN}Build succeeded{colors.ENDC}")

    ##########
    with Halo(text="Copying generated files to wasm-build directory\n", spinner="arc", placement="right"):
        run_command(commands["wasm-copy-wasm"].split(), run_verbose=opts.verbose)
        run_command(commands["wasm-copy-js"].split(), run_verbose=opts.verbose)
    print(f"{colors.OKGREEN}Copied generated files to wasm-build directory{colors.ENDC}")

    ##########
    with Halo(text="Compressing generated WASM file to .gz\n", spinner="arc", placement="right"):
        run_command(commands["wasm-zip-wasm"].split(), run_verbose=opts.verbose, cwd="./wasm-build")
    print(f"{colors.OKGREEN}Compressed generated WASM file to .gz{colors.ENDC}")
def build_entry(opts):
    if opts.clean_build:
        subprocess.run(["rm","-rf","./build"])
        subprocess.run(["mkdir","build"])

    if "--wasm" in sys.argv:
        build_wasm(opts)
    else:
        build_cxx(opts)

def main():
    opts = options()

    for arg in sys.argv:
        if arg in build_types:
            opts.build_arg=arg
            break

    if ("--clean" in sys.argv) or ("-cln" in sys.argv):
        opts.clean_build = True
    elif ("--keep-cache" in sys.argv) or ("-kc" in sys.argv):
        opts.keep_cache = True

    if ("--compile" in sys.argv) or ("-c" in sys.argv):
        opts.run_argument = "none"
    elif ("--run-exec" in sys.argv) or ("-re" in sys.argv):
        opts.run_argument = "exec"
    elif ("--run-gen" in sys.argv) or ("-rg" in sys.argv):
        opts.run_argument = "gen"
    
    if ("--verbose-all" in sys.argv) or ("-va" in sys.argv):
        opts.verbose = True
        opts.verbose_commands= True
    elif ("--verbose" in sys.argv) or ("-v" in sys.argv):
        opts.verbose = False
        opts.verbose_commands = True
    elif ("--silent" in sys.argv) or ("-s" in sys.argv):
        opts.verbose = False
        opts.verbose_commands = False
    
    if ("-dbg" in sys.argv) or ("--debug" in sys.argv):
        opts.debug=True

    build_entry(opts)

if(__name__ == "__main__"):
    main()