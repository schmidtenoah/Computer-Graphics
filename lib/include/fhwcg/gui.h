/**
 * @file
 * 
 * @brief
 * Modul zum Erstellen und Rendern eines GUI.
 * 
 * @copyright 2025, FH Wedel
 * @authors Nicolas Hollmann, Philipp Munz
 */

#ifndef FHWCG_GUI_H
#define FHWCG_GUI_H

#include <fhwcg/common.h>

//////////////////////////// ÖFFENTLICHE DATENTYPEN ////////////////////////////

/** Zeichenfunktion eines GUI */
typedef void (*Fhwcg_Gui_Func)(ProgContext ctx);

/** Zeichenfunktion eines RenderImage */
typedef void (*Fhwcg_Gui_RenderImageFunc)(ProgContext ctx, int width, int height, void* data);

/** Zeile einer Auflistung der Tastenbelegung */
typedef char* GuiHelpLine[2];

struct renderWindow;

/** Datentyp eines RenderWindow */
typedef struct renderWindow* GuiRenderWindow;

//////////////////////////// ÖFFENTLICHE FUNKTIONEN ////////////////////////////

/**
 * Initialisiert das GUI.
 * 
 * @warning Der Programmkontext muss ein Handle auf ein existierendes GLFW Fenster enthalten.
 * 
 * @param[in] ctx Programmkontext.
 */
void gui_init(ProgContext ctx);

/**
 * Rendert das GUI.  
 * Muss als letztes vor dem Double-Buffer Swap aufgerufen werden.
 * 
 * @warning Nuklear modifiziert zum Rendern einige OpenGL Zustände. Stellt bei Problemen sicher,
 * dass ihr beim Rendern sämtliche relevanten Zustände korrekt setzt.
 * 
 * @note Nuklear nutzt zum Rendern Texture Unit 0
 * 
 * @param[in] ctx Programmkontext.
 * @param[in] func Funktion, die den Inhalt des GUI rendert
 */
void gui_render(ProgContext ctx, Fhwcg_Gui_Func func);

/**
 * Gibt die Ressourcen des GUI wieder frei.
 * 
 * @param[in] ctx Programmkontext.
 */
void gui_cleanup(ProgContext ctx);

/**
 * Hilfswidget um einen Colorpicker anzuzeigen, der die Farbe
 * als vec4 speichert.
 * 
 * @param[in] ctx Programmkontext.
 * @param[in] name der anzuzeigende Name
 * @param[in,out] col die einstellbare Farbe
 */
void gui_widgetColor(ProgContext ctx, const char* name, vec4 col);

/**
 * Hilfswidget um einen Colorpicker anzuzeigen, der die Farbe
 * als vec3 speichert.
 * 
 * @param[in] ctx Programmkontext.
 * @param[in] name der anzuzeigende Name
 * @param[in,out] col die einstellbare Farbe
 */
void gui_widgetColor3(ProgContext ctx, const char* name, vec3 col);

/**
 * Hilfswidget um einen 3D Vektor anzupassen.
 * 
 * @param[in] ctx Programmkontext.
 * @param[in] name der anzuzeigende Name
 * @param[in,out] val der einstellbare Vektor
 * @param[in] limit Obergrenze für die Werte (wird auch für die Untergrenze genutzt)
 * @param[in] step Schrittgröße bei Klick auf einen Pfeil
 * @param[in] inc_per_pixel Wertänderung beim Ziehen pro Pixel
 */
void gui_widgetVec3(ProgContext ctx, const char* name, vec3 val, float limit, float step, float inc_per_pixel);

/**
 * Zeigt ein Hilfefenster an, in dem alle Maus- und Tastaturbefehle aufgelistet
 * werden.
 * 
 * @param[in] ctx Programmkontext.
 * @param[in] lines Auflistung der Tastenbefehle
 * @param[in] numLines Anzahl an Zeilen
 * @param[in] rect Position und Größe des Fensters
 * 
 * @return neuer Status des Fensters
 */
bool gui_widgetHelp(ProgContext ctx, const GuiHelpLine lines[], unsigned int numLines, struct nk_rect rect);

/**
 * Zeigt ein Fenster mit den Achsen im Worldspace an.  
 * Hilfreich zur Orientierung.
 * 
 * @param[in] ctx Programmkontext.
 * @param[in] bounds Position und Größe des Fensters
 * @param[in] shader Shader zum Zeichnen
 * @param[in] viewMatrix die aktuelle View Matrix
 * 
 * @note Der Shader muss so aufgebaut sein wie der Stack Demo Shader
 */
void gui_widgetAxes(ProgContext ctx, struct nk_rect bounds, Shader* shader, mat4* viewMatrix);

/**
 * Zeigt ein RenderWindow an.  
 * Ein RenderWindow ist ein Fenster des GUI, dessen Inhalt zur Laufzeit gerendert wird.  
 * Das erzeugte RenderWindow kann sowohl in der Position als auch Größe verändert werden.
 * 
 * @note Dieses Widget nutzt Texture Unit 0
 * 
 * @param[in] ctx Programmkontext.
 * @param[in] name der anzuzeigende Name
 * @param[in] renderWindow das RenderWindow
 * @param[in] bounds (Start)Position und (Start)Größe des Fensters
 * @param[in] func Zeichenfunktion, die den Inhalt erzeugt
 * @param[in] data Daten, die an die Zeichenfunktion weitergereicht werden sollen
 */
void gui_widgetRenderWindow(ProgContext ctx, const char* name, GuiRenderWindow renderWindow, struct nk_rect bounds, Fhwcg_Gui_RenderImageFunc func, void* data);

/**
 * Ein spezielles RenderWindow, was den Inhalt in einer festen Auflösung rendert.  
 * Sinnvoll, wenn der Inhalt noch weiter genutzt werden soll.  
 * Das erzeugte RenderWindow kann verschoben werden werden.
 * 
 * @note Die erzeugte Textur nutzt 4 Kanäle (RGBA), die Daten sind vom Typ Float
 * 
 * @param[in] ctx Programmkontext.
 * @param[in] name der anzuzeigende Name
 * @param[in] renderWindow das RenderWindow
 * @param[in] bounds (Start-)Position und Inhaltsgröße des Fensters
 * @param[in] scaling Skalierung der Anzeige. Der Inhalt wird weiterhin in der von @p bounds spezifizierten Auflösung gerendert. Über diesen Parameter wird nur die anschließende Anzeige skaliert.
 * @param[in] texUnit Die Texture Unit, die zur Anzeige genutzt werden soll, standardmäßig 0
 * @param[in] func Zeichenfunktion, die den Inhalt erzeugt
 * @param[in] data Daten, die an die Zeichenfunktion weitergereicht werden sollen
 */
void gui_widgetRenderWindowFixed(ProgContext ctx, const char* name, GuiRenderWindow renderWindow, struct nk_rect bounds, float scaling, GLuint texUnit, Fhwcg_Gui_RenderImageFunc func, void* data);

/**
 * Erzeugt ein RenderWindow
 * 
 * @return das RenderWindow
 */
GuiRenderWindow gui_createRenderWindow(void);

/**
 * Gibt die durch gui_createRenderWindow() belegten Ressourcen wieder frei.
 * 
 * @param[in,out] window Das Fenster, dessen Ressourcen freigegeben werden sollen. Die referenzierte Variable wird auf NULL gesetzt.
 */
void gui_disposeRenderWindow(GuiRenderWindow* window);

/**
 * Erzeugt ein neues Fenster.
 * 
 * @param[in] ctx Programmkontext.
 * @param[in] title Anzeigename und Titel
 * @param[in] bounds Position und Größe des Fensters
 * @param[in] flags Fenstereigenschaften
 * 
 * @return true, wenn das Fenster gefüllt werden kann, sonst false (z.B. wenn minimiert)
 * 
 * @see nk_begin()  
 * https://immediate-mode-ui.github.io/Nuklear/doc/index.html#nk_begin
 * 
 * @see nk_window_flags
 */
bool gui_begin(ProgContext ctx, const char *title, struct nk_rect bounds, nk_flags flags);

/**
 * Erzeugt ein neues Fenster.  
 * Erweiterung von gui_begin() mit separatem Namen und Titel.  
 * So können mehrere Fenster mit gleichem Titel erzeugt werden.
 * 
 * @param[in] ctx Programmkontext.
 * @param[in] name ID des Fensters
 * @param[in] title Titel des Fensters
 * @param[in] bounds Position und Größe des Fensters
 * @param[in] flags Fenstereigenschaften
 *
 * @return true, wenn das Fenster gefüllt werden kann, sonst false (z.B. wenn minimiert)
 * 
 * @see nk_begin_titled()  
 * https://immediate-mode-ui.github.io/Nuklear/doc/index.html#nk_begin_titled
 */
bool gui_beginTitled(ProgContext ctx, const char *name, const char *title, struct nk_rect bounds, nk_flags flags);

/**
 * Beendet den Erzeugungsprozess eines Fensters.
 * 
 * @param[in] ctx Programmkontext.
 * 
 * @see nk_end()  
 * https://immediate-mode-ui.github.io/Nuklear/doc/index.html#nk_end
 */
void gui_end(ProgContext ctx);

/**
 * Startet eine Tree View.
 * 
 * Tree Views erlauben es, Kontrollfelder hierarchisch zu sortieren.  
 * Einzelne Knoten lassen sich ein-/ausklappen.  
 * Es gibt zwei Arten von Tree-Views, diese unterschieden sich nur in der Art der Darstellung:
 * - NK_TREE_TAB  : Darstellung mit einer Leiste als Überschrift
 * - NK_TREE_NODE : Darstellung als einfacher Baum
 * 
 * @param[in] ctx Programmkontext.
 * @param[in] type NK_TREE_TAB oder NK_TREE_NODE
 * @param[in] title Anzeigename und Titel
 * @param[in] state NK_MINIMIZED oder NK_MAXIMIZED 
 * 
 * @see nk_tree_push()  
 * https://immediate-mode-ui.github.io/Nuklear/doc/index.html#nk_tree_push
 * 
 * @see nk_collapse_states
 */
#define gui_treePush(ctx, type, title, state) gui_treePush_internal(ctx, type, title, state, NK_FILE_LINE,nk_strlen(NK_FILE_LINE),__LINE__)

/**
 * Startet eine Tree View.
 * 
 * Variante von gui_treePush() mit separater ID. Nützlich wenn Bäume in einer Schleife erzeugt werden.
 * 
 * Tree Views erlauben es, Kontrollfelder hierarchisch zu sortieren.  
 * Einzelne Knoten lassen sich ein-/ausklappen.  
 * Es gibt zwei Arten von Tree-Views, diese unterschieden sich nur in der Art der Darstellung:
 * - NK_TREE_TAB  : Darstellung mit einer Leiste als Überschrift
 * - NK_TREE_NODE : Darstellung als einfacher Baum
 * 
 * @param[in] ctx Programmkontext.
 * @param[in] type NK_TREE_TAB oder NK_TREE_NODE
 * @param[in] title Titel
 * @param[in] state NK_MINIMIZED oder NK_MAXIMIZED 
 * @param[in] id ID bzw. Index des Elements
 * 
 * @see nk_tree_push_id()  
 * https://immediate-mode-ui.github.io/Nuklear/doc/index.html#nk_tree_push_id
 * 
 * @see nk_collapse_states
 */
#define gui_treePushId(ctx, type, title, state, id) gui_treePush_internal(ctx, type, title, state, NK_FILE_LINE,nk_strlen(NK_FILE_LINE),id)

/**
 * Wrapper für nk_tree_push_hashed()
 * 
 * @see https://immediate-mode-ui.github.io/Nuklear/doc/index.html#nk_tree_push_hashed
 * 
 * @warning
 * Sollte nicht direkt aufgerufen werden sondern nur implizit über gui_treePush() und gui_treePushId()
 * 
 * @param[in] ctx Programmkontext.
 * @param[in] type NK_TREE_TAB oder NK_TREE_NODE
 * @param[in] title Titel
 * @param[in] state NK_MINIMIZED oder NK_MAXIMIZED 
 * @param[in] hash Speicherblock oder String, aus dem die ID des Elementes errechnet wird
 * @param[in] len Größe von @p hash in Byte
 * @param[in] seed Seed Value, falls der gleiche Aufruf mehrere Elemente erzeugt (z.B. in einer Schleife)
 * 
 * @return True, wenn das Element sichtbar/ausgeklappt/füllbar ist, sonst False
 */
bool gui_treePush_internal(ProgContext ctx, enum nk_tree_type type, const char* title, 
                           enum nk_collapse_states state, const char *hash, int len, int seed);

/**
 * Beendet eine Tree View.
 * 
 * @param[in] ctx Programmkontext.
 * 
 * @see nk_tree_pop()  
 * https://immediate-mode-ui.github.io/Nuklear/doc/index.html#nk_tree_pop
 */
void gui_treePop(ProgContext ctx);

/**
 * Setzt ein dynamisches Layout, was die Breite automatisch an den zur Verfügung stehendem Platz
 * anpasst.  
 * Der Platz wird gleichmäßig auf @p cols Elemente aufgeteilt.
 * 
 * @param[in] ctx Programmkontext.
 * @param[in] height Höhe einer Zeile
 * @param[in] cols Anzahl an Spalten
 * 
 * @see nk_layout_row_dynamic()  
 * https://immediate-mode-ui.github.io/Nuklear/doc/index.html#nk_layout_row_dynamic
 */
void gui_layoutRowDynamic(ProgContext ctx, float height, int cols);

/**
 * Erzeugt einen einfachen Button.
 * 
 * @param[in] ctx Programmkontext.
 * @param[in] title Anzeigename und Titel
 * 
 * @return true, wenn der Button diesen Frame ausgelöst wurde. Sonst false.
 * 
 * @see nk_button_label()
 * https://immediate-mode-ui.github.io/Nuklear/doc/index.html#nk_button_label
 */
bool gui_button(ProgContext ctx, const char* title);

/**
 * Erzeugt eine Kontrollkästchen (Ein-/Ausschalter)
 * 
 * @param[in] ctx Programmkontext.
 * @param[in] title Anzeigename und Titel
 * @param[in,out] active aktueller Zustand
 * 
 * @return true, wenn der Wert diesen Frame verändert wurde. Sonst false.
 * 
 * @see nk_checkbox_label()
 * https://immediate-mode-ui.github.io/Nuklear/doc/index.html#nk_checkbox_label
 */
bool gui_checkbox(ProgContext ctx, const char* title, bool* active);

/**
 * Zeigt einen Text an.
 * 
 * @param[in] ctx Programmkontext.
 * @param[in] title Anzeigename und Titel
 * @param[in] align Ausrichtung des Textes.
 * 
 * @see nk_label()
 * https://immediate-mode-ui.github.io/Nuklear/doc/index.html#nk_label
 * 
 * @see nk_text_alignment
 */
void gui_label(ProgContext ctx, const char* title, nk_flags align);

/**
 * Zeigt einen farbigen Text an.  
 * Die Farbe gilt nur für dieses Objekt.
 * 
 * @param[in] ctx Programmkontext.
 * @param[in] title Anzeigename und Titel
 * @param[in] align Ausrichtung des Textes.
 * @param[in] color RGB Farbe, jeder Kanal im Bereich [0..255]
 * 
 * @see gui_label()
 */
void gui_labelColor(ProgContext ctx, const char* title, nk_flags align, ivec3 color);

/**
 * Option, auch als Radio-Button bekannt.  
 * Hilfreich wenn eine Option aus einer Menge ausgewählt werden soll.
 * 
 * @param[in] ctx Programmkontext.
 * @param[in] title Anzeigename und Titel
 * @param[in] active aktueller Zustand
 * 
 * @return true, wenn der Zustand geändert wurde, sonst false
 * 
 * @see nk_option_label
 * https://immediate-mode-ui.github.io/Nuklear/doc/index.html#nk_option_label
 * 
 */
bool gui_option(ProgContext ctx, const char* title, bool active);

/**
 * Erzeugt einen Slider.  
 * Dieser Slider erlaubt es sowohl durch Ziehen mit der Maus den Wert zu modifizieren, als auch
 * manuell einen Wert einzugeben.
 * 
 * @param[in] ctx Programmkontext.
 * @param[in] name Anzeigename und Titel
 * @param[in] min Unteres Limit für den Wert
 * @param[in,out] val aktueller Wert
 * @param[in] max Oberes Limit für den Wert
 * @param[in] step Schrittgröße bei Klick auf einen Pfeil
 * @param[in] inc_per_pixel Wertänderung beim Ziehen pro Pixel
 * 
 * @see nk_property_int
 * https://immediate-mode-ui.github.io/Nuklear/doc/index.html#nk_property_int
 */
void gui_propertyInt(ProgContext ctx, const char *name, int min, int* val, int max, int step, float inc_per_pixel);

/**
 * Erzeugt einen Slider.  
 * Dieser Slider erlaubt es sowohl durch Ziehen mit der Maus den Wert zu modifizieren, als auch
 * manuell einen Wert einzugeben.
 * 
 * @param[in] ctx Programmkontext.
 * @param[in] name Anzeigename und Titel
 * @param[in] min Unteres Limit für den Wert
 * @param[in] val aktueller Wert
 * @param[in] max Oberes Limit für den Wert
 * @param[in] step Schrittgröße bei Klick auf einen Pfeil
 * @param[in] inc_per_pixel Wertänderung beim Ziehen pro Pixel
 * 
 * @return neuer Wert
 * 
 * @see nk_propertyi
 * https://immediate-mode-ui.github.io/Nuklear/doc/index.html#nk_propertyi
 */
int gui_propertyI(ProgContext ctx, const char *name, int min, int val, int max, int step, float inc_per_pixel);

/**
 * Erzeugt einen Slider.  
 * Dieser Slider erlaubt es sowohl durch Ziehen mit der Maus den Wert zu modifizieren, als auch
 * manuell einen Wert einzugeben.
 * 
 * @param[in] ctx Programmkontext.
 * @param[in] name Anzeigename und Titel
 * @param[in] min Unteres Limit für den Wert
 * @param[in,out] val aktueller Wert
 * @param[in] max Oberes Limit für den Wert
 * @param[in] step Schrittgröße bei Klick auf einen Pfeil
 * @param[in] inc_per_pixel Wertänderung beim Ziehen pro Pixel
 * 
 * @see nk_property_float
 * https://immediate-mode-ui.github.io/Nuklear/doc/index.html#nk_property_float
 */
void gui_propertyFloat(ProgContext ctx, const char *name, float min, float* val, float max, float step, float inc_per_pixel);

/**
 * Erzeugt einen Slider.  
 * Dieser Slider erlaubt es sowohl durch Ziehen mit der Maus den Wert zu modifizieren, als auch
 * manuell einen Wert einzugeben.
 * 
 * @param[in] ctx Programmkontext.
 * @param[in] name Anzeigename und Titel
 * @param[in] min Unteres Limit für den Wert
 * @param[in] val aktueller Wert
 * @param[in] max Oberes Limit für den Wert
 * @param[in] step Schrittgröße bei Klick auf einen Pfeil
 * @param[in] inc_per_pixel Wertänderung beim Ziehen pro Pixel
 * 
 * @return neuer Wert
 * 
 * @see nk_propertyf
 * https://immediate-mode-ui.github.io/Nuklear/doc/index.html#nk_propertyf
 */
float gui_propertyF(ProgContext ctx, const char *name, float min, float val, float max, float step, float inc_per_pixel);

/**
 * Erzeugt einen Slider.  
 * Dieser Slider erlaubt es sowohl durch Ziehen mit der Maus den Wert zu modifizieren, als auch
 * manuell einen Wert einzugeben.
 * 
 * @param[in] ctx Programmkontext.
 * @param[in] name Anzeigename und Titel
 * @param[in] min Unteres Limit für den Wert
 * @param[in,out] val aktueller Wert
 * @param[in] max Oberes Limit für den Wert
 * @param[in] step Schrittgröße bei Klick auf einen Pfeil
 * @param[in] inc_per_pixel Wertänderung beim Ziehen pro Pixel
 * 
 * @see nk_property_double
 * https://immediate-mode-ui.github.io/Nuklear/doc/index.html#nk_property_double
 */
void gui_propertyDouble(ProgContext ctx, const char *name, double min, double* val, double max, double step, float inc_per_pixel);

/**
 * Erzeugt einen Slider.  
 * Dieser Slider erlaubt es sowohl durch Ziehen mit der Maus den Wert zu modifizieren, als auch
 * manuell einen Wert einzugeben.
 * 
 * @param[in] ctx Programmkontext.
 * @param[in] name Anzeigename und Titel
 * @param[in] min Unteres Limit für den Wert
 * @param[in] val aktueller Wert
 * @param[in] max Oberes Limit für den Wert
 * @param[in] step Schrittgröße bei Klick auf einen Pfeil
 * @param[in] inc_per_pixel Wertänderung beim Ziehen pro Pixel
 * 
 * @return neuer Wert
 * 
 * @see nk_propertyd
 * https://immediate-mode-ui.github.io/Nuklear/doc/index.html#nk_propertyd
 */
double gui_propertyD(ProgContext ctx, const char *name, double min, double val, double max, double step, float inc_per_pixel);

/**
 * Erzeugt ein Dropdown Menü.
 * 
 * @param[in] ctx Programmkontext.
 * @param[in] items Die Auswahlmöglichkeiten
 * @param[in] count Anzahl an Auswahlmöglichkeiten
 * @param[in] selected Index der bisher ausgewählten Möglichkeit
 * @param[in] item_height Vertikaler Platz pro Möglichkeit
 * @param[in] size Breite und Höhe der Anzeige
 * 
 * @return Index des aktuell ausgewählten Elements
 * 
 * Das Dropdown Menü nutzt die Nuklear ComboBox.  
 * Diese können noch deutlich mehr als nur einfache Dropdowns. Wir nutzen sie zum Beispiel auch in
 * gui_widgetColor() und gui_widgetVec3().
 * 
 * Die erweiterten Möglichkeiten sind für CG erstmal nicht weiter relevant. Bei Interesse könnt ihr
 * im verlinkten Codeabschnitt ein paar erweiterte Beispiele anschauen.
 * 
 * @see
 * https://github.com/Immediate-Mode-UI/Nuklear/blob/a315d25b4c33efa0e112cab9562460e785935a10/demo/common/overview.c#L356
 */
int gui_dropdown(ProgContext ctx, const char** items, int count, int selected, int item_height, struct nk_vec2 size);

/**
 * Liefert die aktuellen Anzeigeeinstellungen des GUI
 * 
 * @param[in] ctx Programmkontext
 * 
 * @return aktuelle Anzeigeeinstellungen
 * 
 * @see https://immediate-mode-ui.github.io/Nuklear/structnk__style.html
 */
struct nk_style gui_getStyle(ProgContext ctx);

/**
 * Überschreibt die aktuellen Anzeigeeinstellungen des GUI
 * 
 * @warning Mit schlecht gewählten Werten kann man sich schnell das GUI zerstören.
 *          Deswegen ist es nicht empfohlen diese Einstellungen zu ändern, ihr dürft es aber.
 * 
 * @param[in] ctx Programmkontext
 * @param[in] style neue Anzeigeeinstellungen
 * 
 * @see https://immediate-mode-ui.github.io/Nuklear/structnk__style.html
 */
void gui_setStyle(ProgContext ctx, struct nk_style style);

#endif
