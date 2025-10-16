/**
 * @file
 * 
 * @brief
 * Modul zum Laden und Verwenden von Shadern.
 * 
 * @copyright 2025, FH Wedel
 * @author Nicolas Hollmann, Philipp Munz
 */

#ifndef FHWCG_SHADER_H
#define FHWCG_SHADER_H

#include <fhwcg/common.h>

//////////////////////////// ÖFFENTLICHE DATENTYPEN ////////////////////////////

// Datenstruktur, die einen Shader repräsentiert.
struct Shader;

/** Datentyp, die einen Shader repräsentiert. */
typedef struct Shader Shader;

//////////////////////////// ÖFFENTLICHE FUNKTIONEN ////////////////////////////

/**
 * Erzeugt einen neuen, leeren Shader.
 * 
 * @return ein neuer Shader.
 */
Shader* shader_createShader(void);

/**
 * Hängt eine GLSL Datei an einen bestehenden Shader an.  
 * Der Code wird dabei auch sofort übersetzt und nur bei Erfolg an den
 * Shader gehängt.
 * 
 * @note Bei Misserfolg gibt die Funktion eine Fehlermeldung aus.
 * 
 * @param[in] shader der Shader an den die Datei angehängt werden soll.
 * @param[in] type der Shadertyp der Datei.
 * @param[in] file der Pfad zur Datei.
 * @return true, wenn die operation erfolgreich war, false wenn nicht.
 */
bool shader_attachShaderFile(Shader* shader, GLenum type, const char* file);

/**
 * Baut einen Shader zusammen (linken) nachdem mehrere Dateien an ihn
 * gehängt wurden.
 * 
 * @note Bei Misserfolg gibt die Funktion eine Fehlermeldung aus.
 * 
 * @param[in] name Name, der in RenderDoc angezeigt wird
 * @param[in,out] shader der Shader, der gebaut werden soll.
 * @return true, wenn die operation erfolgreich war, false wenn nicht.
 */
bool shader_buildShader(const char* const name, Shader* shader);

/**
 * Aktiviert einen Shader für die Benutzung.
 * 
 * @warning Der Shader muss bereits gebaut worden sein.
 * 
 * @param[in] shader der Shader, der genutzt werden soll.
 */
void shader_useShader(Shader* shader);

/**
 * Löscht einen bestehenden Shader und gibt alle Ressourcen wieder frei.  
 * Dabei ist es egal, ob der Shader bereits gebaut wurde oder nicht.  
 * Auch bereits angehängte Codes werden wieder freigegeben.
 * 
 * @param[in] shader der Shader, der gelöscht werden soll.
 */
void shader_deleteShader(Shader** shader);

/**
 * Hilfsfunktion zum Anlegen eines Shaders, der aus einem Vertex- und
 * einem Fragmentshader besteht.
 * 
 * @note Bei Misserfolg gibt die Funktion eine Fehlermeldung aus.
 * 
 * @param[in] name Name, der in RenderDoc angezeigt wird
 * @param[in] vert Pfad zur Datei, die den Vertex Shader enthält
 * @param[in] frag Pfad zur Datei, die den Fragment Shader enthält
 * 
 * @return ein Shader, der aus den übergebenen Dateien gebaut wurde oder NULL
 *         wenn etwas schief gegangen ist.
 */
Shader* shader_createVeFrShader(const char* const name, const char* vert, const char* frag);

/**
 * Erzeugt einen Shader zum Anzeigen von Normalen von 3D Objekten, die aus Dreiecken bestehen.
 * 
 * @note Bei Misserfolg gibt die Funktion eine Fehlermeldung aus.
 * 
 * @param[in] libShaderPath Pfad zu den Shadern der Bibliothek (wird von CMake erzeugt)
 * 
 * @return der erzeugte Shader oder NULL wenn etwas schief gegangen ist.
 */
Shader* shader_createNormalsShader(const char* const libShaderPath);

/**
 * Überprüft den Status eines Shader-Objektes nach dem Kompilieren.
 * 
 * @note Bei Misserfolg gibt die Funktion eine Fehlermeldung aus.
 * 
 * @param[in] shader Das Shader-Objekt
 * @param[in] file Datei, aus der der Shader Code geladen wurde
 * 
 * @return true wenn der Shader erfolgreich kompiliert wurde, sonst false
 */
bool shader_compileStatus(GLuint shader, const char* file);

/**
 * Überprüft den Status eines Shader-Programmes nach dem Linken.
 * 
 * @note Bei Misserfolg gibt die Funktion eine Fehlermeldung aus.
 * 
 * @param[in] program Das Shader-Programm
 * 
 * @return true wenn der Shader erfolgreich gelinked wurde, sonst false
 */
bool shader_linkStatus(GLuint program);

/**
 * Erzeugt die Deklaration und Dokumentation für einen Uniform Setter in verschiedenen Varianten:  
 *  1. Setter für einen einzigen Wert  
 *  2. Setter für mehrere Werte (Array)  
 *  3. Setter mit einem Format-String. So können einfach Elemente in einer Liste oder Struct-Felder gesetzt werden.  
 *     Beispiel: Array aus MyStructs und in einem Listenelement soll ein Feld überschrieben werden, also sowas wie "list[i].foo".
 * 
 */
#define CREATE_UNIFORM_SETTER(type, typeName) \
/**
 Übergibt einen typeName an einen Shader über eine Uniform-Variable.
 
 @warning Der Shader muss zuvor mit shader_useShader aktiviert worden sein!
 
 @param[in] shader der Shader, bei dem die Uniform Variable gesetzt werden soll
 @param[in] name der Name der Uniform Variable
 @param[in] value der zu setzende Wert
 */\
void shader_set##typeName(Shader* shader, char* name, type value); \
/**
 Übergibt mehrere typeName an einen Shader über eine Uniform-Variable.
 
 @warning Der Shader muss zuvor mit shader_useShader aktiviert worden sein!
 
 @param[in] shader der Shader, bei dem die Uniform Variable gesetzt werden soll
 @param[in] name der Name der Uniform Variable
 @param[in] values die Werte
 @param[in] count Anzahl an Werten
*/\
void shader_set##typeName##N(Shader* shader, char* name, type* values, GLsizei count); \
/**
 Übergibt einen typeName an einen Shader über eine Uniform Variable.  
 Mit dem Format-String können einfach Elemente in einer Liste oder Struct-Felder gesetzt werden.  
 Beispiel: Wir haben im Shader ein Array aus Structs und in einem Element soll ein Feld überschrieben
 werden, also so etwas wie *`list[i].foo`* könnte dann mit *`"list[%d].foo", i`* oder sogar 
 *`"list[%d].%s", i, "foo"`* übergeben werden.

 @warning Der Shader muss zuvor mit shader_useShader aktiviert worden sein!
 
 @param[in] shader der Shader, bei dem die Uniform Variable gesetzt werden soll
 @param[in] value der zu setzende Wert
 @param[in] fmt Format-String wie bei printf()
 @param[in] ... die weiteren Argumente für den Format-String in @p fmt
 */\
void shader_set##typeName##Fmt(Shader* shader, type value, char* fmt, ...);

/**
 * Setter für Vektoren und Matrizen
 * @see CREATE_UNIFORM_SETTER
 */
#define CREATE_UNIFORM_VECTOR_SETTER(type, typeName) \
/**
 Übergibt einen typeName an einen Shader über eine Uniform-Variable.
 
 @warning Der Shader muss zuvor mit shader_useShader aktiviert worden sein!
 
 @param[in] shader der Shader, bei dem die Uniform Variable gesetzt werden soll
 @param[in] name der Name der Uniform Variable
 @param[in] value der zu setzende Wert
 */\
void shader_set##typeName(Shader* shader, char* name, type* value); \
/**
 Übergibt mehrere typeName an einen Shader über eine Uniform-Variable.
 
 @warning Der Shader muss zuvor mit shader_useShader aktiviert worden sein!
 
 @param[in] shader der Shader, bei dem die Uniform Variable gesetzt werden soll
 @param[in] name der Name der Uniform Variable
 @param[in] values die Werte
 @param[in] count Anzahl an Werten
*/\
void shader_set##typeName##N(Shader* shader, char* name, type* values, GLsizei count); \
/**
 Übergibt einen typeName an einen Shader über eine Uniform Variable.  
 Mit dem Format-String können einfach Elemente in einer Liste oder Struct-Felder gesetzt werden.  
 Beispiel: Wir haben im Shader ein Array aus Structs und in einem Element soll ein Feld überschrieben
 werden, also so etwas wie *`list[i].foo`* könnte dann mit *`"list[%d].foo", i`* oder sogar 
 *`"list[%d].%s", i, "foo"`* übergeben werden.

 @warning Der Shader muss zuvor mit shader_useShader aktiviert worden sein!
 
 @param[in] shader der Shader, bei dem die Uniform Variable gesetzt werden soll
 @param[in] value der zu setzende Wert
 @param[in] fmt Format-String wie bei printf()
 @param[in] ... die weiteren Argumente für den Format-String in @p fmt
 */\
void shader_set##typeName##Fmt(Shader* shader, type* value, char* fmt, ...);

CREATE_UNIFORM_SETTER(GLint, Int)
CREATE_UNIFORM_SETTER(GLuint, Uint)
CREATE_UNIFORM_SETTER(GLfloat, Float)
CREATE_UNIFORM_SETTER(GLuint, Bool) // Die Bools sind etwas speziell, da sie in GLSL als uint behandelt werden, in C bool und uint aber unterschiedlich groß sind
CREATE_UNIFORM_VECTOR_SETTER(vec2, Vec2)
CREATE_UNIFORM_VECTOR_SETTER(vec3, Vec3)
CREATE_UNIFORM_VECTOR_SETTER(vec4, Vec4)
CREATE_UNIFORM_VECTOR_SETTER(mat3, Mat3)
CREATE_UNIFORM_VECTOR_SETTER(mat4, Mat4)

#undef CREATE_UNIFORM_SETTER
#undef CREATE_UNIFORM_VECTOR_SETTER

#endif // FHWCG_SHADER_H
