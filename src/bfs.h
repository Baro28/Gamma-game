/** @file
 * Interfejs klasy przechowującej funkcje przeszukujące planszę.
 *
 * @author Bartosz Ławny <bl418396@students.mimuw.edu.pl>
 * @copyright Uniwersytet Warszawski
 * @date 17,05.2020
 */

#ifndef GAMMA_BFS_H
#define GAMMA_BFS_H

#include <stdbool.h>
#include "gamma.h"
#include "gamma_t.h"

/**
  * Struktura przechowująca współrzędne pola.
  */

typedef struct coordinates
{
    uint32_t x; ///< Współrzędna x
    uint32_t y; ///< Współrzędna y
}coords_t;

/** @brief Zlicza liczbę różnych obszarów sąsiadujących z podanym polem.
 * Najpierw sprawdza czy sąsiedzi należą do tego samego gracza.
 * Jeśli tak, zapisuje indeksy ich obszarów do tablicy.
 * Następnie zlicza ilość różnych niezerowych indeksów obszarów.
 * @param[in, out] g   – struktura przechowująca stan gry,
 * @param[in] player   – indeks gracza,
 * @param[in] x   – współrzędna x pola,
 * @param[in] y   – współrzędna y pola,
 * @return Zwraca liczbę różnych obszarów sąsiadujących z podanym polem.
 */

uint32_t count_areas(gamma_t *g, uint32_t player, uint32_t x, uint32_t y);

/** @brief Sprawdza czy jest możliwe przejście między dwoma polami w tym samym obszarze za pomocą bfsa
 * @param[in, out] g   – struktura przechowująca stan gry,
 * @param[in] player   – indeks gracza,
 * @param[in] begin   – współrzędne początku,
 * @param[in] end   – współrzędne końca,
 * @param[in] area_number  – indeks obszaru po którym będzie przechodzić,
 * @return Zwraca true jeśli udało się przejść pomiędzy tymi polami w jednym obszarze,
 * false w przeciwnym wypadku.
 */

bool bfs(gamma_t *g, uint32_t player, coords_t begin, coords_t end, uint32_t area_number);

/** @brief Aktualizuje indeksy obszaru w przypadku złączania obszarów
 * Najpierw sprawdza który z sąsiednich obszarów jest największy by
 * później przechodzić bfsem po pozostałych, na pewno mniejszych lub równych
 * obszarach i aktualizować ich indeks obszaru na indeks obszaru największego
 * obszaru.
 * @param[in, out] g   – struktura przechowująca stan gry,
 * @param[in] player   – indeks gracza,
 * @param[in] x  – współrzędna x pola,
 * @param[in] y  – współrzędna y pola,
 * @param[in] add  – flaga informująca czy zwiększać czy zmniejszać liczbę obszarów
 * należących do danego gracza.
 */

void gamma_update_area(gamma_t *g, uint32_t player, uint32_t x, uint32_t y, bool add);

#endif //GAMMA_BFS_H
