/**
 * @file
 * 
 * @brief
 * Wichtige Datentypen, Funktionen und Includes die
 * im gesamten Programm gebraucht werden.
 * 
 * @copyright 2025, FH Wedel
 * @author Nicolas Hollmann, Philipp Munz
 */

#ifndef FHWCG_COMMON_H
#define FHWCG_COMMON_H

// Windows Warnungen deaktivieren
#define _CRT_SECURE_NO_WARNINGS

#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
#endif
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include <stdbool.h>
#include <stdlib.h>

//////////////////////////// ÖFFENTLICHE DATENTYPEN ////////////////////////////

/** Programmkontext-Datentyp */ 
struct ProgContext;

/** Hier werden alle persistenten Informationen gespeichert */
typedef struct ProgContext* ProgContext;

#ifndef uint
/** Deutlich kürzer als `unsigned int` */
typedef unsigned int uint;
#endif

#endif // FHWCG_COMMON_H
