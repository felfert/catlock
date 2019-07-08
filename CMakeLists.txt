cmake_minimum_required(VERSION 3.10)
project("catlock"
    LANGUAGES CXX
    VERSION 1.0
    DESCRIPTION "Lock keyboard and pointer"
)
include(GNUInstallDirs)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Build type" FORCE)
endif()
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror")
if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    if (${CMAKE_CXX_COMPILER_VERSION} VERSION_GREATER 6)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wduplicated-branches -Wrestrict")
    endif()
    if (${CMAKE_CXX_COMPILER_VERSION} VERSION_GREATER 5)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wduplicated-cond -Wnull-dereference")
    endif()
    if (${CMAKE_CXX_COMPILER_VERSION} VERSION_GREATER 4.7)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wuseless-cast")
    endif()
    if (${CMAKE_CXX_COMPILER_VERSION} VERSION_GREATER 4.2)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wlogical-op")
    endif()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wold-style-cast -Wshadow")
endif()
add_definitions(-DVERSION=\"${PROJECT_VERSION}\")

find_package(XCB COMPONENTS KEYSYMS IMAGE DPMS REQUIRED)
find_package(X11 REQUIRED)
find_package(POPT REQUIRED)
find_package(XmlTo REQUIRED)

add_executable(catlock catlock.cpp)
target_link_libraries(catlock ${XCB_LIBRARIES} ${X11_LIBRARIES} ${POPT_LIBRARIES})
install(TARGETS catlock RUNTIME DESTINATION ${BINDIR} ${CMAKE_INSTALL_BINDIR})
install(FILES catlock.desktop DESTINATION "${CMAKE_INSTALL_DATADIR}/applications")
install(FILES catlock.png DESTINATION "${CMAKE_INSTALL_DATADIR}/pixmaps")

configure_file(catlock.xml.in catlock.xml @ONLY)
XMLTO(catlock.xml MODES man ALL)
foreach(file ${XMLTO_FILES_man})
    get_filename_component(fileExt ${file} EXT)
    string( REGEX REPLACE "^[.]" "" fileExt ${fileExt} )
    install(
        FILES ${file}.gz
        DESTINATION ${CMAKE_INSTALL_MANDIR}/man${fileExt}
    )
endforeach()

# Add ghrelease target
add_custom_target(ghrelease
    COMMAND tar czf "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}-${CMAKE_PROJECT_VERSION}.tar.gz"
    --exclude=.git --exclude=build --owner root --group root --transform
    "s/^\\./${CMAKE_PROJECT_NAME}-${CMAKE_PROJECT_VERSION}/" .
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMENT "Creating source tarball"
    COMMAND sh .github/release "${GITHUB_USER}" "${CMAKE_PROJECT_NAME}" "${CMAKE_PROJECT_VERSION}"
        "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}-${CMAKE_PROJECT_VERSION}.tar.gz"
    VERBATIM
)
