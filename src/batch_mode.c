/** @file
 * Implementacja klasy obsługującej tryb wsadowy
 *
 * @author Bartosz Ławny <bl418396@students.mimuw.edu.pl>
 * @copyright Uniwersytet Warszawski
 * @date 17.05.2020
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "batch_mode.h"
#include "input.h"
#include "gamma_t.h"
#include "gamma.h"

/** @brief Funkcja sprawdza poprawność komendy i wywołuje odpowiednią
 * Funkcja najpierw sprawdza czy zgadza się liczba słów do danej komendy,
 * jeśli tak to wypisuje jej wynik.
 */

static void choose_command(gamma_t *g, uint32_t line, char *string, char command)
{
    uint32_t correct_number_of_words = 0;

    if (command == 'm' || command == 'g')
        correct_number_of_words = 3;
    else if(command == 'b' || command == 'f' || command == 'q')
        correct_number_of_words = 1;
    else if(command != 'p')
    {
        fprintf(stderr, "ERROR %u\n", line);
        return;
    }

    if(!isNumberOfWordsCorrect(string, correct_number_of_words))
    {
        fprintf(stderr, "ERROR %u\n", line);
        return;
    }

    bool error = false;
    uint32_t x = 0, y = 0;
    uint32_t player = word_to_int(&string, &error);
    if (command == 'm' || command == 'g')
    {
        x = word_to_int(&string, &error);
        y = word_to_int(&string, &error);
    }

    if(error)
    {
        fprintf(stderr, "ERROR %u\n", line);
        return;
    }
    char *board;

    switch(command)
    {
        case 'm':
            printf("%d\n", gamma_move(g, player, x, y));
            break;
        case 'g':
            printf("%d\n", gamma_golden_move(g, player, x, y));
            break;
        case 'b':
            printf("%lu\n", gamma_busy_fields(g, player));
            break;
        case 'f':
            printf("%lu\n", gamma_free_fields(g, player));
            break;
        case 'q':
            printf("%d\n", gamma_golden_possible(g, player));
            break;
        case 'p':
            board = gamma_board(g);
            printf("%s", board);
            free(board);
            break;
    }
}

void batch_mode(gamma_t *g, uint32_t line)
{
    char *string = NULL;
    size_t size = 0;

    while (getline(&string, &size, stdin) != -1)
    {
        line++;
        char *temp = string;

        if (*temp == '#' || *temp == '\n')
            continue;

        char mode = *temp;
        temp++;

        choose_command(g, line, temp, mode);
    }
    free(string);
}
