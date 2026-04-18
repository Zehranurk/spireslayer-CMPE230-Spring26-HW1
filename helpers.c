#include "helpers.h"
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

/*
Is reserved keyword
Returns 1 if the given word is a reserved keyword, 0 otherwise.
*/
int is_reserved_keyword(const char *word){
    static const char *keywords[] = {
        "Ironclad", "gains", "gold", "max", "hp", "card", "relic",
        "potion", "buys", "for", "removes", "upgraded", "upgrades",
        "enters", "room", "learns", "is", "effective", "against",
        "fights", "heals", "takes", "damage", "discards", "sells",
        "marks", "as", "exhaust", "Total", "Floor", "Where", "Deck",
        "size", "Relics", "Potions", "What", "Defeated", "Health",
        "Exhausts", "Exit",
        NULL
    };
    for(int i = 0; keywords[i] != NULL; i++){
        if(strcmp(word, keywords[i]) == 0){
            return 1;
        }
    }
    return 0;   
}

/*
Is valid name
Returns 1 if the given name is valid, 0 otherwise.
Rules for a valid name:
- Must not be a reserved keyword.
- Only letters and single spaces between words are allowed.
- Cannot be empty or consist solely of spaces.
- No leading or trailing spaces.
- No double spaces.
- At least one word
*/
int is_valid_name(const char *name){
    if(name == NULL || name[0] == '\0'){
        return 0; //Empty
    }

    if(name[0] == ' '){
        return 0; //Leading space
    }

    if(name[strlen(name) - 1] == ' '){
        return 0; //Trailing space
    }

    //Now check every character (only valid characters are letters and spaces)
    for(int i = 0 ; name[i] != '\0'; i++){
        if(name[i] == ' '){
            if(name[i+1] == ' '){
                return 0; //Double space
            }
        }else if(!isalpha((unsigned char)name[i])){
            return 0; //Not a letter or space
        }
    }

    char buffer[512];
    strncpy(buffer, name, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0'; 

    char *token = strtok(buffer, " ");
    while(token != NULL){
        if(is_reserved_keyword(token)){
            return 0; //Contains a reserved keyword
        }
        token = strtok(NULL, " ");
    }

    return 1; //Valid name
    

}

/*
Is valid positive integer
Returns 1 if the given string represents a valid positive integer, 0 otherwise.
Rules for a valid positive integer:
- No leading zeros
- No sign prefix
- Value at least 1
*/
int is_valid_positive_integer(const char *s){
    if(s == NULL || s[0] == '\0'){
        return 0;
    }
    if(s[0] == '+' || s[0] == '-'){
        return 0; //Sign prefix
    }
    if(s[0] == '0'){
        return 0; //Leading zero
    }
    for(int i = 0; s[i] != '\0'; i++){
        if(!isdigit((unsigned char)s[i])){
            return 0; //Not a digit
        }
    }

    long val = strtol(s, NULL, 10);
    if(val < 1 || val > INT_MAX){
        return 0; //Value out of range 
    }
    return 1; //Valid positive integer

}

//Returns 1 if str starts with prefix, 0 otherwise
int starts_with(const char *str, const char *prefix){
    return strncmp(str,prefix,strlen(prefix)) == 0;
}

//Returns 1 if the given name is in the exhaust list, 0 otherwise
int is_exhaust_tagged(ExhaustList *list, const char *name) {
    for (int i = 0; i < list->size; i++) {
        if (strcmp(list->names[i], name) == 0) return 1;
    }
    return 0;
}


void join_tokens(char *dest, char **tokens, int start, int end){
    dest[0]='\0';
    
    for(int i = start; i <= end; i++){
        strcat(dest, tokens[i]);
        if(i < end){
            strcat(dest, " ");
        }
    }
}

void add_card_to_deck(GameState *state, const char *card_name){
    int found = -1;
    for(int i = 0 ; i < state->deck.size; i++){
        if(strcmp(state->deck.names[i], card_name) == 0){
            found = i;
            break;
        }
    }

    if(found != -1){
        //card already exists, increment the count
        state->deck.base[found]++;
    }else{
        //grow deck if necessary
        if(state->deck.size >= state->deck.capacity){
            state->deck.capacity *= 2;
            char **tmp_names = realloc(state->deck.names, state->deck.capacity * sizeof(char *));
            int *tmp_base     = realloc(state->deck.base, state->deck.capacity * sizeof(int));
            int *tmp_upgraded = realloc(state->deck.upgraded, state->deck.capacity * sizeof(int));
            int *tmp_exhaust  = realloc(state->deck.exhaust, state->deck.capacity * sizeof(int));

            if(!tmp_names || !tmp_base || !tmp_upgraded || !tmp_exhaust){
                fprintf(stderr, "Memory allocation failed\n");
                exit(1);
            }

            state->deck.names    = tmp_names;
            state->deck.base     = tmp_base;
            state->deck.upgraded = tmp_upgraded;
            state->deck.exhaust  = tmp_exhaust;
        }

        int idx = state->deck.size;
        state->deck.names[idx] = strdup(card_name);
        state->deck.base[idx] = 1; // First copy of the card
        state->deck.upgraded[idx] = 0; // No upgraded version yet
        state->deck.exhaust[idx] = is_exhaust_tagged(&state->exhaust_list, card_name); // Check if the card is exhaust-tagged
        state->deck.size++;
    }
}

void add_potion_to_belt(GameState *state, const char *potion_name){
    state->potions.names[state->potions.size] = strdup(potion_name);
    state->potions.size++;
}

void add_relic(GameState *state, const char *relic_name){
    
    if(state -> relics.size >= state -> relics.capacity){
        state -> relics.capacity *= 2;
        char **tmp = realloc(state->relics.names, state->relics.capacity * sizeof(char *));
        if(!tmp){
            fprintf(stderr, "Memory allocation failed\n");
            exit(1);
        }
        state->relics.names = tmp;
    }

    state -> relics.names[state -> relics.size] = strdup(relic_name);
    state -> relics.size++;
}

const char *find_token_start(const char* s, int n){
    const char *p = s;
    for(int i = 0; i < n; i++){

        //skip current token
        while(*p && *p != ' '){
            p++;
        }
        //skip spaces to next token
        while(*p == ' '){
            p++;
        }
    }
    if(*p == '\0') return NULL;
    return p;
}

int has_double_space(const char *s){
    for(int i = 0; s[i] != '\0'; i++){
        if(s[i] == ' ' && s[i+1] == ' ') return 1;
    }
    return 0;
}

int tokenize(char *buf, char **tokens, int max_tokens){
    int count = 0;
    char *t = strtok(buf, " ");
    while(t != NULL && count < max_tokens){
        tokens[count++] = t;
        t = strtok(NULL, " ");
    }
    return count;
}