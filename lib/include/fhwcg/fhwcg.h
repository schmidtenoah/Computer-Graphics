/**
 * @file
 * 
 * @brief
 * Header, der die gesamte FHWCG Bibliothek einbindet.
 * 
 * @copyright 2025, FH Wedel
 * @authors Nicolas Hollmann, Philipp Munz
 */

#ifndef FHWCG_H
#define FHWCG_H

#include <fhwcg/common.h>
// #include <fhwcg/font.h> FONT MODUL DEAKTIVIERT
#include <fhwcg/shader.h>
#include <fhwcg/stb_ds.h>
#include <fhwcg/stb_image.h>
#include <fhwcg/texture.h>
#include <fhwcg/utils.h>
#include <fhwcg/window.h>
#include <fhwcg/scene.h>
#include <fhwcg/mesh.h>
#include <fhwcg/debug.h>
#include <fhwcg/nuklear.h>
#include <fhwcg/gui.h>
#include <fhwcg/allocator.h>
#include <fhwcg/camera.h>

#ifdef _DEBUG
    #include <setjmp.h>
    #include <cmocka.h>
#endif

#endif // FHWCG_H
