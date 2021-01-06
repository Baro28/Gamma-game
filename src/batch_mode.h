/** @file
 * Interfejs klasy obsługującej tryb wsadowy
 *
 * @author Bartosz Ławny <bl418396@students.mimuw.edu.pl>
 * @copyright Uniwersytet Warszawski
 * @date 17.05.2020
 */

#ifndef GAMMA_BATCH_MODE_H
#define GAMMA_BATCH_MODE_H
#include "gamma.h"

/** @brief Główna funkcja obsługująca tryb wsadowy
 * Funkcja zawiera pętlę obsługującą input w trybie wsadowym.
 */

void batch_mode(gamma_t *g, uint32_t line);

#endif //GAMMA_BATCH_MODE_H
