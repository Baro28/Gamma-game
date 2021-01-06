/** @file
 * Implementacja klasy przechowującej stan gry gamma
 *
 * @author Bartosz Ławny <barteklawny@gmail.com>
 * @copyright Uniwersytet Warszawski
 * @date 17.04.2020
 */

#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "gamma.h"
#include "bfs.h"
#include "gamma_t.h"

/** @brief Zwalnia podaną ilość wcześniejszych alokacji w gamma_new
 * Zwalnia alokacje w gamma_new w zależności od momentu błędu alokacji,
 * @param[in, out] game   – struktura przechowująca stan gry,
 * @param[in] how_many   – ile alokacji zwolnić,
 * @param[in] width   – szerokość planszy,
 * @param[in] players   – liczba graczy,
 */

static void free_previous_allocations(gamma_t *game, uint32_t how_many, uint32_t width, uint32_t players)
{
    if(how_many > 5)
    {
        for (uint32_t l = 0; l < players + 1; l++)
            free(game->player_info[l].area_number);
        for (uint32_t j = 2; j < players + 1; j++)
            free(game->player_info[j].area_size);
    }
    if(how_many > 4)
        free(game->player_info);
    if(how_many > 3)
        for(uint32_t k = 0; k < width; k++)
            free(((*(game->board))[k]));
    if(how_many > 2)
        free(*(game->board));
    if(how_many > 1)
        free(game->board);
    free(game);
}

gamma_t *gamma_new(uint32_t width, uint32_t height,
                   uint32_t players, uint32_t areas)
{
    if (width == 0 || height == 0 || players == 0 || areas == 0)
        return NULL;

    gamma_t *game = NULL;
    game = malloc(sizeof(*game));
    if(!game)
    {
        free_previous_allocations(game, 1, width, players);
        return NULL;
    }

    game->board = NULL;
    game->board = malloc(sizeof((game->board)));
    if(!game->board)
    {
        free_previous_allocations(game, 2, width, players);
        return NULL;
    }

    *(game->board) = NULL;
    *(game->board) = malloc(width * sizeof *(game->board));
    if(!(*(game->board)))
    {
        free_previous_allocations(game, 3, width, players);
        return NULL;
    }
    for (uint32_t i = 0; i < width; i++)
        ((*(game->board))[i]) = NULL;
    for (uint32_t i = 0; i < width; i++)
    {
        ((*(game->board))[i]) = malloc(height * sizeof ***(game->board));
        if(!((*(game->board))[i]))
            free_previous_allocations(game, 4, width, players);
    }
    for (uint32_t i = 0; i < width; i++)
    {
        for (uint32_t j = 0; j < height; j++)
        {
            ((*(game->board))[i][j]).owner = 0;
            ((*(game->board))[i][j]).area = 0;
            ((*(game->board))[i][j]).visited = false;
        }
    }
    game->player_info = NULL;
    game->player_info = malloc((players + 1) * sizeof(*(game->player_info)));
    if(!(game->player_info) || players + 1 == 0)
    {
        free_previous_allocations(game, 4, width, 0);
        return NULL;
    }
    for(uint32_t i = 0; i < players + 1; i++)
    {
        game->player_info[i].area_size = NULL;
        game->player_info[i].area_number = NULL;
    }

    for (uint32_t i = 0; i < (players + 1); i++)
    {
        game->player_info[i].area_size = malloc((areas + 9) * sizeof *(game->player_info->area_size));
        if(!game->player_info[i].area_size)
        {
            free_previous_allocations(game, 5, width, players);
            return NULL;
        }
        game->player_info[i].area_number = malloc((areas + 9) * sizeof *(game->player_info->area_number));
        if(!game->player_info[i].area_number)
        {
            free_previous_allocations(game, 5, width, players);
            return NULL;
        }
        game->player_info[i].busy_fields = 0;
        game->player_info[i].current_areas = 0;
        game->player_info[i].used_golden_move = false;
        game->player_info[i].free_adjacent_fields = 0;

        for (uint32_t j = 0; j < (areas + 9); j++)
        {
            (game->player_info[i]).area_size[j] = 0;
            (game->player_info[i]).area_number[j] = 0;
        }

    }

    game->max_areas = areas;
    game->players = players;
    game->max_width = width;
    game->max_height = height;
    game->free_fields = (uint64_t) width * height;

    return game;
}

void gamma_delete(gamma_t *g)
{
    if (!g)
        return;
    if (g->board)
    {
        for (uint32_t i = 0; i < g->max_width; i++)
        {
            free((*(g->board))[i]);
        }

        for (uint32_t i = 0; i < g->players + 1; i++)
        {
            free(g->player_info[i].area_number);
            free(g->player_info[i].area_size);
        }
        free(g->player_info);
        free(*(g->board));
        free(g->board);
    }
    free(g);
}

uint64_t gamma_busy_fields(gamma_t *g, uint32_t player)
{
    if (g == NULL || player == 0 || player > g->players)
        return 0;
    return g->player_info[player].busy_fields;
}

uint64_t gamma_free_fields(gamma_t *g, uint32_t player)
{
    if (g == NULL || player == 0 || player > g->players)
        return 0;

    if (g->player_info[player].current_areas == g->max_areas)
    {
        return g->player_info[player].free_adjacent_fields;
    }

    return g->free_fields;
}

/** @brief Sprawdza czy dwa pola mają ten sam numer obszaru a nie są w rzeczywistości w jednym obszarze
 * Najpierw sprawdza czy dwa pola mają ten sam numer,
 * jeśli tak to próbuje przejść między nimi bfsem.
 * Jeśli się nie udało to zwraca prawdę.
 * W przeciwnym wypadku fałsz.
 * @param[in, out] g   – struktura przechowująca stan gry,
 * @param[in] player   – indeks gracza,
 * @param[in] begin_x   – współrzędna x pierwszego pola,
 * @param[in] begin_y   – współrzędna y pierwszego pola,
 * @param[in] end_x   – współrzędna x drugiego pola,
 * @param[in] end_y   – współrzędna y drugiego pola,
 * @return True jeśli dwa pola mają ten sam numer obszaru a nie są w jednym obszarze,
 * w przeciwnym wypadku false.
 */

static bool same_area_unconnected(gamma_t *g, uint32_t player, uint32_t begin_x, uint32_t begin_y, uint32_t end_x, uint32_t end_y)
{
    coords_t begin, end;

    begin.x = begin_x;
    begin.y = begin_y;

    end.x = end_x;
    end.y = end_y;

    if((*(g->board))[begin_x][begin_y].area != (*(g->board))[end_x][end_y].area)
        return false;

    return !bfs(g, player, begin, end, (*(g->board))[end_x][end_y].area);
}

/** @brief Oblicza liczbę różnych sąsiednich obszarów przy złotym ruchu
 * Najpierw sprawdza które krawędzie należą do tego samego gracza.
 * Potem tymczasowo ustawia właściciela pola na nowego gracza i
 * próbuje przejść bfsem z każdej posiadanej krawędzi do każdej innej.
 * Jeśli się nie da to oznacza że złoty ruch rozspójni obszar więc
 * zwiększa licznik obszarów.
 * @param[in, out] g   – struktura przechowująca stan gry,
 * @param[in] player   – indeks starego gracza,
 * @param[in] new_player  – indeks nowego gracza,
 * @param[in] x   – współrzędna x pola,
 * @param[in] y   – współrzędna y pola,
 * @return Liczba nowych obszarów po złotym ruchu.
 */

static uint32_t golden_count_areas(gamma_t *g, uint32_t player, uint32_t new_player, uint32_t x, uint32_t y)
{
    if(count_areas(g, player, x, y) != 1)
        return 0;

    bool up = false, left = false, right = false, down = false;
    uint32_t counter = 0;
    if (y > 0 && (*(g->board))[x][y - 1].owner == player)
    {
        down = true;
    }
    if (y < g->max_height - 1 && (*(g->board))[x][y + 1].owner == player)
    {
        up = true;
    }
    if (x > 0 && ((*(g->board))[x - 1][y]).owner == player)
    {
        left = true;
    }
    if (x < g->max_width - 1 && (*(g->board))[x + 1][y].owner == player)
    {
        right = true;
    }

    uint32_t original_owner = (*(g->board))[x][y].owner;
    (*(g->board))[x][y].owner = new_player;

    if (up && left && same_area_unconnected(g, player, x, y + 1, x - 1, y))
        counter++;

    if (up && right && same_area_unconnected(g, player, x, y + 1, x + 1, y))
        counter++;

    if (up && down && same_area_unconnected(g, player, x, y + 1, x, y - 1))
        counter++;

    if (left && right && same_area_unconnected(g, player, x - 1, y, x + 1, y))
        counter++;

    if (left && down && same_area_unconnected(g, player, x - 1, y, x, y - 1))
        counter++;

    if (right && down && same_area_unconnected(g, player, x + 1, y, x, y - 1))
        counter++;

    (*(g->board))[x][y].owner = original_owner;
    return counter;
}

/** @brief Sprawdza czy pole ma sąsiada należącego do tego samego gracza
 * @param[in, out] g   – struktura przechowująca stan gry,
 * @param[in] player   – indeks gracza,
 * @param[in] x   – współrzędna x pola,
 * @param[in] y   – współrzędna y pola,
 * @return Zwraca true jeśli ma sąsiada należącego do tego samego gracza,
 * false w przeciwnym wypadku.
 */

static bool gamma_single_area(gamma_t *g, uint32_t player, uint32_t x, uint32_t y)
{
    if ((x < g->max_width - 1 && (*(g->board))[x + 1][y].owner == player)
        || (x > 0 && ((*(g->board))[x - 1][y]).owner == player)
        || (y < g->max_height - 1 && (*(g->board))[x][y + 1].owner == player)
        || (y > 0 && (*(g->board))[x][y - 1].owner == player)
            )
        return false;
    return true;
}

/** @brief Znajduje najmniejszy indeks o wartości zero nowego obszaru
 * @param[in, out] g   – struktura przechowująca stan gry,
 * @param[in] player   – indeks gracza,
 * @return Zwraca znaleziony indeks.
 */

static uint32_t next_free_area_number(gamma_t *g, uint32_t player)
{
    uint32_t i = 1;
    while (g->player_info[player].area_number[i] != 0)
        i++;
    return i;
}

/** @brief Sprawdza czy ruch jest legalny oraz przygotowuje pole jeśli tak
 * Jeśli ruch jest legalny to aktualizuje informacje o obszarach gracza
 * lub aktualizuje obszary jeśli nowe pole ma już sąsiada
 * @param[in, out] g   – struktura przechowująca stan gry,
 * @param[in] player   – indeks gracza,
 * @param[in] x   – współrzędna x pola,
 * @param[in] y   – współrzędna y pola,
 * @return Zwraca true jeśli ruch jest legalny,
 * false w przeciwnym wypadku.
 */

static bool gamma_move_possible(gamma_t *g, uint32_t player, uint32_t x, uint32_t y)
{
    if (x >= g->max_width || y >= g->max_height || player > g->players)
        return false;

    if (((*(g->board))[x][y]).owner != 0)
        return false;

    if (gamma_single_area(g, player, x, y))
    {
        if (g->player_info[player].current_areas < g->max_areas)
        {

            (g->player_info[player].current_areas)++;
            uint32_t area_number = next_free_area_number(g, player);
            (*(g->board))[x][y].area = area_number;
            g->player_info[player].area_number[area_number] = area_number;
            g->player_info[player].area_size[area_number] += 1;

            return true;
        }
        return false;
    }
    else
    {
        gamma_update_area(g, player, x, y, false);
    }

    return true;
}

/** @brief Resetuje flagę odwiedzenia w sąsiednich polach
 * @param[in, out] g   – struktura przechowująca stan gry,
 * @param[in] x   – współrzędna x pola,
 * @param[in] y   – współrzędna y pola,
 */

static void reset_visited_neighbours(gamma_t *g, uint32_t x, uint32_t y)
{
    (*(g->board))[x][y].visited = false;

    if (x > 0)
        (*(g->board))[x - 1][y].visited = false;

    if (y > 0)
        (*(g->board))[x][y - 1].visited = false;

    if (x < g->max_width - 1)
        (*(g->board))[x + 1][y].visited = false;

    if (y < g->max_height - 1)
        (*(g->board))[x][y + 1].visited = false;
}

/** @brief Sprawdza czy gracz występuje po raz pierwszy w obiegu po krawędziach
 * Zależnie od parametru @p direction sprawdza czy gracz wystąpił już w obiegu po krawędziach
 * w kierunku przeciwnym do kierunku wskazówek zegara zaczynając od lewej krawędzi.
 * Używana by nie duplikować operacji dotyczących tego samego pola.
 * @param[in] neighbours   – tablica zawierająca indeksy sąsiadów,
 * @param[in] direction   – parametr od którego sąsiada sprawdzamy,
 * @return Zwraca true jeśli gracz występuje po raz pierwszy,
 * false w przeciwnym wypadku.
 */

static bool first_player_ocurrence(uint32_t *neighbours, uint32_t direction)
{
    if (direction == 0)
        return true;
    if (direction == 1)
    {
        if (neighbours[1] == neighbours[0])
            return false;
    }
    if (direction == 2)
    {
        if (neighbours[2] == neighbours[1] || neighbours[2] == neighbours[0])
            return false;
    }
    if (direction == 3)
    {
        if (neighbours[3] == neighbours[2] || neighbours[3] == neighbours[1] || neighbours[3] == neighbours[0])
            return false;
    }
    return true;
}

/** @brief Inicjalizuje tablicę zawierającą indeksy graczy posiadajacych sąsiednie pola
 * @param[in, out] g   – struktura przechowująca stan gry,
 * @param[out] neighbours   – tablica którą inicjalizuje,
 * @param[in] x   – współrzędna x pola,
 * @param[in] y   – współrzędna y pola,
 */

static void initialize_neighbours(gamma_t *g, uint32_t *neighbours, uint32_t x, uint32_t y)
{
    for (uint32_t i = 0; i < 4; i++)
        neighbours[i] = 0;
    if (x > 0)
        neighbours[0] = (*(g->board))[x - 1][y].owner;
    if (y > 0)
        neighbours[1] = (*(g->board))[x][y - 1].owner;
    if (x < g->max_width - 1)
        neighbours[2] = (*(g->board))[x + 1][y].owner;
    if (y < g->max_height - 1)
        neighbours[3] = (*(g->board))[x][y + 1].owner;
}

/** @brief Sprawdza czy niezajęte pole ma tylko jednego sąsiada o danym graczu
 * Aby uniknąć duplikowania operacji dla tego samego niezajętego pola
 * sprawdza czy to pole ma tylko jednego sąsiada o właścicielu pod indeksem @p player
 * @param[in, out] g   – struktura przechowująca stan gry,
 * @param[in] player   – indeks gracza,
 * @param[in] x   – współrzędna x pola,
 * @param[in] y   – współrzędna y pola,
 * @return Zwraca true jeśli licznik sąsiadów o indeksie @p player jest równy 1,
 * false w przeciwnym wypadku.
 */

static bool is_single_neighbour(gamma_t *g, uint32_t player, uint32_t x, uint32_t y)
{
    uint32_t counter = 0;
    if (x > 0 && ((*(g->board))[x - 1][y].owner == player && ((*(g->board))[x - 1][y].visited == false)))
    {
        counter++;
    }
    if (y > 0 && ((*(g->board))[x][y - 1].owner == player && ((*(g->board))[x][y - 1].visited == false)))
    {
        counter++;
    }
    if (x < g->max_width - 1
        && ((*(g->board))[x + 1][y].owner == player && ((*(g->board))[x + 1][y].visited == false)))
    {
        counter++;
    }
    if (y < g->max_height - 1
        && ((*(g->board))[x][y + 1].owner == player && ((*(g->board))[x][y + 1].visited == false)))
    {
        counter++;
    }
    return (counter == 1);
}

/** @brief Resetuje flagę odwiedzenia w danym polu i wszystkich jego sąsiadach
 * @param[in, out] g   – struktura przechowująca stan gry,
 * @param[in] x   – współrzędna x pola,
 * @param[in] y   – współrzędna y pola,
 */

static void reset_all_directions(gamma_t *g, uint32_t x, uint32_t y)
{

    reset_visited_neighbours(g, x, y);
    (*(g->board))[x][y].visited = false;

    if (x > 0)
        reset_visited_neighbours(g, x - 1, y);
    if (x < g->max_width - 1)
        reset_visited_neighbours(g, x + 1, y);
    if (y > 0)
        reset_visited_neighbours(g, x, y - 1);
    if (y < g->max_height - 1)
        reset_visited_neighbours(g, x, y + 1);
}

/** @brief Aktualizuję liczbę sąsiednich niezajętych pól przy złotym ruchu
 * Najpierw zmniejsza liczbę tych pól dla starego gracza w zaleźności od
 * niezajętych pól których jest jedynym sąsiadem.
 * Później ustawia właściciela pola na nowego gracza i analogicznie
 * zwiększa liczbę tych pól dla nowego gracza.
 * @param[in, out] g   – struktura przechowująca stan gry,
 * @param[in] old_player   – indeks starego gracza,
 * @param[in] new_player   – indeks nowego gracza,
 * @param[in] x   – współrzędna x pola,
 * @param[in] y   – współrzędna y pola,
 */

static void golden_update_adjacent_fields(gamma_t *g, uint32_t old_player, uint32_t new_player, uint32_t x, uint32_t y)
{
    if (x > 0 && (*(g->board))[x - 1][y].owner == 0 && is_single_neighbour(g, old_player, x - 1, y))
        (g->player_info[old_player].free_adjacent_fields)--;

    if (y > 0 && (*(g->board))[x][y - 1].owner == 0 && is_single_neighbour(g, old_player, x, y - 1))
        (g->player_info[old_player].free_adjacent_fields)--;

    if (x < g->max_width - 1 && (*(g->board))[x + 1][y].owner == 0 && is_single_neighbour(g, old_player, x + 1, y))
        (g->player_info[old_player].free_adjacent_fields)--;

    if (y < g->max_height - 1 && (*(g->board))[x][y + 1].owner == 0 && is_single_neighbour(g, old_player, x, y + 1))
        (g->player_info[old_player].free_adjacent_fields)--;

    reset_all_directions(g, x, y);

    (*(g->board))[x][y].owner = new_player;

    if (x > 0 && (*(g->board))[x - 1][y].owner == 0 && is_single_neighbour(g, new_player, x - 1, y))
        (g->player_info[new_player].free_adjacent_fields)++;

    if (y > 0 && (*(g->board))[x][y - 1].owner == 0 && is_single_neighbour(g, new_player, x, y - 1))
        (g->player_info[new_player].free_adjacent_fields)++;

    if (x < g->max_width - 1 && (*(g->board))[x + 1][y].owner == 0 && is_single_neighbour(g, new_player, x + 1, y))
        (g->player_info[new_player].free_adjacent_fields)++;

    if (y < g->max_height - 1 && (*(g->board))[x][y + 1].owner == 0 && is_single_neighbour(g, new_player, x, y + 1))
        (g->player_info[new_player].free_adjacent_fields)++;

    reset_all_directions(g, x, y);
}

/** @brief Aktualizuję liczbę sąsiednich niezajętych pól przy zwykłym ruchu
 * Dla każdej krawędzi odpowiednio zwiększa liczbę tych pól jeśli
 * sąsiad jest niezajętym polem oraz ma tylko jednego sąsiada o danym graczu.
 * Jeśli sąsiednie pole jest niezajęte to zmniejsza liczbę tych pól
 * uważając przy tym by nie duplikować operacji.
 * @param[in, out] g   – struktura przechowująca stan gry,
 * @param[in] player   – indeks gracza,
 * @param[in] x   – współrzędna x pola,
 * @param[in] y   – współrzędna y pola,
 */

static void update_adjacent_fields(gamma_t *g, uint32_t player, uint32_t x, uint32_t y)
{
    uint32_t neighbours[4];
    initialize_neighbours(g, neighbours, x, y);

    if (x > 0 && (*(g->board))[x - 1][y].owner == 0
        && is_single_neighbour(g, player, x - 1, y))
        (g->player_info[player].free_adjacent_fields)++;

    else if (x > 0 && (g->player_info[(*(g->board))[x - 1][y].owner].free_adjacent_fields) > 0
             && first_player_ocurrence(neighbours, 0))
        (g->player_info[(*(g->board))[x - 1][y].owner].free_adjacent_fields)--;

    if (y > 0 && (*(g->board))[x][y - 1].owner == 0
        && is_single_neighbour(g, player, x, y - 1))
        (g->player_info[player].free_adjacent_fields)++;

    else if (y > 0 && (g->player_info[(*(g->board))[x][y - 1].owner].free_adjacent_fields) > 0
             && first_player_ocurrence(neighbours, 1))
        (g->player_info[(*(g->board))[x][y - 1].owner].free_adjacent_fields)--;

    if (x < g->max_width - 1 && (*(g->board))[x + 1][y].owner == 0
        && is_single_neighbour(g, player, x + 1, y))
        (g->player_info[player].free_adjacent_fields)++;

    else if (x < g->max_width - 1 && (g->player_info[(*(g->board))[x + 1][y].owner].free_adjacent_fields) > 0
             && first_player_ocurrence(neighbours, 2))
        (g->player_info[(*(g->board))[x + 1][y].owner].free_adjacent_fields)--;

    if (y < g->max_height - 1 && (*(g->board))[x][y + 1].owner == 0
        && is_single_neighbour(g, player, x, y + 1))
        (g->player_info[player].free_adjacent_fields)++;

    else if (y < g->max_height - 1 && (g->player_info[(*(g->board))[x][y + 1].owner].free_adjacent_fields) > 0
             && first_player_ocurrence(neighbours, 3))
        (g->player_info[(*(g->board))[x][y + 1].owner].free_adjacent_fields)--;

    reset_all_directions(g, x, y);
}

bool gamma_move(gamma_t *g, uint32_t player, uint32_t x, uint32_t y)
{
    if (g == NULL || player == 0 || player > g->players || x >= g->max_width || y >= g->max_height)
        return false;

    if (!gamma_move_possible(g, player, x, y))
        return false;
    (((*(g->board))[x][y]).owner) = player;
    (g->free_fields)--;
    (g->player_info[player].busy_fields)++;
    update_adjacent_fields(g, player, x, y);
    return true;
}

/** @brief Wstawia każdą cyfrę podanej liczby do stringa
 * Jeśli liczba graczy przekracza 9 to liczby są wstawiane w
 * nawiasach kwadratowych dla czytelności przy wyświetlaniu.
 * @param[in, out] board   – string do którego wstawia liczbę,
 * @param[in, out] i   – pozycja w stringu,
 * @param[in] number   – liczba którą wstawia do stringa,
 * @param[in] no_brackets – flaga mówiąca czy wstawiać nawiasy.
 */

static void parse_number(char *board, uint64_t *i, uint32_t number, bool no_brackets)
{
    uint32_t length = log_10(number);
    uint32_t *array = malloc(length * sizeof(uint32_t));
    checkNull(array);

    uint32_t index = length;

    while (number > 0)
    {
        uint32_t digit = number % 10;
        array[--index] = digit;
        number /= 10;
    }
    if (no_brackets)
        (*i)--;
    else
        board[*i] = ' ';
    for (uint32_t j = 0; j < length; j++)
    {
        (*i)++;
        board[*i] = array[j] + '0';

    }

    if (length == 0)
    {
        (*i)++;
        board[*i] = '.';
    }

    if (!no_brackets)
    {
        (*i)++;
        board[*i] = ' ';
    }
    (*i)++;

    free(array);
}

char *gamma_board(gamma_t *g)
{
    if(g == NULL)
        return NULL;
    uint32_t width_of_field = log_10(g->players) + 2;
    if (g->players < 10)
        width_of_field -= 2;

    char *board = malloc((g->max_height * (g->max_width + 1) * width_of_field + 1) * sizeof(char));
    if(!board)
        return NULL;

    uint64_t x = 0;

    bool no_brackets = false;
    if (g->players < 10)
        no_brackets = true;

    for (uint32_t i = 0; i < g->max_height; i++)
    {
        for (uint32_t j = 0; j < g->max_width; j++)
        {
            parse_number(board, &x, ((*(g->board))[j][g->max_height - 1 - i]).owner, no_brackets);
        }
        board[x] = '\n';
        x++;
    }

    board[x] = '\0';
    return board;
}

/** @brief Sprawdza czy dany gracz może wykonać złoty ruch na podanym polu
 * Sprawdza czy dane pole nie jest puste ani zajęte przez tego samego gracza.
 * Potem w zaleźności od liczby obszarów starego i nowego gracza sprawdza czy
 * złoty ruch nie sprawi, że liczba obszarów przekroczy maksymalną.
 * @param[in, out] g   – struktura przechowująca stan gry,
 * @param[in] new_player   – numer nowego gracza,
 * @param[in] x   – współrzędna x pola,
 * @param[in] y   – współrzędna y pola,
 * @return Zwraca true jeśli dany gracz może wykonać ruch na podanym polu,
 * w przeciwnym wypadku false.
 */

static bool gamma_golden_possible_on_field(gamma_t *g, uint32_t new_player, uint32_t x, uint32_t y)
{
    uint32_t old_player = ((*(g->board))[x][y]).owner;
    uint32_t areas = golden_count_areas(g, old_player, new_player, x, y);

    if(old_player == new_player || old_player == 0)
        return false;

    if(gamma_single_area(g, new_player, x, y))
    {
        if(g->player_info[new_player].current_areas >= g->max_areas)
        {
            return false;
        }
        else if (!gamma_single_area(g, old_player, x, y)
                && g->player_info[old_player].current_areas + areas > g->max_areas)
            return false;
    }
    else
    {
        if (!gamma_single_area(g, old_player, x, y)
            &&
            g->player_info[old_player].current_areas + areas > g->max_areas)
            return false;
    }

    return true;
}

bool gamma_golden_possible(gamma_t *g, uint32_t player)
{
    if(g == NULL || player > g->players)
        return false;
    uint64_t board_size = (uint64_t) g->max_height * g->max_width;

    if (g->player_info[player].used_golden_move == false
        && g->free_fields + g->player_info[player].busy_fields != board_size)
    {
        for(uint32_t i = 0; i < g->max_width; i++)
        {
            for(uint32_t j = 0; j < g->max_height; j++)
            {
                if(gamma_golden_possible_on_field(g, player, i, j))
                {
                    return true;
                }
            }
        }
    }

    return false;
}

bool gamma_golden_move(gamma_t *g, uint32_t player, uint32_t x, uint32_t y)
{
    if (g == NULL || player == 0 || player > g->players || x >= g->max_width || y >= g->max_height)
        return false;

    if (!gamma_golden_possible(g, player))
        return false;

    uint32_t old_player = (((*(g->board))[x][y]).owner);

    if (old_player == 0 || old_player == player)
        return false;

    if (gamma_single_area(g, player, x, y))
    {
        if (g->player_info[player].current_areas < g->max_areas)
        {
            if (gamma_single_area(g, old_player, x, y))
            {
                golden_update_adjacent_fields(g, old_player, player, x, y);
                (g->player_info[player].current_areas)++;
                (g->player_info[old_player].current_areas)--;
                (((*(g->board))[x][y]).owner) = player;
                (((*(g->board))[x][y]).area) = next_free_area_number(g, player);
                (g->player_info[player].busy_fields)++;
                (g->player_info[old_player].busy_fields)--;
                g->player_info[player].area_number[(((*(g->board))[x][y]).area)] = (((*(g->board))[x][y]).area);
                g->player_info[player].area_size[(((*(g->board))[x][y]).area)] = 1;
                g->player_info[player].used_golden_move = true;
                return true;
            }
            else
            {
                uint32_t areas = golden_count_areas(g, old_player, player, x, y);
                if (!gamma_single_area(g, old_player, x, y)
                    && g->player_info[old_player].current_areas + areas >
                       g->max_areas)
                    return false;
                golden_update_adjacent_fields(g, old_player, player, x, y);
                gamma_update_area(g, old_player, x, y, true);
                if (areas == 1)
                    areas++;
                if (count_areas(g, old_player, x, y) != 1
                    || areas != 0)
                    (g->player_info[old_player].current_areas) += areas - 1;
                if (gamma_single_area(g, old_player, x, y))
                    (g->player_info[old_player].current_areas)--;
                (((*(g->board))[x][y]).owner) = player;
                (g->player_info[player].current_areas)++;
                (g->player_info[player].busy_fields)++;
                (g->player_info[old_player].busy_fields)--;
                g->player_info[player].area_number[(((*(g->board))[x][y]).area)] = (((*(g->board))[x][y]).area);
                g->player_info[player].area_size[(((*(g->board))[x][y]).area)] = 1;
                g->player_info[player].used_golden_move = true;
            }
            return true;
        }
        return false;
    }
    else
    {
        uint32_t areas = golden_count_areas(g, old_player, player, x, y);
        if (!gamma_single_area(g, old_player, x, y)
            &&
            g->player_info[old_player].current_areas + areas > g->max_areas)
            return false;
        else
        {
            golden_update_adjacent_fields(g, old_player, player, x, y);
            gamma_update_area(g, player, x, y, false);
            if (areas == 1)
                areas++;
            if (count_areas(g, old_player, x, y) != 1
                || areas != 0)
                (g->player_info[old_player].current_areas) += areas - 1;
            (((*(g->board))[x][y]).owner) = player;
            (g->player_info[player].busy_fields)++;
            (g->player_info[old_player].busy_fields)--;
            g->player_info[player].area_number[(((*(g->board))[x][y]).area)] = (((*(g->board))[x][y]).area);
            g->player_info[player].area_size[(((*(g->board))[x][y]).area)] = 1;
            g->player_info[player].used_golden_move = true;
            return true;
        }
    }
    return true;
}
