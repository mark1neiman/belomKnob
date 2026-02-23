file(GLOB_RECURSE SOURCES_C *.c)
file(GLOB_RECURSE SOURCES_CPP *.cpp)
list(FILTER SOURCES_C EXCLUDE REGEX "[\\\\/]ui_assets[\\\\/]belom_logo_black\\.c$")
list(FILTER SOURCES_C EXCLUDE REGEX "[\\\\/]lv_font_montserrat_200_num\\.c$")
list(FIND SOURCES_C "${CMAKE_CURRENT_SOURCE_DIR}/blur.c" BLUR_SRC_INDEX)
if(BLUR_SRC_INDEX EQUAL -1)
    list(APPEND SOURCES_C "${CMAKE_CURRENT_SOURCE_DIR}/blur.c")
endif()

idf_component_register(
    SRCS
        ${SOURCES_C}
        ${SOURCES_CPP}
    INCLUDE_DIRS
        "."
)
