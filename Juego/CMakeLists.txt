cmake_minimum_required(VERSION 3.12)
project(MiJuegoSDL2)

# --- 1. Obtener SDL2 automáticamente ---
include(FetchContent)
FetchContent_Declare(
    SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-2.30.0 # Versión estable
)
FetchContent_MakeAvailable(SDL2)

# --- 2. Crear el ejecutable ---
add_executable(mi_juego src/main.c)

# --- 3. Enlazar con SDL2 ---
# SDL2::SDL2main y SDL2::SDL2 son los objetivos oficiales de CMake
target_link_libraries(mi_juego PRIVATE SDL2::SDL2main SDL2::SDL2)

# --- 4. (Opcional) Copiar assets al directorio de compilación ---
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/src/assets)
    add_custom_command(TARGET mi_juego POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/src/assets
        $<TARGET_FILE_DIR:mi_juego>/assets)
endif()
