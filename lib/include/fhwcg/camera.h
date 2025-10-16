/**
 * @file
 * 
 * @brief
 * Modul für die Steuerung einer frei beweglichen "First-Person" Kamera.
 * 
 * @copyright 2025, FH Wedel
 * @authors Nicolas Hollmann, Philipp Munz
 */

#ifndef FHWCG_CAMERA_H
#define FHWCG_CAMERA_H

#include <fhwcg/common.h>

//////////////////////////// ÖFFENTLICHE DATENTYPEN ////////////////////////////

// Datenstruktur für die Repräsentation einer 3D Kamera.
struct Camera;

/** Datentyp für die Repräsentation einer 3D Kamera. */
typedef struct Camera Camera;

//////////////////////////// ÖFFENTLICHE FUNKTIONEN ////////////////////////////

/**
 * Erzeugt eine neue Kamera.
 * 
 * @param[in] ctx der Programmkontext.
 * @param[in] position Startposition der Kamera
 * @param[in] speed normale Geschwindigkeit der Kamera
 * @param[in] fastSpeed Geschwindigkeit beim Boosten (Shift)
 * @param[in] sensitivity Mausempfindlichkeit
 * @param[in] yaw Anfangsrotation in Grad
 * @param[in] pitch Anfangsneigung in Grad
 * 
 * @return die neu erzeugte Kamera.
 */
Camera* camera_createCamera(ProgContext ctx, vec3 position, float speed, float fastSpeed, float sensitivity, float yaw, float pitch);

/**
 * Erzeugt eine Kamera mit Standardwerten.
 * 
 * Die Kamera startet dabei an Position {0, 0, 1} und schaut in Richtung Ursprung
 * 
 * @param[in] ctx der Programmkontext.
 * @return die neu erzeugte Kamera.
 */
Camera* camera_createDefaultCamera(ProgContext ctx);

/**
 * Callback bei Mausbewegung
 * 
 * @param[in,out] camera Kamera, die gesteuert wird
 * @param[in] ctx der Programmkontext.
 * @param[in] x,y neue Position der Maus
 */
void camera_mouseMoveCallback(Camera* camera, ProgContext ctx, float x, float y);

/**
 * Callback bei Maustaste
 * 
 * @param[in, out] camera Kamera, die gesteuert wird
 * @param[in] button Taste, die das Event ausgelöst hat
 * @param[in] action Art des Events
 */
void camera_mouseButtonCallback(Camera* camera, int button, int action);

/**
 * Callback bei Tastatur
 * 
 * @param[in,out] camera Kamera, die gesteuert wird
 * @param[in] key Taste, die das Event ausgelöst hat
 * @param[in] action Art des Events
 */
void camera_keyboardCallback(Camera* camera, int key, int action);

/**
 * Gibt die Position der Kamera zurück.
 * 
 * @param[in] camera Kamera, deren Position ausgelesen werden soll.
 * @param[out] position Vektor, in den das Ergebnis geschrieben werden soll.
 */
void camera_getPosition(Camera* camera, vec3 position);

/**
 * Gibt die Blickrichtung der Kamera zurück.
 * 
 * @param[in] camera Kamera, deren Blickrichtung ausgelesen wird
 * @param[out] front Vektor, in den das Ergebnis geschrieben werden soll.
 */
void camera_getFront(Camera* camera, vec3 front);

/**
 * Löscht eine Kamera.
 * 
 * @param[in,out] camera die zu löschende Kamera. Die referenzierte Variable wird auf NULL gesetzt.
 */
void camera_deleteCamera(Camera** camera);

/**
 * Berechnet die Position und Blickrichtung ausgehend von den Input Callbacks neu.
 * 
 * @param[in,out] camera Kamera, die verändert wird
 * @param[in] deltaTime Zeit seit dem letzten Aufruf
 * 
 * @return true, wenn die Kamera diesen Frame bewegt wurde. Sonst false.
 */
bool camera_updateCamera(Camera* camera, float deltaTime);

/**
 * Berechnet die Position und Blickrichtung ausgehend von den Input Callbacks neu.
 * 
 * @param[in,out] camera Kamera, die verändert wird
 * @param[in] deltaTime Zeit seit dem letzten Aufruf
 * @param[in] limits Bewegungslimits der Kamera pro Achse. Je Achse erst Minimum dann Maximum.
 * 
 * @return true, wenn die Kamera diesen Frame bewegt wurde. Sonst false.
 */
bool camera_updateCameraLimited(Camera* camera, float deltaTime, vec2 limits[3]);

/**
 * Legt die von der Kamera erzeugte View Matrix auf den Stack
 * 
 * @see scene_look()
 * 
 * @param[in] camera Kamera, deren View Matrix genutzt werden soll
 */
void camera_setMatrix(Camera* camera);

#endif // FHWCG_CAMERA_H
