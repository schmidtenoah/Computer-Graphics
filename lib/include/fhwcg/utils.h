/**
 * @file
 * 
 * @brief
 * Allgemeine Hilfsfunktionen, die überall nützlich sein
 * können.
 * 
 * @copyright 2020, FH Wedel
 * @author Nicolas Hollmann
 */

#ifndef FHWCG_UTILS_H
#define FHWCG_UTILS_H

#include <stdbool.h>

//////////////////////////// ÖFFENTLICHE FUNKTIONEN ////////////////////////////

/**
 * Hilfsfunktion zum Einlesen einer Datei in den Arbeitsspeicher.
 * 
 * @note Der zurückgegebene String muss mit free wieder gelöscht werden.
 * 
 * @param[in] filename der Dateiname der einzulesenden Datei
 * @return der Inhalt der eingelesenen Datei
 */
char* utils_readFile(const char* filename);

/**
 * Prüft, ob ein Suffix am Ende eines Strings zu finden ist.  
 * Diese Funktion kann zum Beispiel genutzt werden, um Dateiendungen
 * abzufragen: utils_hasSuffix(filename, ".png");
 * 
 * @note Ein leeres Suffix ist nie Teil des Subjektes.
 * 
 * @param[in] subject der String, bei dem das Suffix gesucht werden soll
 * @param[in] suffix das zu suchende Suffix
 * @return true, wenn das Suffix vorhanden ist, false wenn nicht
 */
bool utils_hasSuffix(const char* subject, const char* suffix);

/**
 * Gibt den Ordnerpfad einer Datei zurück. Dabei handelt es sich um eine
 * reine String-Bearbeitung, es findet keine Validierung auf Dateisystem-
 * Ebene statt. Auch ".." Verzeichnisse bleiben erhalten.
 * 
 * @note Der zurückgegebene String muss selbstständig wieder freigegeben werden.
 * 
 * @param[in] filepath der Dateipfad, aus dem der Ordner extrahiert werden soll
 * @return der Pfad des Ordners
 */
char* utils_getDirectory(const char* filepath);

/**
 * Gibt den Dateinamen in einem Pfad zurück. Dabei handelt es sich um eine
 * reine String-Bearbeitung, es findet keine Validierung auf Dateisystem-
 * Ebene statt.
 * 
 * @note Der zurückgegebene String muss selbstständig wieder freigegeben werden.
 * 
 * @param[in] filepath der Dateipfad, aus dem der Dateiname extrahiert werden soll
 * @return der Dateiname
 */
char* utils_getFilename(const char* filepath);

/**
 * Gibt den Größeren von zwei Integern zurück.
 * 
 * @note Diese Funktionalität wurde als Funktion und nicht als Makro umgesetzt,
 * da ein Makro schnell zu Fehlern führen kann:  
 * Das Makro MAX(a. b) ((a) > (b) ? (a) : (b)) führt bei a = x++ und b = y++
 * zu folgender Ersetzung: ((x++) > (y++) ? (x++) : (y++))  
 * Demnach würde der jeweils größere Wert zweimal erhöht werden. Solche Probleme
 * können auch in anderer Weise auftreten, weshalb eine Funktion hier sicherer
 * ist.
 * 
 * @param[in] a der erste Integer.
 * @param[in] b der zweite Integer.
 * @return der gößere Wert.
 */
int utils_maxInt(int a, int b);

/**
 * Gibt den Kleineren von zwei Integern zurück.
 * 
 * @note Diese Funktionalität wurde als Funktion und nicht als Makro umgesetzt,
 * da ein Makro schnell zu Fehlern führen kann:  
 * Das Makro MIN(a. b) ((a) < (b) ? (a) : (b)) führt bei a = x++ und b = y++
 * zu folgender Ersetzung: ((x++) < (y++) ? (x++) : (y++))  
 * Demnach würde der jeweils kleinere Wert zweimal erhöht werden. Solche 
 * Probleme können auch in anderer Weise auftreten, weshalb eine Funktion hier 
 * sicherer ist.
 * 
 * @param[in] a der erste Integer.
 * @param[in] b der zweite Integer.
 * @return der kleinere Wert.
 */
int utils_minInt(int a, int b);

/**
 * Gibt einen String zurück, der beschreibt, ob es sich um eine 32bit oder 64bit
 * Architektur handelt.
 * 
 * @warning Der String liegt im statischem Speicher und darf NICHT verändert
 * werden. Er muss NICHT wieder freigegeben werden.
 * 
 * @return der beschreibende String
 */
const char* utils_getArchBits(void);

#endif // FHWCG_UTILS_H
