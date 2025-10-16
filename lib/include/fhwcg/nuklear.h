/**
 * @file
 * 
 * @brief
 * Diese Datei bindet eine
 * [Single-Header-Library](https://nicolashollmann.de/de/blog/single-header-libraries/) (SHL) ein
 * und setzt dabei alle nötigen Flags.
 * 
 * SHL: Nuklear
 * 
 * @warning DIE VERWENDUNG VON ZUSÄTZLICHEN BIBLIOTHEKEN MUSS IM VORAUS ABGESPROCHEN WERDEN.
 * 
 * @copyright 2020, FH Wedel
 * @author Nicolas Hollmann
 */

#ifndef FHWCG_NUKLEAR_H
#define FHWCG_NUKLEAR_H

#include <fhwcg/common.h>

// Nuklear Flags setzen, die überall gebraucht werden:
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_KEYSTATE_BASED_INPUT

// Hier wird Nuklear abschließend eingebunden:
#pragma warning( push )
#pragma warning( disable : 5287 )
#include <nuklear/nuklear.h>
#include <nuklear/nuklear_glfw_gl3.h>
#pragma warning( pop ) 

#endif // FHWCG_NUKLEAR_H
