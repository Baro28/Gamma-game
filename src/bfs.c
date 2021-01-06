/** @file
 * Implementacja klasy przechowującej funkcje przeszukujące planszę.
 *
 * @author Bartosz Ławny <bl418396@students.mimuw.edu.pl>
 * @copyright Uniwersytet Warszawski
 * @date 17,05.2020
 */

#include <inttypes.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include "bfs.h"

/**
  * Struktura implementująca listę.
  */

typedef struct list
{
    coords_t data; ///< Wartość elementu
    struct list *next; ///< Wskaźnik na następny element w liście
} list_t;

/**
  * Struktura implementująca kolejkę.
  */

typedef struct queue
{
    list_t *begin; ///< Wskaźnik na początek kolejki
    list_t *end; ///< Wskaźnik na koniec kolejki
} queue_t;

/** @brief Sprawdza czy kolejka jest pusta
 * @param[in] q   – wskaźnik na kolejkę
 * @return Zwraca true jeśli q jest NULLem, false w przeciwnym wypadku.
 */

static bool queue_empty(queue_t *q)
{
    return (q->begin == NULL);
}

/** @brief Inicjalizuje kolejkę
 * Ustawia początek kolejki na NULL.
 * @param[in] q   – wskaźnik na kolejkę
 */

static void queue_initialize(queue_t *q)
{
    q->begin = NULL;
}


/** @brief Wstawia do kolejki ZA podany element
 * Alokuje pamięć na nowy element i wstawia go do kolejki za element @p x.
 * @param[in] l   – wskaźnik na listę,
 * @param[in] x  – element za który wstawia.
 */
static void queue_insert_behind(list_t *l, coords_t x)
{
    list_t *temp = malloc(sizeof(list_t));
    checkNull(temp);
    temp->next = l->next;
    temp->data = x;
    l->next = temp;
}

/** @brief Wstawia do kolejki podany element
 * Alokuje pamięć jeśli kolejka jest pusta lub wstawia za ostatni element jeśli nie jest pusta.
 * @param[in] q   – wskaźnik na kolejkę
 * @param[in] x  – element który wstawia.
 */

static void queue_insert(queue_t *q, coords_t x)
{
    if (q->begin == NULL)
    {
        q->begin = malloc(sizeof(list_t));
        checkNull(q->begin);
        q->begin->data = x;
        q->end = q->begin;
    }
    else
    {
        queue_insert_behind(q->end, x);
        q->end = q->end->next;
    }
}

/** @brief Pobiera pierwszy element z kolejki
 * Alokuje pamięć jeśli kolejka jest pusta lub wstawia za ostatni element jeśli nie jest pusta.
 * @param[in] q   – wskaźnik na kolejkę
 * @return Zwraca pierwszy element z kolejki
 */

static coords_t queue_get(queue_t *q)
{
    list_t *temp = q->begin;
    coords_t x = q->begin->data;
    if (q->begin == q->end)
        q->begin = NULL;
    else
        q->begin = q->begin->next;
    free(temp);
    return x;
}

/** @brief Zwalnia całą kolejkę
 * @param[in] q   – wskaźnik na kolejkę
 */

static void queue_free(queue_t *q)
{
    while (!queue_empty(q))
        queue_get(q);
}

/** @brief Sprawdza czy podane pole należy do tego samego gracza
 * @param[in, out] g   – struktura przechowująca stan gry,
 * @param[in] player   – indeks gracza,
 * @param[in] x   – współrzędna x pola,
 * @param[in] y   – współrzędna y pola,
 */

static bool check_if_same_player(gamma_t *g, uint32_t player, uint32_t x, uint32_t y)
{
    if ((*(g->board))[x][y].visited == false
        && (*(g->board))[x][y].owner == player)
        return true;
    return false;
}

/** @brief Zwraca większą z dwóch podanych liczb
 * @param[in] a   – pierwsza liczba,
 * @param[in] b   – druga liczba,
 * @return Zwraca a jeśli a > b lub b w przeciwnym wypadku.
 */

static uint32_t max(uint32_t a, uint32_t b)
{
    return (a > b ? a : b);
}

/** @brief Funkcja pomocnicza do wstawiania koordynatów pola do kolejki bfsa
 * @param[in, out] q   – wskaźnik na kolejkę,
 * @param[in] x   – współrzędna x pola,
 * @param[in] y   – współrzędna y pola,
 */

static void queue_insert_field(queue_t *q, uint32_t x, uint32_t y)
{
    coords_t *neighbour = malloc(sizeof(coords_t));
    checkNull(neighbour);
    (*neighbour).x = x;
    (*neighbour).y = y;
    queue_insert(q, *neighbour);
    free(neighbour);
}

/** @brief Przechodzi bfsem pola resetując przy tym flagi odwiedzenia
 * @param[in, out] g   – struktura przechowująca stan gry,
 * @param[in] coordinates   – współrzędne pola od którego zaczyna resetowanie,
 */

static void reset_visited(gamma_t *g, coords_t coordinates)
{
    queue_t *q = malloc(sizeof(queue_t));
    checkNull(q);
    queue_initialize(q);
    queue_insert(q, coordinates);

    (*(g->board))[coordinates.x][coordinates.y].visited = false;

    while (!queue_empty(q))
    {
        coords_t coords = queue_get(q);
        uint32_t x = coords.x;
        uint32_t y = coords.y;
        if (x > 0 && (*(g->board))[x - 1][y].visited == true)
        {
            (*(g->board))[x - 1][y].visited = false;
            queue_insert_field(q, x - 1, y);
        }

        if (x < g->max_width - 1 && (*(g->board))[x + 1][y].visited == true)
        {
            (*(g->board))[x + 1][y].visited = false;
            queue_insert_field(q, x + 1, y);
        }

        if (y > 0 && (*(g->board))[x][y - 1].visited == true)
        {
            (*(g->board))[x][y - 1].visited = false;
            queue_insert_field(q, x, y - 1);
        }

        if (y < g->max_height - 1 && (*(g->board))[x][y + 1].visited == true)
        {
            (*(g->board))[x][y + 1].visited = false;
            queue_insert_field(q, x, y + 1);
        }
    }
    free(q);
}

uint32_t count_areas(gamma_t *g, uint32_t player, uint32_t x, uint32_t y)
{
    uint32_t up = 0, left = 0, right = 0, down = 0;
    if (x < g->max_width - 1 && (*(g->board))[x + 1][y].owner == player)
        right = (*(g->board))[x + 1][y].area;

    if (x > 0 && (*(g->board))[x - 1][y].owner == player)
        left = (*(g->board))[x - 1][y].area;

    if (y < g->max_height - 1 && (*(g->board))[x][y + 1].owner == player)
        up = (*(g->board))[x][y + 1].area;

    if (y > 0 && (*(g->board))[x][y - 1].owner == player)
        down = (*(g->board))[x][y - 1].area;

    uint32_t counter = 1;
    uint32_t array[4] = {up, left, right, down};

    for (uint32_t i = 0; i < 4; i++)
        if (array[i] == 0)
        {
            counter--;
            break;
        }

    for (uint32_t i = 1; i < 4; i++)
    {
        uint32_t j = 0;
        for (j = 0; j < i; j++)
            if (array[i] == array[j])
                break;

        if (i == j)
            counter++;
    }

    return counter;
}

bool bfs(gamma_t *g, uint32_t player, coords_t begin, coords_t end, uint32_t area_number)
{
    if (begin.x == end.x && begin.y == end.y)
        return true;
    queue_t *q = malloc(sizeof(queue_t));
    checkNull(q);
    queue_initialize(q);
    queue_insert(q, begin);

    (*(g->board))[begin.x][begin.y].visited = true;

    while (!queue_empty(q))
    {
        coords_t coords = queue_get(q);
        uint32_t x = coords.x;
        uint32_t y = coords.y;
        if (x > 0 && (*(g->board))[x - 1][y].visited == false
            && (*(g->board))[x - 1][y].owner == player && (*(g->board))[x - 1][y].area == area_number)
        {
            if (x - 1 == end.x && y == end.y)
            {
                queue_free(q);
                free(q);
                reset_visited(g, begin);
                return true;
            }
            (*(g->board))[x - 1][y].visited = true;
            queue_insert_field(q, x - 1, y);
        }

        if (x < g->max_width - 1 && (*(g->board))[x + 1][y].visited == false
            && (*(g->board))[x + 1][y].owner == player && (*(g->board))[x + 1][y].area == area_number)
        {
            if (x + 1 == end.x && y == end.y)
            {
                queue_free(q);
                free(q);
                reset_visited(g, begin);
                return true;
            }
            (*(g->board))[x + 1][y].visited = true;
            queue_insert_field(q, x + 1, y);
        }

        if (y > 0 && (*(g->board))[x][y - 1].visited == false
            && (*(g->board))[x][y - 1].owner == player && (*(g->board))[x][y - 1].area == area_number)
        {
            if (x == end.x && y - 1 == end.y)
            {
                queue_free(q);
                free(q);
                reset_visited(g, begin);
                return true;
            }
            (*(g->board))[x][y - 1].visited = true;
            queue_insert_field(q, x, y - 1);
        }

        if (y < g->max_height - 1 && (*(g->board))[x][y + 1].visited == false
            && (*(g->board))[x][y + 1].owner == player && (*(g->board))[x][y + 1].area == area_number)
        {
            if (x == end.x && y + 1 == end.y)
            {
                queue_free(q);
                free(q);
                reset_visited(g, begin);
                return true;
            }
            (*(g->board))[x][y + 1].visited = true;
            queue_insert_field(q, x, y + 1);
        }
    }
    reset_visited(g, begin);
    free(q);
    return false;
}

/** @brief Ustawia indeksy obszaru na nowy w przypadku złączania obszarów
 * Przechodzi za pomocą bfsa dany obszar i zmienia wartość area w każdym z napotkanych pól.
 * @param[in, out] g   – struktura przechowująca stan gry,
 * @param[in] coordinates   – współrzędne początkowego pola,
 * @param[in] player   – indeks gracza,
 * @param[in] new_area_number  – indeks nowego obszaru,
 */

static void set_new_area_number(gamma_t *g, coords_t coordinates, uint32_t player, uint32_t new_area_number)
{
    if (coordinates.x >= g->max_width || coordinates.y >= g->max_height
        || (*(g->board))[coordinates.x][coordinates.y].owner != player)
        return;

    queue_t *q = malloc(sizeof(queue_t));
    checkNull(q);
    queue_initialize(q);

    queue_insert(q, coordinates);

    (*(g->board))[coordinates.x][coordinates.y].visited = true;
    (*(g->board))[coordinates.x][coordinates.y].area = new_area_number;

    while (!queue_empty(q))
    {
        coords_t coords = queue_get(q);
        uint32_t x = coords.x;
        uint32_t y = coords.y;
        if (x > 0 && check_if_same_player(g, player, x - 1, y))
        {
            (*(g->board))[x - 1][y].visited = true;
            (*(g->board))[x - 1][y].area = new_area_number;
            queue_insert_field(q, x - 1, y);
        }
        if (x < g->max_width - 1 && check_if_same_player(g, player, x + 1, y))
        {
            (*(g->board))[x + 1][y].visited = true;
            (*(g->board))[x + 1][y].area = new_area_number;
            queue_insert_field(q, x + 1, y);
        }
        if (y > 0 && check_if_same_player(g, player, x, y - 1))
        {
            (*(g->board))[x][y - 1].visited = true;
            (*(g->board))[x][y - 1].area = new_area_number;
            queue_insert_field(q, x, y - 1);
        }
        if (y < g->max_height - 1 && check_if_same_player(g, player, x, y + 1))
        {
            (*(g->board))[x][y + 1].visited = true;
            (*(g->board))[x][y + 1].area = new_area_number;
            queue_insert_field(q, x, y + 1);
        }
    }
    free(q);
    reset_visited(g, coordinates);
}

/** @brief Resetuje informacje o obszarze sąsiadów podanego pola
 * Sprawdza czy sąsiad podanego pola należy do gracza i czy nie ma numeru nowego obszaru.
 * Jeśli tak to zeruje obie wartości.
 * @param[in, out] g   – struktura przechowująca stan gry,
 * @param[in] player   – indeks gracza,
 * @param[in] x   – współrzędna x pola,
 * @param[in] y   – współrzędna y pola,
 * @param[in] new_area_number  – indeks nowego obszaru,
 */

static void reset_area_info(gamma_t *g, uint32_t player, uint32_t x, uint32_t y, uint32_t new_area_number)
{
    if (x > 0 && ((*(g->board))[x - 1][y]).owner == player
        && g->player_info[player].area_number[((*(g->board))[x - 1][y]).area] != new_area_number)
    {
        g->player_info[player].area_size[((*(g->board))[x - 1][y]).area] = 0;
        g->player_info[player].area_number[((*(g->board))[x - 1][y]).area] = 0;
    }
    if (x < g->max_width - 1 && ((*(g->board))[x + 1][y]).owner == player
        && g->player_info[player].area_number[((*(g->board))[x + 1][y]).area] != new_area_number)
    {
        g->player_info[player].area_size[((*(g->board))[x + 1][y]).area] = 0;
        g->player_info[player].area_number[((*(g->board))[x + 1][y]).area] = 0;
    }
    if (y > 0 && ((*(g->board))[x][y - 1]).owner == player
        && g->player_info[player].area_number[((*(g->board))[x][y - 1]).area] != new_area_number)
    {
        g->player_info[player].area_size[((*(g->board))[x][y - 1]).area] = 0;
        g->player_info[player].area_number[((*(g->board))[x][y - 1]).area] = 0;
    }
    if (x < g->max_width - 1 && ((*(g->board))[x + 1][y]).owner == player
        && g->player_info[player].area_number[((*(g->board))[x + 1][y]).area] != new_area_number)
    {
        g->player_info[player].area_size[((*(g->board))[x + 1][y]).area] = 0;
        g->player_info[player].area_number[((*(g->board))[x + 1][y]).area] = 0;
    }
}

/** @brief Aktualizuje numer obszaru z jednej podanej strony
 * Sprawdza kierunek przez podany znak i ustawia numery obszarów z innych stron
 * na ten sam.
 * @param[in, out] g   – struktura przechowująca stan gry,
 * @param[in] player   – indeks gracza,
 * @param[in] x   – współrzędna x pola,
 * @param[in] y   – współrzędna y pola,
 * @param[in] new_area_size  – rozmiar nowego obszaru,
 * @param[in] direction  – kierunek,
 */

static void update_one_direction(gamma_t *g, uint32_t player, uint32_t x, uint32_t y, uint32_t new_area_size, char direction)
{
    coords_t *coords = malloc(sizeof(coords_t));
    checkNull(coords);

    uint32_t directed_x = 0, directed_y = 0;
    switch(direction)
    {
        case 'u':
            directed_x = x;
            directed_y = y + 1;
            break;
        case 'l':
            directed_x = x - 1;
            directed_y = y;
            break;
        case 'r':
            directed_x = x + 1;
            directed_y = y;
            break;
        case 'd':
            directed_x = x;
            directed_y = y - 1;
            break;
    }

    uint32_t new_area_number = g->player_info[player].area_number[((*(g->board))[directed_x][directed_y]).area];

    reset_area_info(g, player, x, y, new_area_number);

    g->player_info[player].area_number[((*(g->board))[directed_x][directed_y]).area] = new_area_number;

    coords->x = x - 1;
    coords->y = y;
    set_new_area_number(g, *coords, player, ((*(g->board))[directed_x][directed_y]).area);

    coords->x = x + 1;
    coords->y = y;
    set_new_area_number(g, *coords, player, ((*(g->board))[directed_x][directed_y]).area);

    coords->x = x;
    coords->y = y - 1;
    set_new_area_number(g, *coords, player, ((*(g->board))[directed_x][directed_y]).area);

    coords->x = x;
    coords->y = y + 1;
    set_new_area_number(g, *coords, player, ((*(g->board))[directed_x][directed_y]).area);

    g->player_info[player].area_size[((*(g->board))[directed_x][directed_y]).area] = new_area_size;
    ((*(g->board))[x][y]).area = ((*(g->board))[directed_x][directed_y]).area;

    free(coords);
}

void gamma_update_area(gamma_t *g, uint32_t player, uint32_t x, uint32_t y, bool add)
{
    uint32_t left = 0, up = 0, right = 0, down = 0;
    if (x > 0 && ((*(g->board))[x - 1][y]).owner == player)
        left = g->player_info[player].area_size[((*(g->board))[x - 1][y]).area];

    if (y < g->max_height - 1 && ((*(g->board))[x][y + 1]).owner == player)
        up = g->player_info[player].area_size[((*(g->board))[x][y + 1]).area];

    if (x < g->max_width - 1 && ((*(g->board))[x + 1][y]).owner == player)
        right = g->player_info[player].area_size[((*(g->board))[x + 1][y]).area];

    if (y > 0 && ((*(g->board))[x][y - 1]).owner == player)
        down = g->player_info[player].area_size[((*(g->board))[x][y - 1]).area];

    uint32_t max_area = max(max(left, up), max(right, down));

    uint32_t new_area_size = left + up + right + down + 1;

    uint32_t areas = count_areas(g, player, x, y);

    if (add)
        (g->player_info[player].current_areas) += areas - 1;
    else
        (g->player_info[player].current_areas) -= areas - 1;


    if (max_area == up)
    {
       update_one_direction(g, player, x, y, new_area_size, 'u');
    }
    else if (max_area == left)
    {
        update_one_direction(g, player, x, y, new_area_size, 'l');
    }
    else if (max_area == right)
    {
        update_one_direction(g, player, x, y, new_area_size, 'r');
    }
    else if (max_area == down)
    {
        update_one_direction(g, player, x, y, new_area_size, 'd');
    }
}
