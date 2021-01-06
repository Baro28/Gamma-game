/** @file
 * Interfejs klasy przechowującej pomocnicze funkcje i struktury
 *
 * @author Bartosz Ławny <bl418396@students.mimuw.edu.pl>
 * @copyright Uniwersytet Warszawski
 * @date 17.05.2020
 */

#ifndef GAMMA_GAMMA_T_H
#define GAMMA_GAMMA_T_H

#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>

/** @brief Oblicza logarytm dziesiętny z liczby
 * @param[in] x - liczba z której oblicza logarytm.
 * @return Zwraca obliczony logarytm.
 */

uint32_t log_10(uint32_t x);

/** @brief Sprawdza czy podany wskaźnik jest NULLem
 * Jeśli wskaźnik zwrócony przez malloc jest NULLem
 * to wychodzi z kodem błędu 1.
 */

void checkNull(void *pointer);


/**
  * Struktura przechowująca informacje o pojedynczym polu planszy
  */

typedef struct field
{
    uint32_t owner; ///< numer gracza posiadające to pole (domyślnie 0)
    uint32_t area; ///< numer obszaru do którego pole należy (domyślnie 0)
    bool visited; ///< pomocnicza flaga używana przy przechodzeniu za pomocą bfs
} field_t;

/**
  * Struktura przechowująca informacje o graczu
  */

typedef struct player
{
    uint64_t free_adjacent_fields; ///< liczba dostępnych pól gdy osiągnięto maksymalną liczbę obszarów
    bool used_golden_move; ///< flaga informująca czy gracz użył już swojego złotego ruchu
    uint32_t current_areas; ///< liczba posiadanych różnych spójnych obszarów
    uint32_t *area_number; ///< tablica przechowująca unikalne numery różnych obszarów
    uint64_t *area_size; ///< tablica przechowująca rozmiar obszaru o numerze i pod indeksem i
    uint64_t busy_fields; ///< liczba zajętych przez gracza pól
} player_t;

/**
  * Struktura przechowująca stan gry
  */

struct gamma
{
    uint32_t max_width; ///< szerokość planszy
    uint32_t max_height; ///< wysokość planszy
    uint32_t max_areas; ///< maksymalna liczba spójnych obszarów posiadanych przez jednego gracza
    uint32_t players; ///< liczba graczy
    uint64_t free_fields; ///< liczba wolnych pól na planszy
    player_t *player_info; ///< tablica struktur przechowujących informacje o graczu i pod indeksem i
    field_t ***board; ///< wskaźnik do dwuwymiarowej tablicy struktur pól planszy
};


#endif //GAMMA_GAMMA_T_H
