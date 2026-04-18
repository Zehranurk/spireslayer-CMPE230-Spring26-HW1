#ifndef STATE_H
#define STATE_H
#define MAX_NAME_LEN  128
#define MAX_LINE_LEN  1024

#include <stdlib.h>
#include <string.h>

typedef struct {
    char **names;
    int    size;
    int    capacity;
} ExhaustList;

typedef struct{
    char **names;
    int *base; //counts of the base version of the card
    int *upgraded; //counts of the upgraded version of the card
    int *exhaust; // exhaust[i] -> 0 = not exhaust, 1 = exhaust
    int size;
    int capacity;

}Deck;

typedef struct{
    char **names;
    int size;
    int capacity;
}Relics;

typedef struct{
    char *names[3]; //potion belt is limited to max size of 3
    int size;

}Potions;

typedef struct{
    char *type;
    char *item;
}CodexEntry;

typedef struct{
    char *enemy;
    CodexEntry *entries; //entries to a single record of an enemy
    int entry_count;
    int entry_capacity;

}CodexRecord;

typedef struct{
    CodexRecord *records; //records of a single enemy
    int size;
    int capacity;
}Codex;

typedef struct{  
    char *enemy;
    int count;
} DefeatedEntry;

typedef struct{
    DefeatedEntry *entries;
    int size;
    int capacity;
}DefeatedLog;

typedef struct{
    
    int gold;
    int current_hp;
    int max_hp;
    int floor;
    char current_room[16];
    
    Deck deck;
    Relics relics;
    Potions potions;
    Codex codex;
    DefeatedLog defeated_log;
    ExhaustList exhaust_list;



}GameState;

void init_game_state(GameState *state);
void free_game_state(GameState *state);

int find_codex_enemy(Codex *c, const char *enemy);
void add_codex_enemy(Codex *c, const char *enemy);
void add_codex_entry(CodexRecord *rec, const char *type, const char *item);
int codex_has_entry(CodexRecord *rec, const char *type, const char *item);

int find_relic(Relics *r, const char *name);
int find_potion(Potions *p, const char *name);
int find_deck_card(Deck *d, const char *name);

void add_defeated_enemy(DefeatedLog *d, const char *enemy);


#endif