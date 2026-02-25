include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

option(USE_PORT_SUFFIX "Add a port specific suffix to the package name of Rodos" OFF)
if (USE_PORT_SUFFIX)
    if (NOT DEFINED port_name)
        cmake_path(GET CMAKE_TOOLCHAIN_FILE STEM port_name)
    endif ()
    set(package rodos_${port_name})
else ()
    set(package rodos)
endif ()
# Print name of the package to use when calling find_package(<package>) in consumer code
message("Rodos package name = ${package}")

# Copy toolchain files to the install src directory
set(rodos_INSTALL_SRCDIR src/rodos)
install(
    DIRECTORY ${CMAKE_SOURCE_DIR}/cmake/port/
    DESTINATION ${rodos_INSTALL_SRCDIR}/cmake/port
)

# Copy linker script to install src directory if necessary
if (DEFINED linker_script)
    string(LENGTH "${CMAKE_SOURCE_DIR}/" path_prefix_length)
    string(SUBSTRING "${linker_script}" "${path_prefix_length}" -1 relative_linker_script)
    cmake_path(REMOVE_FILENAME relative_linker_script OUTPUT_VARIABLE relative_linker_script_directory)
    install(
        FILES ${linker_script}
        DESTINATION ${rodos_INSTALL_SRCDIR}/${relative_linker_script_directory}
    )
endif ()

# Copy all necessary header files to the install include directory
set(rodos_INSTALL_INCLUDEDIR ${CMAKE_INSTALL_INCLUDEDIR}/rodos)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/api/
    DESTINATION ${rodos_INSTALL_INCLUDEDIR}/api
    FILES_MATCHING PATTERN "*.h"
)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/src/
    DESTINATION ${rodos_INSTALL_INCLUDEDIR}/src/
    FILES_MATCHING PATTERN "*.h"
)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/support/support-libs/
    DESTINATION ${rodos_INSTALL_INCLUDEDIR}/support/support-libs
    FILES_MATCHING PATTERN "*.h"
)

# Some of the directories_to_include must also be included in the INSTALL_INTERFACE. For simplicity
# I add all of them and use this foreach loop cause I don't know how to do this the install(TARGET
# ... INCLUDES DESTINATION ...) way.
foreach(dir ${directories_to_include})
    target_include_directories(rodos_rodos PUBLIC $<INSTALL_INTERFACE:${rodos_INSTALL_INCLUDEDIR}/${dir}>)
endforeach()

set(rodos_INSTALL_LIBDIR ${CMAKE_INSTALL_LIBDIR}/${package})

# Define how to install which targets and create an export. If possible define the include
# directories for the exported targets here.
install(
    TARGETS rodos_rodos EXPORT rodosTargets
    ARCHIVE
        DESTINATION ${rodos_INSTALL_LIBDIR}
    INCLUDES
        DESTINATION
            ${rodos_INSTALL_INCLUDEDIR}/api
            ${rodos_INSTALL_INCLUDEDIR}/api/hal
            ${rodos_INSTALL_INCLUDEDIR}/src/${port_dir}
            ${rodos_INSTALL_INCLUDEDIR}/support/support-libs
            $<$<BOOL:is_port_baremetal>:${rodos_INSTALL_INCLUDEDIR}/src/bare-metal-generic>
            $<$<BOOL:is_port_baremetal>:${rodos_INSTALL_INCLUDEDIR}/src/bare-metal-generic/ip/lwip/include>
)

write_basic_package_version_file("${package}ConfigVersion.cmake"
    VERSION ${rodos_VERSION}
    COMPATIBILITY SameMajorVersion
)

set(rodos_INSTALL_CMAKEDIR ${rodos_INSTALL_LIBDIR}/cmake)

install(FILES cmake/install-config.cmake
    DESTINATION ${rodos_INSTALL_CMAKEDIR}
    RENAME "${package}Config.cmake"
)

install(FILES "${PROJECT_BINARY_DIR}/${package}ConfigVersion.cmake"
    DESTINATION ${rodos_INSTALL_CMAKEDIR}
)

# Generate and install a CMake file with code for installing targets from the given export
install(EXPORT rodosTargets
    NAMESPACE rodos::
    DESTINATION ${rodos_INSTALL_CMAKEDIR}
)

# Target for removing all installed files
if (NOT TARGET uninstall)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
        IMMEDIATE @ONLY
    )
    add_custom_target(
        uninstall
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
    )
endif ()

# Target for removing all installed files and directories
if (NOT TARGET uninstall-all)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall-all.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall-all.cmake"
        IMMEDIATE @ONLY
    )
    add_custom_target(
        uninstall-all
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall-all.cmake
    )
endif ()

