/**
 * @file gui.h
 * @brief GUI rendering for help overlay, settings menu, and game controls.
 *
 * Provides the main GUI rendering function that displays all UI elements
 * including help text, settings menu, and start button.
 *
 * @authors Nikolaos Tsetsas, Noah Schmidt
 */

#ifndef GUI_H
#define GUI_H

#include <fhwcg/fhwcg.h>

/**
 * Entry point for GUI rendering called each frame.
 * Renders all GUI elements (help overlay, settings menu, start button).
 *
 * @param ctx Program context
 */
void gui_renderContent(ProgContext ctx);

#endif // GUI_H