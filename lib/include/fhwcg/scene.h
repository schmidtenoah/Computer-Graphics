/**
 * @file
 * 
 * @brief
 * Modul für Operationen auf dem Szenengraphen.
 * 
 * @copyright 2024, FH Wedel
 * @author Philipp Munz
 */

#ifndef FHWCG_SCENE_H
#define FHWCG_SCENE_H

#include <fhwcg/common.h>

//////////////////////////// ÖFFENTLICHE FUNKTIONEN ////////////////////////////

/**
 * Fügt eine Matrix auf dem Stack hinzu
 * @note Zu Beginn liegt bereits eine Einheitsmatrix auf dem Stack
 */
void scene_pushMatrix(void);

/**
 * Fügt eine Matrix auf dem Stack hinzu
 * @note Zu Beginn liegt bereits eine Einheitsmatrix auf dem Stack
 * 
 * @param[in] m Matrix, welche auf den Stack gelegt wird
 */
void scene_pushMatrixM(mat4 m);

/**
 * Entfernt die oberste Matrix auf dem Stack
 */
void scene_popMatrix(void);

/**
 * Erzeugt eine Matrix zur Rotation die Szene.  
 * Die oberste Matrix im Stack wird dann mit dieser multipliziert.
 * 
 * @param[in] angle Winkel in Grad
 * @param[in] axis Achse, um die rotiert wird
 */
void scene_rotateV(float angle, vec3 axis);

/**
 * Variante von scene_rotateV() mit separaten Parametern
 * 
 * @param[in] angle Winkel in Grad
 * @param[in] axisX,axisY,axisZ Achse, um die rotiert wird
 */
void scene_rotate(float angle, float axisX, float axisY, float axisZ);

/**
 * Erzeugt eine Matrix zur Verschiebung der Szene.  
 * Die oberste Matrix im Stack wird dann mit dieser multipliziert.
 * 
 * @param[in] v Verschiebung in allen Achsen
 */
void scene_translateV(vec3 v);

/**
 * Variante von scene_translateV() mit separaten Parametern
 * 
 * @param[in] x,y,z Verschiebung in die jeweilige Achse
 */
void scene_translate(float x, float y, float z);

/**
 * Erzeugt eine Matrix zur Skalierung der Szene.  
 * Die oberste Matrix im Stack wird dann mit dieser multipliziert.
 * 
 * @param[in] v Skalierung in allen Achsen
 */
void scene_scaleV(vec3 v);

/**
 * Variante von scene_scaleV() mit separaten Parametern
 * 
 * @param[in] x,y,z Skalierung in der jeweiligen Achse
 */
void scene_scale(float x, float y, float z);

/**
 * Liefert die Vereinigung aller Matrizen (ModelView und Projection)
 * 
 * @param[out] res Hier wird die kombinierte Matrix abgelegt
 */
void scene_getMVP(mat4 res);

/**
 * Liefert die oberste ModelView Matrix vom Stack
 *
 * @param[out] res Hier wird die Matrix abgelegt
 */
void scene_getMV(mat4 res);

/**
 * Liefert die Projection Matrix
 *
 * @param[out] res Hier wird die Matrix abgelegt
 */
void scene_getP(mat4 res);

/**
 * Liefert die Normal Matrix
 * 
 * @param[out] res Hier wird die Matrix abgelegt
 */
void scene_getN(mat4 res);

/**
 * Erstellt eine orthographische Projektionsmatrix.
 * 
 * @see https://cglm.readthedocs.io/en/latest/cam.html#c.glm_ortho
 * 
 * @param[in] left,right Horizontale Ausdehnung des sichtbaren Bereichs
 * @param[in] bottom,top Vertikale Ausdehnung des sichtbaren Bereichs
 * @param[in] nearZ,farZ Minimale und maximale Distanz, in der Objekte angezeigt werden
 */
void scene_ortho(float left, float right, float bottom, float top, float nearZ, float farZ);

/**
 * Erstellt eine perspektivische Projektionsmatrix.
 * 
 * @see https://cglm.readthedocs.io/en/latest/cam.html#c.glm_perspective
 * 
 * @param[in] fovy Vertikales Sichtfeld in Grad
 * @param[in] aspect Seitenverhältnis von Breite zu Höhe
 * @param[in] nearZ,farZ Minimale und maximale Distanz, in der Objekte angezeigt werden
 */
void scene_perspective(float fovy, float aspect, float nearZ, float farZ);

/**
 * Erstellt eine View-Matrix um auf einen Punkt zu schauen.  
 * Die oberste Matrix im Stack wird dann mit dieser multipliziert.
 * 
 * @see https://cglm.readthedocs.io/en/latest/cam.html#c.glm_lookat
 * 
 * @param[in] eye Position des Betrachters
 * @param[in] center Position die betrachtet wird
 * @param[in] up Definiert, wo "oben" sein soll. Darf nicht parallel zur Blickrichtung sein
 */
void scene_lookAt(vec3 eye, vec3 center, vec3 up);

/**
 * Erstellt eine View-Matrix um in eine Richtung zu schauen.  
 * Die oberste Matrix im Stack wird dann mit dieser multipliziert.
 * 
 * @see https://cglm.readthedocs.io/en/latest/cam.html#c.glm_look
 * 
 * @param[in] eye Position des Betrachters
 * @param[in] dir Blickrichtung
 * @param[in] up Definiert, wo "oben" sein soll. Darf nicht parallel zur Blickrichtung sein
 */
void scene_look(vec3 eye, vec3 dir, vec3 up);

/**
 * Gibt den Stack von unten nach oben aus.  
 * Diese Funktion darf NUR zum Debuggen genutzt werden.  
 * Und selbst dafür ist sie nur begrenzt sinnvoll.
 * 
 * Nutzt https://cglm.readthedocs.io/en/latest/io.html#c.glm_mat4_print
 * 
 * @param[in] output Ort der Ausgabe (also z.B. stdout)
 */
void scene_printStack(FILE* const output);

#endif // FHWCG_SCENE_H
