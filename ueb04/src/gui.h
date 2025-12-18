/**
* @file gui.h
 * @brief GUI rendering
 *
 * @authors Nikolaos Tsetsas, Noah Schmidt
 */

#ifndef GUI_H
#define GUI_H

#include <fhwcg/fhwcg.h>

/**
 * Main GUI rendering function called each frame
 * @param ctx Program context
 */
void gui_renderContent(ProgContext ctx);

#endif // GUI_H