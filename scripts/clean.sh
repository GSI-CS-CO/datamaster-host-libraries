#!/bin/bash

# shellcheck source=.container-helpers
. "$(dirname "$0")/.container-helpers"
reopen_script_in_container

rm -rf "$(dirname "$0")/../build"
rm -rf "$(dirname "$0")/../out"