cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

project(GeSimulation)

find_package(Geant4 REQUIRED ui_all vis_all)

include(${Geant4_USE_FILE})

file(GLOB sources ${PROJECT_SOURCE_DIR}/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/*.hh)

set(GeSimulation_maps   
    CrystalMaps/Ge/config.txt
    CrystalMaps/Ge/FT.ssv
    CrystalMaps/Ge/FTVec.ssv
    CrystalMaps/Ge/L.ssv
    CrystalMaps/Ge/LVec.ssv
    CrystalMaps/Ge/ST.ssv
    CrystalMaps/Ge/STVec.ssv
  )
file(GLOB MACRO_FILES "*.mac")


foreach(_map ${GeSimulation_maps})
    file(COPY ${_map} DESTINATION ${PROJECT_BINARY_DIR}/Ge/)
endforeach()
file(COPY ${MACRO_FILES} DESTINATION ${PROJECT_BINARY_DIR})

add_executable(sim sim.cc ${sources} ${headers})
target_link_libraries(sim ${Geant4_LIBRARIES})

add_custom_target(GeSimulation DEPENDS sim)
