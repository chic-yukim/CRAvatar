
set(header_include
)

# grouping
source_group("include" FILES ${header_include})

set(module_headers
    ${header_include}
)

set(source_src_main_gui
    "${PROJECT_SOURCE_DIR}/src/main_gui/main_gui.cpp"
    "${PROJECT_SOURCE_DIR}/src/main_gui/main_gui.hpp"
)

set(source_src_objects
    "${PROJECT_SOURCE_DIR}/src/objects/floor.cpp"
    "${PROJECT_SOURCE_DIR}/src/objects/floor.hpp"
)

set(source_src
    "${PROJECT_SOURCE_DIR}/src/ar_system.cpp"
    "${PROJECT_SOURCE_DIR}/src/ar_system.hpp"
    "${PROJECT_SOURCE_DIR}/src/main.cpp"
    "${PROJECT_SOURCE_DIR}/src/main.hpp"
    "${PROJECT_SOURCE_DIR}/src/openvr_manager.cpp"
    "${PROJECT_SOURCE_DIR}/src/openvr_manager.hpp"
)

# grouping
source_group("src\\main_gui" FILES ${source_src_main_gui})
source_group("src\\objects" FILES ${source_src_objects})
source_group("src" FILES ${source_src})

set(module_sources
    ${source_src_main_gui}
    ${source_src_objects}
    ${source_src}
)
