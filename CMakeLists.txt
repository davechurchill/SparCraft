cmake_minimum_required(VERSION 3.24)

project(SparCraft LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/EHsc)
endif()

# Put all executable outputs in <repo>/bin.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
if(CMAKE_CONFIGURATION_TYPES)
  foreach(config IN LISTS CMAKE_CONFIGURATION_TYPES)
    string(TOUPPER "${config}" config_upper)
    set("CMAKE_RUNTIME_OUTPUT_DIRECTORY_${config_upper}" "${CMAKE_SOURCE_DIR}/bin")
  endforeach()
endif()

option(SPARCRAFT_BUILD_BWAPIDATA "Build bundled BWAPIData static library" ON)
option(SPARCRAFT_BUILD_APP "Build the SparCraft executable" ON)
option(SPARCRAFT_BUILD_TESTS "Build the SparCraft test executable" ON)

set(SPARCRAFT_SRC_DIR "${CMAKE_SOURCE_DIR}/src")
set(SPARCRAFT_BWAPIDATA_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/src/bwapidata/include")

file(GLOB SPARCRAFT_CORE_SOURCES CONFIGURE_DEPENDS
  "${SPARCRAFT_SRC_DIR}/*.cpp"
)
list(REMOVE_ITEM SPARCRAFT_CORE_SOURCES "${SPARCRAFT_SRC_DIR}/TutorialCode.cpp")

add_library(SparCraftCore STATIC ${SPARCRAFT_CORE_SOURCES})
target_include_directories(SparCraftCore
  PUBLIC
    "${SPARCRAFT_SRC_DIR}"
    "${SPARCRAFT_BWAPIDATA_INCLUDE_DIR}"
)
target_compile_definitions(SparCraftCore PUBLIC NOMINMAX)

if(SPARCRAFT_BUILD_BWAPIDATA)
  file(GLOB SPARCRAFT_BWAPIDATA_SOURCES CONFIGURE_DEPENDS
    "${SPARCRAFT_BWAPIDATA_INCLUDE_DIR}/*.cpp"
    "${SPARCRAFT_BWAPIDATA_INCLUDE_DIR}/Util/*.cpp"
  )

  add_library(BWAPIData STATIC ${SPARCRAFT_BWAPIDATA_SOURCES})
  target_include_directories(BWAPIData PUBLIC "${SPARCRAFT_BWAPIDATA_INCLUDE_DIR}")
  target_compile_definitions(BWAPIData PUBLIC NOMINMAX)
else()
  add_library(BWAPIData INTERFACE)
  target_include_directories(BWAPIData INTERFACE "${SPARCRAFT_BWAPIDATA_INCLUDE_DIR}")
endif()

target_link_libraries(SparCraftCore PUBLIC BWAPIData)

if(SPARCRAFT_BUILD_TESTS)
  add_executable(SparCraftTests
    "${SPARCRAFT_SRC_DIR}/test/main_test.cpp"
    "${SPARCRAFT_SRC_DIR}/test/catch2/catch_amalgamated.cpp"
  )

  target_include_directories(SparCraftTests
    PRIVATE
      "${SPARCRAFT_SRC_DIR}"
      "${SPARCRAFT_SRC_DIR}/test/catch2"
  )

  target_compile_definitions(SparCraftTests PRIVATE NOMINMAX)
  target_link_libraries(SparCraftTests PRIVATE SparCraftCore)

  if(MSVC)
    target_compile_options(SparCraftTests PRIVATE /EHsc)
  endif()
endif()

if(SPARCRAFT_BUILD_APP)
  find_package(SFML 3 COMPONENTS System Window Graphics Audio REQUIRED)
  find_package(OpenGL REQUIRED)

  file(GLOB SPARCRAFT_MAIN_SOURCES CONFIGURE_DEPENDS
    "${SPARCRAFT_SRC_DIR}/main/*.cpp"
  )
  file(GLOB SPARCRAFT_GUI_SOURCES CONFIGURE_DEPENDS
    "${SPARCRAFT_SRC_DIR}/gui/*.cpp"
  )
  file(GLOB SPARCRAFT_IMGUI_SOURCES CONFIGURE_DEPENDS
    "${SPARCRAFT_SRC_DIR}/imgui/*.cpp"
  )

  add_executable(SparCraft
    ${SPARCRAFT_MAIN_SOURCES}
    ${SPARCRAFT_GUI_SOURCES}
    ${SPARCRAFT_IMGUI_SOURCES}
    "${SPARCRAFT_SRC_DIR}/TutorialCode.cpp"
  )

  target_include_directories(SparCraft
    PRIVATE
      "${SPARCRAFT_SRC_DIR}"
      "${SPARCRAFT_SRC_DIR}/imgui"
  )

  target_compile_definitions(SparCraft PRIVATE NOMINMAX)
  target_link_libraries(SparCraft
    PRIVATE
      SparCraftCore
      SFML::System
      SFML::Window
      SFML::Graphics
      SFML::Audio
      OpenGL::GL
  )
endif()
