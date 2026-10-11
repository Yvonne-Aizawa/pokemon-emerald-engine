cmake_minimum_required(VERSION 3.16)
project(poke_battle_engine LANGUAGES C)

option(PB_BUILD_TESTS "Build deterministic headless tests" ON)

add_library(poke_battle STATIC src/core/poke_battle.c src/data/data.c)
target_include_directories(poke_battle PUBLIC include)
target_include_directories(poke_battle PRIVATE src)
target_compile_features(poke_battle PUBLIC c_std_11)
target_compile_options(poke_battle PRIVATE -Wall -Wextra -Wpedantic)

if(PB_BUILD_TESTS)
    enable_testing()
    add_executable(poke_battle_test tests/poke_battle_test.c)
    target_link_libraries(poke_battle_test PRIVATE poke_battle)
    add_test(NAME poke_battle_determinism COMMAND poke_battle_test)
endif()
