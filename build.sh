#!/bin/bash

# If any command errors, stop the script
set -e

CODE_HOME="$(dirname "$0")/.."

# Find the most reasonable candidate build file
SOURCE="$1"
if [ -z "$SOURCE" ]; then
    SOURCE=4coder_leon.cpp
fi

# NOTE(yuval): Removed -Wno-writable-strings as it is the same as -Wno-write-strings
opts="-Wno-write-strings -Wno-null-dereference -Wno-comment -Wno-switch -Wno-missing-declarations -Wno-logical-op-parentheses -g"

arch=-m64

preproc_file=4coder_command_metadata.i
meta_macros="-DMETA_PASS"
clang++ -I"$CODE_HOME" $meta_macros $arch $opts $debug -std=gnu++0x "$SOURCE" -E -o $preproc_file
clang++ -I"$CODE_HOME" $opts $debug -std=gnu++0x "$CODE_HOME/4coder_metadata_generator.cpp" -o "$CODE_HOME/metadata_generator"
"$CODE_HOME/metadata_generator" -R "$CODE_HOME" "$PWD/$preproc_file"

clang++ -I"$CODE_HOME" $arch $opts $debug -std=c++11 "$SOURCE" -shared -o custom_4coder.so -fPIC

rm "$CODE_HOME/metadata_generator"
rm $preproc_file
cp custom_4coder.so ../../
