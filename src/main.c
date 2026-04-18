#include <stdio.h>
#include <string.h>
#include "state.h"
#include "interpreter.h"
#include "queries.h"

#define MAX_LINE_LENGTH 1024


static void chomp_newline(char *line) {
    size_t len = strlen(line);

    if (len > 0 && line[len - 1] == '\n') {
        line[len - 1] = '\0';
    }
}

int main(void) {
    char line[MAX_LINE_LENGTH + 2];
    GameState state;

    init_game_state(&state);

    while (1) {
        printf("» ");
        fflush(stdout);

        if (fgets(line, sizeof(line), stdin) == NULL) { 
            break;
        }

        chomp_newline(line);

        if (strcmp(line, "Exit") == 0) {
            break;
        }
        
        fprintf(stderr,"» ");
        if (!execute_line(&state, line)) {
            printf("INVALID\n");
        }
    }
    
    free_game_state(&state);
    return 0;
}
