#include "input_utils.h"
#include <string.h>

void clean_input(char *str) {
    if (!str) return;

    str[strcspn(str, "\n")] = 0;

    for (int i = 0; str[i]; i++) {
        if (str[i] == '|')
            str[i] = ' ';
    }
}
