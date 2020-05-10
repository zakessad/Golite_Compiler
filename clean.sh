#!/bin/bash

# Remove the generated python files

rm ./internal-test/3-benchmarks+codegen/valid/*.py > /dev/null 2>&1
rm ./internal-test/3-semantics+codegen/valid/*.py > /dev/null 2>&1
rm ./prgrams/4-benchmarks+codegen/valid/*.py > /dev/null 2>&1
rm ./prgrams/3-semantics+codegen/valid/*.py > /dev/null 2>&1
