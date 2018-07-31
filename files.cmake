
set(header_include
)

# grouping
source_group("include" FILES ${header_include})

set(module_headers
    ${header_include}
)

set(source_src
    "${PROJECT_SOURCE_DIR}/src/main.cpp"
    "${PROJECT_SOURCE_DIR}/src/main.hpp"
    "${PROJECT_SOURCE_DIR}/src/main_gui.cpp"
    "${PROJECT_SOURCE_DIR}/src/openvr_manager.cpp"
    "${PROJECT_SOURCE_DIR}/src/openvr_manager.hpp"
)

# grouping
source_group("src" FILES ${source_src})

set(module_sources
    ${source_src}
)
