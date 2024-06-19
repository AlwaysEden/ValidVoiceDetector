# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(Main)

FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE ${app_sources})

# cmake_minimum_required(VERSION 3.20.0)

# # Zephyr 프로젝트 설정
# find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
# project(Main)

# # 어셈블리어 파일 설정
# FILE(GLOB asm_sources src/*.s)

# # C 소스 파일 설정
# FILE(GLOB app_sources src/*.c)

# # 타겟 소스 설정 (C와 어셈블리어 파일 포함)
# target_sources(app PRIVATE ${app_sources} ${asm_sources})