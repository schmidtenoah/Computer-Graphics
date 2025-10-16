###############################################################################
# CG CMake Konfiguration
#
# Copyright (C) 2025, FH Wedel
# Autor: Nicolas Hollmann, Philipp Munz
###############################################################################

############################# Absicherungen ###################################

# In-source builds deaktivieren
if(" ${CMAKE_SOURCE_DIR}" STREQUAL " ${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "
FATAL: In-source builds are not allowed.
       Please create a separate directory for build files.
")
endif()

# Release voreinstellen, wenn keine Auswahl getroffen wurde
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message("Setting build type to 'Release' as none was specified.")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

############################## Einstellungen ##################################

# Wir verwenden C99 für alle Übungen
set(C_STANDARD 99)

# Das ZERO_CHECK Target deaktivieren
set(CMAKE_SUPPRESS_REGENERATION true)

# Das aktuelle Projekt in Visual Studio als Startprojekt festlegen
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

# Aktiviert die Verwendung von Ordnern in IDEs, die dies unterstützen.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Pfad zu den Bibliotheken
set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../lib)

################################# OpenGL ######################################

# OpenGL muss auf dem System vorhanden sein
find_package(OpenGL REQUIRED)

# Alle Quellcode-Dateien und Header im src-Verzeichnis finden:
file(GLOB_RECURSE src_files CONFIGURE_DEPENDS
    "src/*.h"
    "src/*.c"
)

# Alle Dateien im Shader-Verzeichnis werden auch zum Projekt hinzugefügt.
# Nur dadurch ist es möglich, dass die Shader direkt in Visual Studio
# aufgelistet werden.
file(GLOB_RECURSE shaders
    "res/shader/*"
)

# Projekt anlegen
add_executable(${PROJECT_NAME} ${src_files} ${shaders})

# Include Verzeichnis zum Projekt hinzufügen
target_include_directories(${PROJECT_NAME} PUBLIC ${OPENGL_INCLUDE_DIR} ${LIB_DIR}/include)

# Bibliotheken zum Projekt hinzufügen
target_link_libraries(${PROJECT_NAME}
    ${CMAKE_DL_LIBS}
    ${OPENGL_gl_LIBRARY}
    $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:${LIB_DIR}/bin/fhwcg64d.lib>
    $<$<CONFIG:Release>:${LIB_DIR}/bin/fhwcg64.lib>
    ${LIB_DIR}/bin/glfw3.lib
    $<$<CONFIG:Debug>:${LIB_DIR}/bin/cmocka.lib>
)

if(UNIX AND NOT APPLE)
    # Unter Linux muss die Mathebibliothek extra gelinkt werden, wenn Funktionen
    # aus math.h genutzt werden sollen.
    target_link_libraries(${PROJECT_NAME} m)
endif()

# Den Namen des Programms festlegen
target_compile_definitions(${PROJECT_NAME} PRIVATE PROGRAM_NAME="${PROJECT_NAME} $<CONFIG>")

# Bestimmen, ob 64- oder 32-Bit-System
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    # 64 bits
    set(sys_arch_bits "64bit")
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    # 32 bits
    set(sys_arch_bits "32bit")
else()
    # Unkown
    set(sys_arch_bits "??bit")
endif()
target_compile_definitions(${PROJECT_NAME} PRIVATE ARCH_BITS="${sys_arch_bits}")

########################### Visual Studio Filter ##############################

# Die folgende Schleife liest für alle Shader die Pfade aus und sortiert sie in
# einem Visual Studio Filter. Dadurch wieder das Projekt dort aufgeräumter.
foreach(_shader IN ITEMS ${shaders})
    get_filename_component(_source_path "${_shader}" PATH)
    string(REPLACE "${CMAKE_SOURCE_DIR}/res/shader" "" _group_path "${_source_path}")
    string(REPLACE "/" "\\" _group_path "${_group_path}")
    source_group("Shader\\${_group_path}" FILES "${_shader}")
endforeach()

############################## Compilerflags ##################################

if(MSVC)
    # Flags unter Windows mit dem MSVC:
    # /W4: Höchstes Warnungslevel einstellen
    # /WX: Alle Warnungen als Fehler betrachten, aber:
    # /wd4996: Warnung 4996 (veraltete Systemfunktionen) deaktivieren
    # /wd4204: Warnung 4204 (nicht-konstante struct Initialisierung) deaktivieren
    # /wd4127: Warnung 4127 (konstanter Vergleich) deaktivieren
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX /wd4996 /wd4204 /wd4127)
else()
    # Flags bei allen anderen Compilern:
    # -Wall: (Fast) alle Warnungen aktivieren
    # -Wno-long-long: Warnung bezüglich der Verwendung von long-long deaktivieren
    # -Werror: Alle Warnungen als Fehler behandeln
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wno-long-long -Werror)

    if(APPLE)
        # Unter macOS gilt OpenGL als veraltet. Deshalb werden vom Compiler Warnungen erzeugt,
        # die durch -Werror zu Fehlermeldungen werden. Dies verhindert, dass das Projekt
        # übersetzt werden kann. Aus dem Grund ist es erforderlich, die Warnung für
        # veralteten Code zu deaktivieren.
        target_compile_options(${PROJECT_NAME} PRIVATE -Wno-deprecated)
    endif()
endif()

############################### Shaderpfade ###################################

# Eigene Shader
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/res")
    if(WIN32)
        # Auf Windows können Symlinks nur als Administrator oder im Entwicklermodus erstellt werden.
        # Deshalb berechnen wir den relativen Pfad von dem Build-Verzeichnis zum Ressourcen-Verzeichnis.
        # Dazu berechnen wir ersteinmal den absoluten Pfad zu den Ressourcen:
        get_filename_component(RESOURCE_ABS_PATH "./res" REALPATH BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
        get_filename_component(COMMON_RESOURCE_ABS_PATH "../common" REALPATH BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

        # Als nächstes bestimmen wir dann den relativen Pfad von dem Build-Verzeichnis zu dem vorherigen
        # absoluten Pfad und setzen ihn als Konstante in unserem Programm:
        file(RELATIVE_PATH RESOURCE_REL_PATH "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>" "${RESOURCE_ABS_PATH}")
        target_compile_definitions(${PROJECT_NAME} PRIVATE RESOURCE_PATH="${RESOURCE_REL_PATH}/")

        file(RELATIVE_PATH COMMON_RESOURCE_REL_PATH "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>" "${COMMON_RESOURCE_ABS_PATH}")
        target_compile_definitions(${PROJECT_NAME} PRIVATE COMMON_RESOURCE_PATH="${COMMON_RESOURCE_REL_PATH}/")
    else()
        # Unter Linux und macOS kann stattdessen einfach ein Symlink verwendet werden:
        add_custom_target(
            res_link ALL 
            COMMAND ${CMAKE_COMMAND} -E create_symlink
                "${CMAKE_CURRENT_SOURCE_DIR}/res" "${CMAKE_CURRENT_BINARY_DIR}/res")
    endif()
else()
    message(STATUS "CG: No resource directory in use.")
endif()

# FHWCG Shader
if(WIN32)
    # Wir bestimmen den relativen Pfad von dem Build-Verzeichnis zu dem absoluten Pfad 
    # und setzen ihn als Konstante in unserem Programm:
    file(RELATIVE_PATH SHADER_REL_PATH "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>" "${LIB_DIR}/shader")
    target_compile_definitions(${PROJECT_NAME} PRIVATE FHWCG_SHADER_PATH="${SHADER_REL_PATH}/")

else()
    # Unter Linux und macOS kann stattdessen einfach ein Symlink verwendet werden:
    add_custom_target(
        shader_link ALL 
        COMMAND ${CMAKE_COMMAND} -E create_symlink
            "${CMAKE_CURRENT_SOURCE_DIR}/res" "${CMAKE_CURRENT_BINARY_DIR}/res")
endif()

########################## Help Server Einstellungen ##########################

# Standardmäßig wollen wir gar nichts verändern, der Wert kann aber überschrieben werden
set(HELP_SERVER_FLAGS WINDOW_FLAGS_NONE CACHE STRING "Window Flags, die vom Help Server genutzt werden")

# Flags als Define ans Programm geben
add_compile_definitions(HELP_SERVER_FLAGS=${HELP_SERVER_FLAGS})

###############################################################################
################################## EOF ########################################
###############################################################################
