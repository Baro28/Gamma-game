/** @file
 * Interfejs klasy obsługującej tryb interaktywny
 *
 * @author Bartosz Ławny <bl418396@students.mimuw.edu.pl>
 * @copyright Uniwersytet Warszawski
 * @date 17.05.2020
 */

#include "interactive_mode.h"
#include "gamma_t.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>

/**
 * Struktury przechowujące stan terminala przed i po rozgrywce.
 */

struct termios oldt, newt;

/** @brief Sprawdza czy dany gracz może jeszcze wykonać ruch
 * Sprawdza liczbę wolnych pól danego gracza oraz czy wykonał już swój złoty ruch.
 * Jeśli tak to zwraca false, w przeciwnym wypadku true.
 * @param[in] g   – struktura przechowująca stan gry,
 * @param[in] player   – sprawdzany gracz.
 * @return Zwraca true jeśli dany gracz może jeszcze wykonać ruch,
 * false w przeciwnym wypadku.
 */

static bool possible_move(gamma_t *g, uint32_t player)
{
    if(gamma_free_fields(g, player) == 0 && gamma_golden_possible(g, player) == false)
    {
        return false;
    }
    return true;
}

/** @brief Oblicza numer kolejnego gracza w kolejce
 * Przechodzi kolejnych graczy dopóki nie natrafi na gracza który może jeszcze wykonać ruch
 * lub licznik będzie równy liczbie graczy co oznacza że żaden gracz nie może już wykonać ruchu
 * czyli nie ważne jakiego gracza zwrócimy gra i tak zakończy się tuż po wyjściu z funkcji.
 * @param[in] g   – struktura przechowująca stan gry,
 * @param[in] player   – numer gracza.
 * @return Zwraca numer kolejnego gracza w kolejce
 */

static uint32_t next_player(gamma_t *g, uint32_t player)
{
    uint32_t counter = 0;
    do
    {
        counter++;
        if(player == g->players)
            player = 1;
        else
            player++;
    }
    while(!possible_move(g, player) && counter <= g->players);
    return player;
}

/** @brief Czyści ekran i przesuwa kursor w lewy górny róg
 */

static void clear()
{
    printf("\033[2J");
    printf("\033[H");
}

/** @brief Wypisuje obecny stan planszy
 * Przechodzi przez wszystkie pola planszy i wypisuje je.
 * Jeśli natrafi na pole w którym obecnie znajduje się kursor
 * to odpowiednio zaznacza to pole.
 * Koloruje również pola gracza obecnie wykonującego ruch.
 * @param[in] g   – struktura przechowująca stan gry,
 * @param[in] row   – współrzędna y kursora,
 * @param[in] column   – współrzędna x kursora.
 * @param[in] player - numer gracza.
 */

static void print_board(gamma_t *g, uint32_t row, uint32_t column, uint32_t player)
{
    clear();
    for(uint32_t i = 0; i < g->max_height; i++)
    {
        for(uint32_t j = 0; j < g->max_width; j++)
        {
            if(((*(g->board))[j][g->max_height - 1 - i]).owner == player)
                printf("\033[31;1m");
            if(i == row && j == column)
                printf("\033[47;1m\033[30;1m");
            if(((*(g->board))[j][g->max_height - 1 - i]).owner != 0)
                printf("%*u", log_10(g->players), ((*(g->board))[j][g->max_height - 1 - i]).owner);
            else
            {
                printf("%*s.", log_10(g->players) - 1, "");
            }
            printf("\033[0m");
            printf(" ");
        }
        printf("\n");
    }
}

/** @brief Ustawia pozycję kursora na podaną
 * @param[in] row - współrzędna y kursora,
 * @param[in] column - współrzędna x kursora.
 */

static void set_cursor_position(uint32_t row, uint32_t column)
{
    printf("\033[%u;%uH", row, column);
}

/** @brief Zmienia ustawienia terminala z powrotem na oryginalne
 */

static void disable_raw_mode()
{
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}

/** @brief Włącza tryb "surowy" terminala
 * Zmienia dwie flagi w ustawieniach terminala.
 * Zanegowanie ICANON sprawia że terminal nie czeka na enter by przekazać input do programu
 * Zanegowanie ECHO sprawia że terminal nie wypisuje na ekran otrzymanego inputu.
 */

static void enable_raw_mode()
{
    tcgetattr(STDIN_FILENO, &oldt);
    atexit(disable_raw_mode);

    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);

    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
}

/** @brief Wypisuje informacje o graczu wykonującym ruch pod planszą
 * Najpierw przesuwa kursor na pozycję pod planszą po czym wypisuje
 * informacje o obecnym graczu i czy może wykonać złoty ruch.
 * @param[in] g - struktura przechowująca stan gry,
 * @param[in] player - numer gracza.
 */

static void print_player_info(gamma_t *g, uint32_t player)
{
    set_cursor_position(g->max_height + 1, 0);
    printf("\x1b[33;1mPLAYER:\x1b[0m \x1b[31;1m%u\x1b[0m | \x1b[34;1mBUSY FIELDS: %lu\x1b[0m | \x1b[36;1mFREE FIELDS: %lu\x1b[0m\n", player, gamma_busy_fields(g, player),
                                                                                gamma_free_fields(g, player));
    if(gamma_golden_possible(g, player))
        puts("\x1b[32;1mGOLDEN MOVE AVAILABLE\x1b[0m");
    else
        puts("\x1b[31;1mGOLDEN MOVE UNAVAILABLE\x1b[0m");
}

/** @brief Wypisuje końcowy stan planszy
 */

static void print_result(gamma_t *g)
{
    print_board(g, g->max_height + 1, g->max_width + 1, g->players + 1); //Nie chcemy podświetlać żadnego pola ani gracza
    set_cursor_position(g->max_height + 1, 0);
    printf("\033[0J");
    for(uint32_t i = 1; i <= g->players; i++)
        printf("PLAYER %u: %lu fields\n", i, gamma_busy_fields(g, i));
}

/** @brief Sprawdza czy rozgrywka się już zakończyła
 * Sprawdza czy jakikolwiek gracz może jeszcze wykonać ruch.
 * @return Zwraca true jeśli żaden gracz nie może już wykonać ruchu
 * lub false w przeciwnym wypadku.
 */

static bool game_end(gamma_t *g)
{
    for(uint32_t i = 1; i <= g->players; i++)
        if(possible_move(g, i))
            return false;
    return true;
}

/** @brief Przesuwa obecną pozycję kursora
 * @param[in] g - struktura przechowująca stan gry,
 * @param[in] row - współrzędna y kursora,
 * @param[in] column - współrzędna x kursora.
 */

static void move(gamma_t *g, uint32_t *row, uint32_t *column)
{
    int next = getchar();
    if(next == '[')
    {
        int arrow = getchar();
        switch(arrow)
        {
            case 'A':
                if((*row) > 0)
                {
                    (*row)--;
                }
                break;
            case 'B':
                if((*row) < g->max_height - 1)
                {
                    (*row)++;
                }
                break;
            case 'C':
                if((*column) < g->max_width - 1)
                {
                    (*column)++;
                }
                break;
            case 'D':
                if((*column) > 0)
                {
                    (*column)--;
                }
                break;
            default:
                ungetc(arrow, stdin);
                break;
        }
    }
    else
        ungetc(next, stdin);
}

/** @brief Ukyrwa terminalowy kursor
 */

static void hide_cursor()
{
    printf("\033[?25l");
}

/** @brief Pokazuje terminalowy kursor
 */

static void show_cursor()
{
    printf("\033[?25h");
}

void interactive_mode(gamma_t *g)
{

    enable_raw_mode();

    int c;
    uint32_t row = g->max_height - 1;
    uint32_t column = 0;
    uint32_t player = 1;

    set_cursor_position(row, column);
    hide_cursor();
    atexit(show_cursor);


    while(true)
    {
        print_board(g, row, column, player);
        print_player_info(g, player);
        if(game_end(g))
        {
            break;
        }
        c = getchar();
        if(c == '\033')
        {
            move(g, &row, &column);
        }
        else if(c == ' ')
        {
            if(gamma_move(g, player, column, g->max_height - row - 1))
            {
                player = next_player(g, player);
            }
        }
        else if(c == 'G' || c == 'g')
        {
            if(gamma_golden_move(g, player, column , g->max_height - row - 1))
            {
                player = next_player(g, player);
            }
        }
        else if(c == 'C' || c == 'c')
        {
            player = next_player(g, player);
        }
        else if(c == '\4')
            break;
    }
    print_result(g);
}
