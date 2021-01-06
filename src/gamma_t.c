/** @file
 * Implementacja klasy przechowującej pomocnicze funkcje i struktury
 *
 * @author Bartosz Ławny <bl418396@students.mimuw.edu.pl>
 * @copyright Uniwersytet Warszawski
 * @date 17,05.2020
 */
#include <inttypes.h>
#include <stdlib.h>
#include "gamma_t.h"

uint32_t log_10(uint32_t x)
{
    uint32_t i = 0;
    while (x > 0)
    {
        x /= 10;
        i++;
    }
    return i;
}

void checkNull(void *pointer)
{
    if(pointer == NULL)
        exit(1);
}