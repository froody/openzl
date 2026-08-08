# Copyright (c) Meta Platforms, Inc. and affiliates.

if(NOT TARGET fuzz_octree_occupancy)
    add_executable(fuzz_octree_occupancy
        fuzz_octree_occupancy.cpp
    )
    target_link_libraries(fuzz_octree_occupancy PRIVATE openzl)
    apply_openzl_compile_options_to_target(fuzz_octree_occupancy)
endif()

if(NOT TARGET fuzz_morton_3d)
    add_executable(fuzz_morton_3d
        fuzz_morton_3d.cpp
    )
    target_link_libraries(fuzz_morton_3d PRIVATE openzl)
    target_compile_definitions(fuzz_morton_3d PRIVATE STANDALONE_FUZZER)
    apply_openzl_compile_options_to_target(fuzz_morton_3d)
endif()

if(NOT TARGET fuzz_tans4_interleaved)
    add_executable(fuzz_tans4_interleaved
        fuzz_tans4_interleaved.cpp
    )
    target_link_libraries(fuzz_tans4_interleaved PRIVATE openzl)
    apply_openzl_compile_options_to_target(fuzz_tans4_interleaved)
endif()

if(NOT TARGET fuzz_bitplane_split)
    add_executable(fuzz_bitplane_split
        fuzz_bitplane_split.cpp
    )
    target_link_libraries(fuzz_bitplane_split PRIVATE openzl)
    apply_openzl_compile_options_to_target(fuzz_bitplane_split)
endif()

if(NOT TARGET fuzz_elf_erase)
    add_executable(fuzz_elf_erase
        fuzz_elf_erase.cpp
    )
    target_link_libraries(fuzz_elf_erase PRIVATE openzl)
    apply_openzl_compile_options_to_target(fuzz_elf_erase)
endif()

if(NOT TARGET fuzz_spherical_range)
    add_executable(fuzz_spherical_range
        fuzz_spherical_range.cpp
    )
    target_link_libraries(fuzz_spherical_range PRIVATE openzl)
    apply_openzl_compile_options_to_target(fuzz_spherical_range)
endif()

if(NOT TARGET fuzz_quars)
    add_executable(fuzz_quars
        fuzz_quars.cpp
    )
    target_link_libraries(fuzz_quars PRIVATE openzl)
    apply_openzl_compile_options_to_target(fuzz_quars)
endif()

if(NOT TARGET fuzz_btrblocks_selector)
    add_executable(fuzz_btrblocks_selector
        fuzz_btrblocks_selector.cpp
    )
    target_include_directories(fuzz_btrblocks_selector PRIVATE ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/src)
    target_link_libraries(fuzz_btrblocks_selector PRIVATE openzl openzl_cpp tools_training logger)
    apply_openzl_compile_options_to_target(fuzz_btrblocks_selector)
endif()

if(NOT TARGET fuzz_linear_predict)
    add_executable(fuzz_linear_predict
        fuzz_linear_predict.cpp
        ${OPENZL_SRC_DIR}/openzl/codecs/linear_predict/encode_linear_predict_kernel.c
        ${OPENZL_SRC_DIR}/openzl/codecs/linear_predict/decode_linear_predict_kernel.c
    )
    target_include_directories(fuzz_linear_predict PRIVATE
        ${OPENZL_INCLUDE_DIR}
        ${OPENZL_SRC_DIR}
    )
    target_link_libraries(fuzz_linear_predict PRIVATE openzl)
    apply_openzl_compile_options_to_target(fuzz_linear_predict)
endif()

if(NOT TARGET fuzz_octree_occupancy)
    add_executable(fuzz_octree_occupancy
        fuzz_octree_occupancy.cpp
        ${OPENZL_SRC_DIR}/openzl/codecs/octree_occupancy/encode_octree_occupancy_kernel.c
        ${OPENZL_SRC_DIR}/openzl/codecs/octree_occupancy/decode_octree_occupancy_kernel.c
    )
    target_include_directories(fuzz_octree_occupancy PRIVATE
        ${OPENZL_INCLUDE_DIR}
        ${OPENZL_SRC_DIR}
    )
    target_link_libraries(fuzz_octree_occupancy PRIVATE openzl)
    apply_openzl_compile_options_to_target(fuzz_octree_occupancy)
endif()
