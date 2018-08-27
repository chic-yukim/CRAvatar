set(header_include
    "${PROJECT_SOURCE_DIR}/include/${CRMODULE_ID}/module.h"
)

# grouping
source_group("${CRMODULE_ID}" FILES ${header_include})

set(module_headers
    ${header_include}
)


set(source_src
    "${PROJECT_SOURCE_DIR}/src/module.cpp"
)

# grouping
source_group("src" FILES ${source_src})

set(module_sources
    ${source_src}
)
