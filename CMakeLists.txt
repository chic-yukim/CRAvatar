cmake_minimum_required(VERSION 3.12)
project(CRAvatar-Projects LANGUAGES CXX)

# === configure ====================================================================================
if(NOT EXISTS ${CRSF_SDK_DIR})
    message(FATAL_ERROR "'crsf' directory does NOT exist!\nYou can download from http://crseed.org")
endif()

# list modules
add_subdirectory("simple_ik")

# list projects
add_subdirectory("CRAvatar")
# ==================================================================================================
