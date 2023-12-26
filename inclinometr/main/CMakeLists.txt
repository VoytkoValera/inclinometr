# See the build system documentation in IDF programming guide
# for more information about component CMakeLists.txt files.

idf_component_register(
    SRCS "src/main.c" "src/ws_server.c" "src/display.c" "src/display_ui.c" "src/display_helper.c"     # list the source files of this component
    INCLUDE_DIRS include    # optional, add here public include directories
    PRIV_INCLUDE_DIRS   # optional, add here private include directories
    REQUIRES            # optional, list the public requirements (component names)
    PRIV_REQUIRES       # optional, list the private requirements
)
