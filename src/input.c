/** @file
 * Interfejs klasy obsługującej input
 *
 * @author Bartosz Ławny <bl418396@students.mimuw.edu.pl>
 * @copyright Uniwersytet Warszawski
 * @date 17.05.2020
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <errno.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "gamma_t.h"
#include "gamma.h"
#include "batch_mode.h"
#include "interactive_mode.h"

void skipWhitespace(char **string)
{
    while (**string && isspace(**string) && **string != '\n')
        (*string)++;
}

bool isNumberOfWordsCorrect(char *string, uint32_t number)
{
    uint32_t counter = 0;
    char prev;

    for (uint32_t i = 0; i < number; i++)
    {
        char a = *string;
        skipWhitespace(&string);
        char b = *string;
        if(a == b)
            return false;

        prev = *string;

        while (*string && !isspace(*string) && (unsigned char) *string >= '0' && (unsigned char) *string <= '9')
            string++;

        if (*string != prev)
            counter++;
    }
    skipWhitespace(&string);

    if (*string == '\n' && counter == number)
        return true;

    return false;
}

/** @brief Oblicza długość wyrazu zawartego w napisie
 * Przechodzi przez napis dopóki nie napotka białego znaku
 * lub końca napisu
 * @param[in] string   – napis zawierający wyraz,
 * @return Zwraca długość wyrazu zawartego w napisie
 */

static int wordLength(char *string)
{
    int length = 0;
    while (*string && !isspace(*string))
    {
        string++;
        length++;
    }
    return length;
}

/** @brief Kopiuje zawartość jednego wyrazu do drugiego
 * Przechodzi przez wyraz źródłowy i kopiuje go znak po znaku do
 * wyrazu docelowego dopóki nie napotka '\n' lub końca napisu.
 * Dodaje również na koniec znak końca napisu.
 * @param[in] source   – wyraz źródłowy,
 * @param[in] destination   – wyraz docelowy
 */

static void wordCopy(char *source, char *destination)
{
    while (*source && !isspace(*source))
    {
        *destination = *source;
        destination++;
        source++;
    }
    *destination = '\0';
}

uint32_t word_to_int(char **string, bool *error)
{
    (*string)++;
    skipWhitespace(string);
    char *word = malloc(wordLength(*string) + 1 * sizeof(char));
    checkNull(word);

    skipWhitespace(string);
    wordCopy(*string, word);
    *string += wordLength(*string);

    uint64_t output = strtoul(word, NULL, 0);
    free(word);
    if(output > UINT32_MAX || errno != 0)
        (*error) = true;
    return output;
}

/** @brief Sprawdza czy plansza zmieściłaby się w oknie terminalu
 * Sprawdza obecny rozmiar okna terminalu i porównuje go z wymiarami planszy.
 * Szerokość plaszy jest mnożona przez liczbę cyfr w maksymalnym graczu + 1 aby
 * uwzględnić spacje między polami.
 * @param[in] height   – wysokość planszy,
 * @param[in] width   – szerokość planszy,
 * @param[in] players - liczba graczy.
 * @return Zwraca true jeśli plansza zmieści się w oknie lub false w przeciwnym wypadku.
 */

static bool check_window_size(uint32_t height, uint32_t width, uint32_t players)
{
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    if (height > w.ws_row || width * (log_10(players) + 1) > w.ws_col)
    {
        printf("Plansza nie zmieściłaby się w oknie\n");
        return false;
    }
    return true;
}

void parseInput()
{
    char *string = NULL;
    size_t size = 0;
    uint32_t line = 0;
    bool exit1 = false;

    while (getline(&string, &size, stdin) != -1)
    {
        line++;
        char *temp = string;

        if (*temp == '#' || *temp == '\n')
            continue;

        char mode = *temp;
        temp++;

        if(!isNumberOfWordsCorrect(temp, 4) || (mode != 'B' && mode != 'I'))
        {
            fprintf(stderr, "ERROR %u\n", line);
            continue;
        }

        bool error = false;
        uint32_t width = word_to_int(&temp, &error);
        uint32_t height = word_to_int(&temp, &error);
        uint32_t players = word_to_int(&temp, &error);
        uint32_t areas = word_to_int(&temp, &error);

        if(error)
        {
            fprintf(stderr, "ERROR %u\n", line);
            continue;
        }

        if (mode == 'B')
        {
            gamma_t *g = gamma_new(width, height, players, areas);
            if(!g)
            {
                fprintf(stderr, "ERROR %u\n", line);
                continue;
            }
            printf("OK %u\n", line);
            batch_mode(g, line);
            gamma_delete(g);
            break;
        }
        else
        {
            gamma_t *g = gamma_new(width, height, players, areas);
            if (!g)
            {
                fprintf(stderr, "ERROR %u\n", line);
                continue;
            }
            if (!check_window_size(height, width, players))
            {
                exit1 = true;
                break;
            }
            interactive_mode(g);
            gamma_delete(g);
            break;
        }
    }
    free(string);
    if(exit1)
        exit(1);
}