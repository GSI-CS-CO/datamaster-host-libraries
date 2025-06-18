#!/bin/bash

set -e
# This script syncs the current project with the BEL projects repository.

BEL_REPO_LOCATION="${BEL_REPO_LOCATION:-/home/${USER}/projects/bel_projects}"
DM_BEL_REPO_LOCATION="${DM_BEL_REPO_LOCATION:-${BEL_REPO_LOCATION}/modules/ftm/}"

for file in $(find modules/datamaster-host-libraries/src -type f -name "*.h"); do
    # test if file is in the BEL repository
    if [[ ! -f "${DM_BEL_REPO_LOCATION}/include/$(basename "$file")" ]]; then
        echo "ERROR: $(basename "$file") not found in BEL repository, skipping it"
    else
        echo "Syncing $(basename "$file") from BEL repository"
        cp "${DM_BEL_REPO_LOCATION}/include/$(basename "$file")" "$file"
    fi
done

for file in $(find modules/datamaster-host-libraries/src -type f -name "*.cpp"); do
    # test if file is in the BEL repository
    if [[ ! -f "${DM_BEL_REPO_LOCATION}/src/$(basename "$file")" ]]; then
        echo "ERROR: $(basename "$file") not found in BEL repository, skipping it"
    else
        echo "Syncing $(basename "$file") from BEL repository"
        cp "${DM_BEL_REPO_LOCATION}/src/$(basename "$file")" "$file"
    fi
done