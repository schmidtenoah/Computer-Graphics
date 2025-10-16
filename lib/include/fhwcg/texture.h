/**
 * @file
 * 
 * @brief
 * Modul für das Laden und Schreiben von Texturen.
 * 
 * @copyright 2020, FH Wedel
 * @author Nicolas Hollmann
 */

#ifndef FHWCG_TEXTURE_H
#define FHWCG_TEXTURE_H

#include <fhwcg/common.h>

//////////////////////////// ÖFFENTLICHE FUNKTIONEN ////////////////////////////

/**
 * Erzeugt eine OpenGL Textur aus einer Bilddatei.
 * 
 * @note
 * Im Fehlerfall wird immer eine korrekte Textur-ID zurückgegeben. Allerdings
 * fehlen unter Umständen die nötigen Bilddaten.
 * 
 * @param[in] filename der Pfad zur Bilddatei
 * @param[in] wrapping der Wrapping Modus (z.B. GL_REPEAT, GL_MIRRORED_REPEAT, 
 *        GL_CLAMP_TO_EDGE, GL_CLAMP_TO_BORDER)
 * @return eine OpenGL Textur ID
 */
GLuint texture_loadTexture(const char* filename, GLenum wrapping);

/**
 * Löscht eine zuvor angelegte Textur wieder.
 * 
 * @warning Die Textur-ID muss valide und noch nicht gelöscht sein.
 * 
 * @param[in,out] textureId die Textur-ID der Textur, die gelöscht werden soll. Die referenzierte Variable wird auf 0 gesetzt.
 */
void texture_deleteTexture(GLuint* textureId);

/**
 * Speichert einen Screenshot in dem Programmverzeichnis.  
 * Der Dateiname lautet screenshot_yyyy-MM-dd_hh-mm-ss.png wobei das aktuelle
 * Datum und die aktuelle Uhrzeit eingesetzt wird.
 * 
 * @note Es wird grundsätzlich der aktive Framebuffer ausgelesen.
 * 
 * @param[in] ctx der aktuelle Programmkontext
 */
void texture_saveScreenshot(ProgContext ctx);

#endif // FHWCG_TEXTURE_H
