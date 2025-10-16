#include <fhwcg/fhwcg.h>

////////////////////////////////// KONSTANTEN //////////////////////////////////

#define DEFAULT_WINDOW_WIDTH 1200
#define DEFAULT_WINDOW_HEIGHT 800

//////////////////////////// ÖFFENTLICHE FUNKTIONEN ////////////////////////////

int main(void) 
{
    // Zuerst muss das gesamte Programm initialisiert und ein Fenster erzeugt werden.
    ProgContext ctx = window_init(PROGRAM_NAME, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, 1, HELP_SERVER_FLAGS | WINDOW_FLAGS_VSYNC);

    // Diese Farbe wird für das glClear genutzt
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Im zweiten Schritt wird die Hauptschleife des Programms gestartet.
    while (window_startNewFrame(ctx))
    {
        // Alles in dieser Schleife wird für jeden Frame gemacht

        // Im dritten Schritt wird die Logik aktualisiert
        // z.B. updateLogic()

        // Im vierten Schritt wird der Bildschirm "geleert" und dann die Szene neu gezeichnet
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // z.B. drawScene();

        // Am Ende jedes Frames werden Front- und Backbuffer gewechselt
        window_swapBuffers(ctx);
    }

    // Wenn das Programm beendet wird, treten wir aus der Hauptschleife aus und
    // geben alle belegten Ressourcen wieder frei.
    window_cleanup(ctx);

    return EXIT_SUCCESS;
}
