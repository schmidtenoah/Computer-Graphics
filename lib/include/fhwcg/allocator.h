/**
 * @file
 * 
 * @brief
 * Überwacht die Speichernutzung. Am Ende des Programms wird überprüft, ob sämtlicher Speicher auch
 * wieder freigegeben wurde.
 * 
 * @warning Keine der Funktionen (bis auf alloc_checkLeak()) darf direkt aufgerufen werden. Aufrufe sind nur das Resultat von Funktionsmakros.
 * 
 * @copyright 2025, FH Wedel
 * @author Philipp Munz
 */

#ifndef FHWCG_ALLOC_H
#define FHWCG_ALLOC_H

// Der Speicher Check soll aus Performancegründen nur im Debug Modus stattfinden
#ifdef _DEBUG

    #include <fhwcg/common.h>

    // Wir wollen diese Funktionsmakros selbst überschreiben
    #undef glGenVertexArrays
    #undef glDeleteVertexArrays
    #undef glGenBuffers
    #undef glDeleteBuffers
    #undef glCreateShader
    #undef glDeleteShader
    #undef glCreateProgram
    #undef glDeleteProgram
    #undef glGenTextures
    #undef glDeleteTextures
    #undef glGenFramebuffers
    #undef glDeleteFramebuffers

    // Enums direkt in Strings umwandeln zu können wäre ja zu einfach, deswegen dengeln wir uns das
    // mit Makros zurecht. Nicht hübsch, aber besser als ein String Array manuell zu verwalten.
    #define FOREACH_ALLOC(TYPE) \
        TYPE(MESH)    \
        TYPE(TEXTURE) \
        TYPE(SHADER)  \
        TYPE(GUI)     \
        TYPE(VAO)     \
        TYPE(BO)      \
        TYPE(PROGRAM) \
        TYPE(FBO)

    #define GENERATE_ALLOC_ENUM(ENUM) ALLOC_##ENUM,
    #define GENERATE_STRING(STRING) #STRING,

    /** Typen der Allokationen */
    typedef enum{
        FOREACH_ALLOC(GENERATE_ALLOC_ENUM)

        ALLOC_NUM_ALLOC_TYPES
    } AllocType;

    #undef GENERATE_ALLOC_ENUM

    /**
     * Wrapper für malloc
     * 
     * @param[in] size Größe des Speicherbereichs in Byte
     * @param[in] file Datei, in der die Funktion aufgerufen wird
     * @param[in] line Zeile, in der die Funktion aufgerufen wird
     * 
     * @return Adresse des Speicherbereichs
     * 
     * @see malloc in stdlib.h
     */
    void* fhwcg_malloc(size_t size, const char* file, int line);

    /**
     * Wrapper für free
     * 
     * @param[in] ptr Speicherbereich, welcher freigegeben werden soll
     * 
     * @see free in stdlib.h
     */
    void fhwcg_free(void *ptr);

    /**
     * Wrapper für realloc
     * 
     * @param[in] ptr bisherige Speicheradresse
     * @param[in] size Größe des neuen Speicherbereichs in Byte. Bei einer Größe von 0 verhält sich
     *                 die Funktion wie free()
     * @param[in] file Datei, in der die Funktion aufgerufen wird
     * @param[in] line Zeile, in der die Funktion aufgerufen wird
     * 
     * @return Adresse des neuen Speicherbereichs oder NULL bei einer @p size von 0
     * 
     * @see realloc in stdlib.h
     */
    void* fhwcg_realloc(void *ptr, size_t size, const char* file, int line);

    /**
     * Wrapper für calloc
     * 
     * @param[in] count Anzahl an Elementen
     * @param[in] size Größe eines Elementes in Byte
     * @param[in] file Datei, in der die Funktion aufgerufen wird
     * @param[in] line Zeile, in der die Funktion aufgerufen wird
     * 
     * @return Adresse des Speicherbereichs
     * 
     * @see calloc in stdlib.h
     */
    void* fhwcg_calloc(size_t count, size_t size, const char* file, int line);

    /**
     * Wrapper für glGenVertexArrays
     * 
     * @param[in] n Anzahl an zu erzeugenden Elementen
     * @param[in] arrays Hier werden die IDs abgelegt
     * 
     * @see glGenVertexArrays()
     */
    void fhwcg_glGenVertexArrays(GLsizei n, GLuint* arrays);

    /**
     * Wrapper für glDeleteVertexArrays
     * 
     * @param[in] n Anzahl an freizugebenden Elementen
     * @param[in] arrays die IDs
     * 
     * @see glDeleteVertexArrays()
     */
    void fhwcg_glDeleteVertexArrays(GLsizei n, const GLuint* arrays);

    /**
     * Wrapper für glGenBuffers
     * 
     * @param[in] n Anzahl an zu erzeugenden Elementen
     * @param[in] buffers Hier werden die IDs abgelegt
     * 
     * @see glGenBuffers()
     */
    void fhwcg_glGenBuffers(GLsizei n, GLuint* buffers);

    /**
     * Wrapper für glDeleteBuffers
     * 
     * @param[in] n Anzahl an freizugebenden Elementen
     * @param[in] buffers die IDs
     * 
     * @see glDeleteBuffers()
     */
    void fhwcg_glDeleteBuffers(GLsizei n, const GLuint* buffers);

    /**
     * Wrapper für glCreateShader
     * 
     * @param[in] shaderType Typ des Shader-Objektes
     * 
     * @return ID des erzeugten Shader-Objektes
     * 
     * @see glCreateShader()
     */
    GLuint fhwcg_glCreateShader(GLenum shaderType);

    /**
     * Wrapper für glDeleteShader
     * 
     * @param[in] shader ID des freizugebenden Shaders
     * 
     * @see glDeleteShader()
     */
    void fhwcg_glDeleteShader(GLuint shader);

    /**
     * Wrapper für glCreateProgram
     * 
     * @return ID des erzeugten Shader-Programmes
     * 
     * @see glCreateProgram()
     */
    GLuint fhwcg_glCreateProgram(void);

    /**
     * Wrapper für glDeleteProgram
     * 
     * @param[in] program ID des freizugebenden Shader-Programmes
     * 
     * @see glDeleteProgram()
     */
    void fhwcg_glDeleteProgram(GLuint program);

    /**
     * Wrapper für glGenTextures
     * 
     * @param[in] n Anzahl an zu erzeugenden Elementen
     * @param[in] textures Hier werden die IDs abgelegt
     * 
     * @see glGenTextures()
     */
    void fhwcg_glGenTextures(GLsizei n, GLuint* textures);

    /**
     * Wrapper für glDeleteTextures
     * 
     * @param[in] n Anzahl an freizugebenden Elementen
     * @param[in] textures die IDs
     * 
     * @see glDeleteTextures()
     */
    void fhwcg_glDeleteTextures(GLsizei n, GLuint* textures);

    /**
     * Wrapper für glGenFramebuffers
     * 
     * @param[in] n Anzahl an zu erzeugenden Elementen
     * @param[in] ids Hier werden die IDs abgelegt
     * 
     * @see glGenFramebuffers()
     */
    void fhwcg_glGenFramebuffers(GLsizei n, GLuint* ids);

    /**
     * Wrapper für glDeleteFramebuffers
     * 
     * @param[in] n Anzahl an freizugebenden Elementen
     * @param[in] framebuffers die IDs
     * 
     * @see glDeleteFramebuffers()
     */
    void fhwcg_glDeleteFramebuffers(GLsizei n, GLuint* framebuffers);

    /**
     * Testet, ob bisher alle Ressourcen wieder freigegeben wurden.  
     * Wenn nicht, werden Art und Anzahl der Ressourcen auf der Konsole ausgegeben. 
     * Wenn ja, wird eine entsprechende Ausgabe auf der Konsole gemacht.
     * 
     * Diese Funktion wird im Debug Modus automatisch am Ende des Programmes aufgerufen.
     * 
     * @return true, wenn ein Speicherleck aufgetreten ist, sonst false
     */
    bool alloc_checkLeak(void);

    /** Anstatt malloc rufen wir unseren Wrapper auf */
    #define malloc(size) fhwcg_malloc(size, __FILE__, __LINE__)

    /** Anstatt free rufen wir unseren Wrapper auf */
    #define free(ptr) fhwcg_free(ptr)

    /** Anstatt realloc rufen wir unseren Wrapper auf */
    #define realloc(ptr, new_size) fhwcg_realloc(ptr, new_size, __FILE__, __LINE__)

    /** Anstatt calloc rufen wir unseren Wrapper auf */
    #define calloc(num, size) fhwcg_calloc(num, size, __FILE__, __LINE__)

    /** Anstatt glGenVertexArrays rufen wir unseren Wrapper auf */
    #define glGenVertexArrays(n, arrays) fhwcg_glGenVertexArrays(n, arrays)

    /** Anstatt glDeleteVertexArrays rufen wir unseren Wrapper auf */
    #define glDeleteVertexArrays(n, arrays) fhwcg_glDeleteVertexArrays(n, arrays)

    /** Anstatt glGenBuffers rufen wir unseren Wrapper auf */
    #define glGenBuffers(n, buffers) fhwcg_glGenBuffers(n, buffers)

    /** Anstatt glDeleteBuffers rufen wir unseren Wrapper auf */
    #define glDeleteBuffers(n, buffers) fhwcg_glDeleteBuffers(n, buffers)

    /** Anstatt glCreateShader rufen wir unseren Wrapper auf */
    #define glCreateShader(shaderType) fhwcg_glCreateShader(shaderType)

    /** Anstatt glDeleteShader rufen wir unseren Wrapper auf */
    #define glDeleteShader(shader) fhwcg_glDeleteShader(shader)

    /** Anstatt glCreateProgram rufen wir unseren Wrapper auf */
    #define glCreateProgram() fhwcg_glCreateProgram()

    /** Anstatt glDeleteProgram rufen wir unseren Wrapper auf */
    #define glDeleteProgram(program) fhwcg_glDeleteProgram(program)

    /** Anstatt glGenTextures rufen wir unseren Wrapper auf */
    #define glGenTextures(n, textures) fhwcg_glGenTextures(n, textures)

    /** Anstatt glDeleteTextures rufen wir unseren Wrapper auf */
    #define glDeleteTextures(n, textures) fhwcg_glDeleteTextures(n, textures)

    /** Anstatt glGenFramebuffers rufen wir unseren Wrapper auf */
    #define glGenFramebuffers(n, ids) fhwcg_glGenFramebuffers(n, ids)

    /** Anstatt glDeleteFramebuffers rufen wir unseren Wrapper auf */
    #define glDeleteFramebuffers(n, framebuffers) fhwcg_glDeleteFramebuffers(n, framebuffers)

#else

    #include <stdlib.h>

#endif

#endif
