/**
 * @file
 * 
 * @brief
 * Modul für das Laden und Rendern von Texten/Schriftarten.
 * 
 * MODUL DEAKTIVIERT, INKOMPATIBEL MIT NUKLEAR GUI.
 * 
 * @copyright 2024, FH Wedel
 * @authors Nicolas Hollmann, Philipp Munz
 */

#ifndef FHWCG_FONT_H
#define FHWCG_FONT_H

#ifdef FHWCG_FONT_MODULE

#include <fhwcg/common.h>

//////////////////////////// ÖFFENTLICHE DATENTYPEN ////////////////////////////

struct Font;
/** Datenstruktur, die eine Schriftart repräsentiert */
typedef struct Font Font;

//////////////////////////// ÖFFENTLICHE FUNKTIONEN ////////////////////////////

/**
 * Erstellt ein Font aus einem Fontatlas.
 * 
 * @note Das Font benutzt Texture Unit 0.
 * 
 * @param[in] filename Pfad zum Fontatlas
 * @param[in] fontSize die Größe des Fonts in Pixeln
 * 
 * @return das geladene Font oder NULL wenn ein Fehler aufgetreten ist
 */
Font* font_loadFont(const char* filename, float fontSize);

/**
 * Erstellt ein Font aus einem default Fontatlas.
 * 
 * @note Das Font benutzt Texture Unit 0.
 * 
 * @param[in] fontSize die Größe des Fonts in Pixeln
 * 
 * @return das geladene Font oder NULL wenn ein Fehler aufgetreten ist
 */
Font* font_loadDefaultFont(float fontSize);

/**
 * Zeichnet Text auf den Bildschirm.
 * 
 * @param[in] font Das Font, welches genutzt wird
 * @param[in] ctx der Programmkontext
 * @param[in] x X-Position (in Pixeln) an der der Text beginnt
 * @param[in] y Y-Position (in Pixeln) an der der Text beginnt
 * @param[in] color Textfarbe
 * @param[in] text anzuzeigender Text
 */
void font_renderText(Font* font, ProgContext ctx, int x, int y, const vec3 color, const char* const text);

/**
 * Zeichnet formatierten Text auf den Bildschirm.
 * 
 * @warning Der Text ist auf 99 Zeichen beschränkt.
 * 
 * @param[in] font Das Font, welches genutzt wird
 * @param[in] ctx der Programmkontext
 * @param[in] x X-Position (in Pixeln) an der der Text beginnt
 * @param[in] y Y-Position (in Pixeln) an der der Text beginnt
 * @param[in] color Textfarbe
 * @param[in] text anzuzeigender Text mit Formatierung (wie z.B. prinft)
 */
void font_renderFormatText(Font* font, ProgContext ctx, int x, int y, const vec3 color, const char* const text, ...);

/**
 * Gibt ein Font wieder frei
 * 
 * @param[in] font Font, welches freigegeben wird
 */
void font_deleteFont(Font* font);

#endif // FHWCG_FONT_MODULE

#endif // FHWCG_FONT_H
