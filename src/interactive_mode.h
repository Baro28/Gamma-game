/** @file
 * Interfejs klasy obsługującej tryb interaktywny
 *
 * @author Bartosz Ławny <bl418396@students.mimuw.edu.pl>
 * @copyright Uniwersytet Warszawski
 * @date 17.05.2020
 */

#ifndef GAMMA_INTERACTIVE_MODE_H
#define GAMMA_INTERACTIVE_MODE_H

#include <inttypes.h>
#include "gamma.h"

/** @brief Główna funkcja obsługująca tryb interaktywny
 * Funkcja zawiera pętlę obsługującą input w trybie interaktywnym.
 */

void interactive_mode(gamma_t *g);

#endif //GAMMA_INTERACTIVE_MODE_H
