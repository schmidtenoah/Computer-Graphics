/**
 * @file
 * 
 * @brief
 * Modul für mehr Lesbarkeit in RenderDoc
 * 
 * @copyright 2024, FH Wedel
 * @authors Nicolas Hollmann, Philipp Munz
 */

#ifndef FHWCG_DEBUG_H
#define FHWCG_DEBUG_H

#include <fhwcg/common.h>

/**
 * Diese Funktion gibt einem OpenGL-Objekt ein Label, damit
 * es in RenderDoc leichter aufgefunden werden kann.
 * 
 * @warning Das Label darf nicht länger als 244 Zeichen sein.
 * 
 * @param[in] type der OpenGL Typ (GL_SHADER, GL_TEXTURE, ...)
 * @param[in] name die ID der OpenGL Objektes
 * @param[in] label das Label
 */
void debug_labelObjectByType(GLenum type, GLuint name, const char* label);

/**
 * Diese Funktion gibt einem OpenGL-Objekt ein Label, damit
 * es in RenderDoc leichter aufgefunden werden kann. Aus dem
 * Dateipfad wird dabei nur der Name extrahiert.
 * 
 * @param[in] type der OpenGL Typ (GL_SHADER, GL_TEXTURE, ...)
 * @param[in] name die ID der OpenGL Objektes
 * @param[in] filepath der Quell-Dateiname für dieses OpenGL-Objekt
 */
void debug_labelObjectByFilename(GLenum type, GLuint name, const char* filepath);

/**
 * Diese Funktion markiert einen Render-Bereich mit einem Namen,
 * damit dieser in RenderDoc hervorgehoben wird.
 * 
 * @warning Nach einem Push muss irgendwann innerhalb eines Frames zwingend ein Pop
 * aufgerufen werden.
 * 
 * @param[in] scope der Name des Scopes
 * @param[in] source die Quelle für diesen Scope (Anwendung oder 3rd Party)
 */
void debug_pushRenderScopeSource(const char* scope, GLenum source);

/**
 * Kurzschreibweise für einen neuen Scope innerhalb der Anwendung.
 */
#define debug_pushRenderScope(scope) debug_pushRenderScopeSource((scope), GL_DEBUG_SOURCE_APPLICATION)

/**
 * Diese Funktion markiert das Ende eines Render-Bereichs.
 * 
 * @warning Zuvor muss debug_pushRenderScopeSource oder debug_pushRenderScope
 * aufgerufen worden sein.
 */
void debug_popRenderScope(void);

#endif
