#!/bin/bash

cd $(dirname "$0")/..

NEED_UPDATE="false"

# Functions

function updateDependencyByVersion {
    # $1 lua script file to extract version
    # $2 last known version

    SCRIPT="$1"
    LAST="$2"

    NAME=$(cat "${SCRIPT}" | grep NAME -m 1 | cut -d '"' -f2)
    CURRENT=$(cat "${SCRIPT}" | grep VERSION -m 1 | cut -d'"' -f2)

    echo -e "${NAME}:\n    ${CURRENT} (current)\n    ${LAST} (last)"

    if [ "${CURRENT}" != "${LAST}" ]; then
        NEED_UPDATE="true"

        sed -i "s/${CURRENT}/${LAST}/" "${SCRIPT}"
        git add "${SCRIPT}" > /dev/null
        git commit -m "Updated ${NAME} to ${LAST}" > /dev/null
        echo "    Marked ${NAME} to be updated to ${LAST}."
    else
        echo "    Already up-to-date."
    fi

    echo ""
}

function updateDependencyByDate {
    # $1 lua script file to extract repository
    # $2 last known timestamp

    SCRIPT="$1"
    LAST="$2"

    NAME=$(cat "${SCRIPT}" | grep NAME -m 1 | cut -d '"' -f2)
    FILE=$(cat "${SCRIPT}" | grep fileExist -m 1 | cut -d'"' -f2)

    CURRENT=$(stat "./external/${FILE}" --format="%Y" 2> /dev/null)

    if [ -z "${CURRENT}" ]; then
        NEED_UPDATE="true"

        echo -e "${NAME}:\n    Never downloaded.\n    Marked to be updated.\n"
        return
    fi

    echo -e "${NAME}:\n    ${CURRENT} (last download)\n    ${LAST} (remote last commit)"

    if [ "${CURRENT}" -lt "${LAST}" ]; then
        NEED_UPDATE="true"

        rm -rf external/${FILE}
        echo "    Marked ${NAME} to be updated."
    else
        echo "    Already up-to-date."
    fi

    echo ""
}

# Bullet
LAST=$(wget https://github.com/bulletphysics/bullet3/tags -q -O - | grep '\.zip' -m 1 | cut -d'"' -f2 | rev | cut -d'/' -f1 | cut -d'.' -f2- | rev)
updateDependencyByVersion "external/bullet.lua" "${LAST}"

# GLM
LAST=$(wget https://github.com/g-truc/glm/tags -q -O - | grep '\.zip' -m 1 | cut -d'"' -f2 | rev | cut -d'/' -f1 | cut -d'.' -f2- | rev)
updateDependencyByVersion "external/glm.lua" "${LAST}"

# Nlohmann JSON
LAST=$(wget https://github.com/nlohmann/json/tags -q -O - | grep '\.zip' -m 1 | cut -d'"' -f2 | rev | cut -d'v' -f1 | cut -d'.' -f2- | rev)
updateDependencyByVersion "external/nlohmann-json.lua" "${LAST}"

# STB libraries
LAST=$(wget https://github.com/nothings/stb/commits/master -q -O - | grep relative-time -m 1 | cut -d'"' -f2 | xargs date +"%s" -d)
updateDependencyByDate "external/stb.lua" "${LAST}"

# Vulkan SDK
LAST=$(wget https://vulkan.lunarg.com/sdk/home -q -O - | grep linux | cut -d '"' -f2)
updateDependencyByVersion "external/vulkan-sdk.lua" "${LAST}"

# Do update

if [ "${NEED_UPDATE}" == "true" ]; then
    echo "Updating..."
    ./scripts/setup.sh
fi
