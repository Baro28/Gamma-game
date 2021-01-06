/** @file
 * Interfejs klasy obsługującej input
 *
 * @author Bartosz Ławny <bl418396@students.mimuw.edu.pl>
 * @copyright Uniwersytet Warszawski
 * @date 17.05.2020
 */

#ifndef GAMMA_INPUT_H
#define GAMMA_INPUT_H
#include <inttypes.h>
#include <stdbool.h>

/** @brief Pomija białe znaki w przekazanym napisie
 * Pomija wszystkie białe znaki oprócz '\n'
 * @param[in, out] string   – napis który przewijamy
 */

void skipWhitespace(char **string);

/** @brief Sprawdza czy liczba parametrów się zgadza
 * Sprawdza czy w przekazanym napisie mamy @param number liczbę
 * wyrazów które zawierają tylko cyfry
 * @param[in] string   – napis w którym sprawdzamy parametry,
 * @param[in] number   – sprawdzana liczba parametrów
 * @return Zwraca true jeśli liczba parametrów się zgadza lub
 * false jeśli liczba parametrów się nie zgadza albo występują znaki inne niż cyfry
 */

bool isNumberOfWordsCorrect(char *string, uint32_t number);

/** @brief Wyciąga liczbę z napisu i ją zwraca
 * Z przekazanego napisu wyciąga jeden wyraz i
 * wrzuca go do funkcji strtoul po czym sprawdza czy liczba mieści się
 * w uint32_t. Jeśli tak to ją zwraca, jeśli nie to zwraca 0.
 * Zmienia również flagę error w zależności od tego czy liczba mieści się w
 * uint32_t.
 * @param[in] string   – napis z którego wyciągamy liczbę,
 * @param[in, out] error - flaga informująca że wystąpił błąd.
 * @return Zwraca liczbę wyciągniętą z napisu lub 0 jeśli liczba jest większa niż
 * uint32_t.
 */

uint32_t word_to_int(char **string, bool *error);

/** @brief Główna funkcja obsługująca input
 * Zawiera główną pętlę przyjmującą input i wypissującą
 * ERROR line jeśli nie spełnia on specyfikacji
 * lub przechodzi do trybu wsadowego lub interaktywnego zależnie od
 * podanej komendy
 */

void parseInput();

#endif //GAMMA_INPUT_H
