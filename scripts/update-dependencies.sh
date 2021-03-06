#!/bin/bash

cd $(dirname "$0")/..

NEED_UPDATE="false"

# Functions

function updateSkip {
    # $1 lua script file to extract version
    # $2 last known version

    local script="$1"
    local last="$2"

    local name=$(cat "${script}" | grep NAME -m 1 | cut -d '"' -f2)
    local current=$(cat "${script}" | grep VERSION -m 1 | cut -d'"' -f2)

    echo -e "\e[1m${name}\e[0m\n    ${current} (current)\n    ${last} (last)"
    echo -e "    \e[93mSkipped.\e[39m\n"
}

function updateDependencyByVersion {
    # $1 lua script file to extract version
    # $2 last known version
    # $NEED_UPDATE set to "true" if something changed

    local script="$1"
    local last="$2"

    local name=$(cat "${script}" | grep NAME -m 1 | cut -d '"' -f2)
    local current=$(cat "${script}" | grep VERSION -m 1 | cut -d'"' -f2)

    echo -e "\e[1m${name}\e[0m\n    ${current} (current)\n    ${last} (last)"

    if [ -z "${last}" ]; then
            echo -e "    \e[91mWrong version number.\e[39m"
            echo -e "    \e[93mUpdate ignored.\e[39m\n"
            return
    fi

    if [ "${current}" != "${last}" ]; then
        echo -n -e "    \e[93mUpdate to last?\e[39m (y/N) "
        read -n 1 reply
        if [[ $reply =~ ^[Yy]$ ]]; then
            echo -e "\n    \e[94mMarked ${name} to be updated to ${last}.\e[39m"
            sed -i "s/${current}/${last}/" "${script}"
            git add "${script}" > /dev/null
            git commit -m "Updated ${name} to ${last}" > /dev/null
            NEED_UPDATE="true"
        else
            echo -e "\n    \e[93mUpdate ignored.\e[39m"
        fi
    else
        echo -e "    \e[92mAlready up-to-date.\e[39m"
    fi

    echo ""
}

function updateDependencyByDate {
    # $1 lua script file to extract repository
    # $2 last known timestamp
    # $NEED_UPDATE set to "true" if something changed

    local script="$1"
    local last="$2"

    local name=$(cat "${script}" | grep NAME -m 1 | cut -d '"' -f2)
    local file=$(cat "${script}" | grep fileExist -m 1 | cut -d'"' -f2)

    local current=$(stat "./external/${file}" --format="%Y" 2> /dev/null)

    echo -e "\e[1m${name}\e[0m"

    if [ -z "${current}" ]; then
        NEED_UPDATE="true"

        echo -e "    Never downloaded.\n    \e[94mMarked to be updated.\e[39m\n"
        return
    fi

    echo -e "    ${current} (last download)\n    ${last} (remote last commit)"

    if [ "${current}" -lt "${last}" ]; then
        NEED_UPDATE="true"

        rm -rf external/${file}
        echo -e "    \e[94mMarked ${name} to be updated.\e[39m"
    else
        echo -e "    \e[92mAlready up-to-date.\e[39m"
    fi

    echo ""
}

function extractFromGithubTags {
    # $1 repository name "entity/repo"
    # $LAST return value of version number

    local repository="$1"

    # @note We're not sure there's a v before the version number,
    # so we need rev before cutting
    LAST=$(wget https://api.github.com/repos/${repository}/tags -q -O - | grep -m 1 '"name"' | cut -d'"' -f4 | rev | cut -d'v' -f1 | rev)
}

function extractFromGithubReleases {
    # $1 repository name "entity/repo"
    # $LAST return value of version number

    local repository="$1"

    LAST=$(wget https://api.github.com/repos/${repository}/releases/latest -q -O - | grep -m 1 '"tag_name"' | cut -d'"' -f4 | rev | cut -d'v' -f1 | rev)
}

function extractFromGithubLastCommit {
    # $1 repository name "entity/repo"
    # $LAST return value of version number

    local repository="$1"

    LAST=$(wget https://github.com/${repository}/commits/master -q -O - | grep relative-time -m 1 | cut -d'"' -f2 | xargs date +"%s" -d)
}

# Bullet
extractFromGithubReleases "bulletphysics/bullet3"
updateDependencyByVersion "external/bullet.lua" "${LAST}"

# GLM
extractFromGithubReleases "g-truc/glm"
updateDependencyByVersion "external/glm.lua" "${LAST}"

# MikkTSpace
extractFromGithubLastCommit "tcoppex/ext-mikktspace"
updateDependencyByDate "external/mikktspace.lua" "${LAST}"

# Nlohmann JSON
extractFromGithubReleases "nlohmann/json"
updateDependencyByVersion "external/nlohmann-json.lua" "${LAST}"

# OpenVR
extractFromGithubReleases "ValveSoftware/openvr"
updateDependencyByVersion "external/openvr.lua" "${LAST}"

# Shaderc
extractFromGithubTags "google/shaderc"
updateDependencyByVersion "external/shaderc.lua" "${LAST}"

# STB libraries
extractFromGithubLastCommit "nothings/stb"
updateDependencyByDate "external/stb.lua" "${LAST}"

# Vulkan
# @note Removing patch version and sdk-
LAST=$(wget https://api.github.com/repos/KhronosGroup/Vulkan-Headers/git/refs/tags -q -O - | grep '"refs/tags/sdk-' | tail -1 | cut -d'-' -f2 | cut -d'"' -f1 | cut -d'.' -f1,2,3)
updateDependencyByVersion "external/vulkan.lua" "${LAST}"

# Do update

if [ "${NEED_UPDATE}" == "true" ]; then
    echo -e "\e[1mUpdating...\e[0m"
    ./scripts/setup.sh
fi
