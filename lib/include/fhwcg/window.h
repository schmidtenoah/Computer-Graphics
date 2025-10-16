/**
 * @file
 * 
 * @brief
 * Modul zum Erstellen und Verwalten des Hauptfensters.
 * 
 * @copyright 2025, FH Wedel
 * @author Nicolas Hollmann, Philipp Munz
 */

#ifndef FHWCG_WINDOW_H
#define FHWCG_WINDOW_H

#include <fhwcg/common.h>

//////////////////////////// ÖFFENTLICHE DATENTYPEN ////////////////////////////

/** Callback bei Änderung der Framebuffergröße */
typedef void (*Fhwcg_Cb_FramebufferSize)(ProgContext ctx, int width, int height);

/** Callback bei Tastatureingabe */
typedef void (*Fhwcg_Cb_Keyboard)(ProgContext ctx, int key, int action, int mods);

/** Callback bei Mausbewegung */
typedef void (*Fhwcg_Cb_MouseMovement)(ProgContext ctx, double x, double y);

/** Callback bei Mausklick */
typedef void (*Fhwcg_Cb_MouseButton)(ProgContext ctx, int button, int action, int mods);

/** Callback bei Scrollen */
typedef void (*Fhwcg_Cb_MouseScroll)(ProgContext ctx, double x, double y);

/** Callback bei Texteingabe */
typedef void (*Fhwcg_Cb_TextInput)(ProgContext ctx, unsigned int codepoint);

/** Callback bei Reinziehen einer Datei */
typedef void (*Fhwcg_Cb_Drop)(ProgContext ctx, int count, const char** paths);

/** Aufruf der Shader neu-laden Funktion */
typedef void (*Fhwcg_shaderReload)();

/**
 * Flags für das Erzeugen eines Fensters
 */
enum FhwcgWindowFlags
{
    /** Es wird ein Standardfenster erzeugt (default) */
    WINDOW_FLAGS_NONE =          0b00000000,

    /** Das Fenster soll Vertikale Synchronisation nutzen */
    WINDOW_FLAGS_VSYNC =         0b00000001,

    /** Das Fenster soll maximiert erzeugt werde (unabhängig von der eingestellten Auflösung) */
    WINDOW_FLAGS_MAXIMIZED =     0b00000010,

    /** Es wird kein sichtbares Fenster erzeugt (Für euch nicht relevant, wird vom Help Server genutzt) */
    WINDOW_FLAGS_HEADLESS =      0b00000100,

    /**
     * Versteckt den Mauszeiger  
     * @see window_setMouseCapture()
     */
    WINDOW_FLAGS_MOUSE_CAPTURE = 0b00001000,

    /** Das Fenster wird direkt im Vollbild-Modus angezeigt */
    WINDOW_FLAGS_FULLSCREEN =    0b00010000,
};

/**
 * @see FhwcgWindowFlags
 */
typedef uint8_t WindowFlags;

//////////////////////////// ÖFFENTLICHE FUNKTIONEN ////////////////////////////

/**
 * Erzeugt ein neues Fenster und initialisiert das gesamte Rendering-System.
 * 
 * @param[in] title der Titel des Fensters.
 * @param[in] width die Breite des Fensters.
 * @param[in] height die Höhe des Fensters.
 * @param[in] samples Anzahl an MSAA samples. 0/1 samples schaltet MSAA aus. Ansonsten werden 2,4,8,16 unterstützt (so lange es die ausführende Hardware auch unterstützt)
 * @param[in] flags Bitfeld mit Fenstereinstellungen
 * 
 * @return ein neuer Programmkontext, der zu dem Fenster gehört.
 */
ProgContext window_init(const char* title, int width, int height, int samples, WindowFlags flags);

/**
 * Erzeugt ein neues Fenster und initialisiert das gesamte Rendering-System.
 * Diese Funktion erlaubt zusätzlich zu den Einstellungen von window_init
 * auch die Auswahl einer OpenGL Version.
 * 
 * @param[in] title der Titel des Fensters.
 * @param[in] width die Breite des Fensters.
 * @param[in] height die Höhe des Fensters.
 * @param[in] glMajor die zu verwendende OpenGL Major Version.
 * @param[in] glMinor die zu verwendende OpenGL Minor Version.
 * @param[in] samples Anzahl an MSAA samples. 0/1 samples schaltet MSAA aus. Ansonsten werden 2,4,8,16 unterstützt (so lange es die ausführende Hardware auch unterstützt)
 * @param[in] flags Bitfeld mit Fenstereinstellungen
 * 
 * @return ein neuer Programmkontext, der zu dem Fenster gehört.
 */
ProgContext window_initEx(const char* title, int width, int height, int glMajor, int glMinor, int samples, WindowFlags flags);

/**
 * Startet einen neuen Frame. Hier werden auch notwendige Fenstersignale empfangen und
 * behandelt.
 * 
 * @param[in] ctx der Programmkontext.
 * @return true, wenn das Programm fortgesetzt werden und ein neue Frame angezeigt werden soll.
 *         false, wenn die Hauptschleife unterbrochen und das Programm beendet werden soll.
 */
bool window_startNewFrame(ProgContext ctx);

/**
 * Beendet einen Frame und tauscht Back- und Frontbuffer, damit der
 * aktuelle Frame angezeigt werden kann. Außerdem werden auch einige
 * Zeitmessungen für die Delta-Time und FPS durchgeführt.
 * 
 * @param[in] ctx der Programmkontext.
 */
void window_swapBuffers(ProgContext ctx);

/**
 * Signalisiert dem Fenster, dass es geschlossen werden soll.
 * Es wird dabei nicht sofort geschlossen. Stattdessen wird
 * nach Aufruf dieser Funktion window_startNewFrame immer false
 * zurückgeben, damit die Hauptschleife unterbrochen werden kann.
 * 
 * Das Fenster wird tatsächlich erst mit einem Aufruf von window_cleanup
 * geschlossen und freigegeben.
 * 
 * @param[in] ctx der Programmkontext.
 */
void window_shouldCloseWindow(ProgContext ctx);

/**
 * Setzt ein Callback für Änderungen der Fenstergröße.
 * 
 * @param[in] ctx der Programmkontext.
 * @param[in] callback das neue Callback.
 */
void window_setFramebufferSizeCallback(ProgContext ctx, Fhwcg_Cb_FramebufferSize callback);

/**
 * Setzt ein Callback für Tastatureingaben.
 * 
 * @param[in] ctx der Programmkontext.
 * @param[in] callback das neue Callback.
 */
void window_setKeyboardCallback(ProgContext ctx, Fhwcg_Cb_Keyboard callback);

/**
 * Setzt ein Callback für Mausbewegungen.
 * 
 * @param[in] ctx der Programmkontext.
 * @param[in] callback das neue Callback.
 */
void window_setMouseMovementCallback(ProgContext ctx, Fhwcg_Cb_MouseMovement callback);

/**
 * Setzt ein Callback für Mausklicks.
 * 
 * @param[in] ctx der Programmkontext.
 * @param[in] callback das neue Callback.
 */
void window_setMouseButtonCallback(ProgContext ctx, Fhwcg_Cb_MouseButton callback);

/**
 * Setzt ein Callback für das Maus-Scrollen.
 * 
 * @param[in] ctx der Programmkontext.
 * @param[in] callback das neue Callback.
 */
void window_setMouseScrollCallback(ProgContext ctx, Fhwcg_Cb_MouseScroll callback);

/**
 * Setzt ein Callback für Text Input.
 * 
 * @param[in] ctx der Programmkontext.
 * @param[in] callback das neue Callback.
 */
void window_setTextCallback(ProgContext ctx, Fhwcg_Cb_TextInput callback);

/**
 * Setzt ein Callback für das "Reinziehen" von Dateien.
 * 
 * @param[in] ctx der Programmkontext.
 * @param[in] callback das neue Callback.
 */
void window_setDropCallback(ProgContext ctx, Fhwcg_Cb_Drop callback);

/**
 * Setzt die Fullscreen-Einstellung.
 * 
 * @param[in] ctx der Programmkontext.
 * @param[in] fullscreen true für Fullscreen, false für Windowed
 */
void window_setFullscreen(ProgContext ctx, bool fullscreen);

/**
 * Fragt den aktuellen Fullscreen-Zustand ab.
 * 
 * @param[in] ctx der Programmkontext.
 * @return true für Fullscreen, false für Windowed 
 */
bool window_isFullscreen(ProgContext ctx);

/**
 * Gibt die Größe des Framebuffers zurück.
 * 
 * Es ist erlaubt, dass einer der beiden Zeiger NULL ist.
 * 
 * @param[in] ctx der Programmkontext.
 * @param[out] width ein Zeiger auf einen Int, in dem die Breite des Fensters geschrieben wird
 * @param[out] height ein Zeiger auf einen Int, in dem die Höhe des Fensters geschrieben wird
 */
void window_getFramebufferSize(ProgContext ctx, int* width, int* height);

/**
 * Gibt die echte Größe des Fensters zurück.
 * Bei HiDPI bzw. Retina-Displays kann diese Größe von der, der Framebuffer
 * Size abweichen. Für die meisten OpenGL Funktionen sollte lieber die
 * Framebuffer-Size verwendet werden! Für 2D-UI kann die echte Größe besser
 * geeignet sein.
 * 
 * Es ist erlaubt, dass einer der beiden Zeiger NULL ist.
 * 
 * @param[in] ctx der Programmkontext.
 * @param[out] width ein Zeiger auf einen Int, in dem die Breite des Fensters geschrieben wird
 * @param[out] height ein Zeiger auf einen Int, in dem die Höhe des Fensters geschrieben wird
 */
void window_getRealSize(ProgContext ctx, int* width, int* height);

/**
 * Gibt die aktuellen FPS zurück.
 * Die FPS werden einmal pro Sekunde neu bestimmt.
 * 
 * @param[in] ctx der Programmkontext.
 * @return FPS
 */
int window_getFps(ProgContext ctx);

/**
 * Gibt die vergangene Zeit in Sekunden seit dem letzten Frame zurück.
 * 
 * @param[in] ctx der Programmkontext.
 * @return Sekunden seit dem letzten Frame. 
 */
double window_getDeltaTime(ProgContext ctx); 

/**
 * Gibt das interne GLFW Fenster zurück.
 * 
 * @param[in] ctx der Programmkontext.
 * @return das GLFW Fensterhandle
 */
GLFWwindow* window_getGlfwWindow(ProgContext ctx);

/**
 * Gibt alle Ressourcen, die durch das Fenster und dem dahinter
 * liegendem System belegt wurden, wieder frei.
 * 
 * @param[in] ctx der Programmkontext.
 */
void window_cleanup(ProgContext ctx);

/**
 * Setzt den Status des Mouse Captures.  
 * Wenn aktiv, wird der Mauszeiger nicht mehr angezeigt und kann sich auch unendlich weit bewegen.  
 * Bildschirmränder etc werden also ignoriert und die Maus kann auch nicht mehr mit anderen Fenstern interagieren.  
 * 
 * Dieser Modus ist zum Beispiel für First Person Kameras sinnvoll.
 * 
 * @param[in] ctx der Programmkontext.
 * @param[in] status der neue Zustand
 */
void window_setMouseCapture(ProgContext ctx, bool status);

/**
 * Gibt den Zustand des Mouse Captures
 * 
 * @see window_setMouseCapture()
 * 
 * @param[in] ctx der Programmkontext.
 * @return Zustand des Mouse Captures
 */
bool window_isMouseCaptured(ProgContext ctx);

#ifdef _DEBUG

/**
 * Speichert die Funktion für das neu laden der Shader
 * 
 * @param[in] ctx der Programmkontext.
 * @param[in] shaderReload Funktion, mit der alle Shader neu geladen werden
 */
void window_registerServerFuncs(ProgContext ctx, Fhwcg_shaderReload shaderReload);
#endif

#endif // FHWCG_WINDOW_H
