#include "state.h"
#include <stdlib.h>
#include <stdio.h>

static void init_deck(Deck *deck) {
    deck->size =0;
    deck->capacity = 8;
    deck->names = malloc(deck->capacity * sizeof(char *));
    deck->base = malloc(deck->capacity * sizeof(int));  
    deck->upgraded = malloc(deck->capacity * sizeof(int)); 
    deck->exhaust = malloc(deck->capacity * sizeof(int));
}

static void free_deck(Deck *deck){
    for (int i = 0; i < deck->size; i++) {
        free(deck->names[i]);
    }
    free(deck->names);
    free(deck->base);
    free(deck->upgraded);
    free(deck->exhaust);
}

static void init_relics(Relics *relics) {
    relics->size = 0;
    relics->capacity = 8;
    relics->names = malloc(relics->capacity * sizeof(char *));
}

static void free_relics(Relics *relics) {
    for (int i = 0; i < relics->size; i++) {
        free(relics->names[i]);
    }
    free(relics->names);
}

static void init_potions(Potions *potions){
    potions->size = 0;
    for (int i = 0; i < 3; i++) {
        potions->names[i] = NULL;
    }
}

static void free_potions(Potions *potions){
    for (int i = 0; i < potions->size; i++) {
        free(potions->names[i]);
    }
}

static void init_codex(Codex *codex) {
    codex->size = 0;
    codex->capacity = 8;
    codex->records = malloc(codex->capacity * sizeof(CodexRecord));
}

static void free_codex(Codex *codex) {
    for (int i = 0; i < codex->size; i++) {
        CodexRecord *record = &codex->records[i];
        free(record->enemy);
        for (int j = 0; j < record->entry_count; j++) {
            free(record->entries[j].type);
            free(record->entries[j].item);
        }
        free(record->entries);
    }
    free(codex->records);
}

static void init_defeated_log(DefeatedLog *log) {
    log->size = 0;
    log->capacity = 8;
    log->entries = malloc(log->capacity * sizeof(DefeatedEntry));
}

static void free_defeated_log(DefeatedLog *log) {
    for (int i = 0; i < log->size; i++) {
        free(log->entries[i].enemy);
    }
    free(log->entries);
}

static void init_exhaust_list(ExhaustList *list) {
    list->size = 0;
    list->capacity = 8;
    list->names = malloc(list->capacity * sizeof(char *));
}

static void free_exhaust_list(ExhaustList *list) {
    for (int i = 0; i < list->size; i++) {
        free(list->names[i]);
    }
    free(list->names);
}

void init_game_state(GameState *state){
    state->gold= 0;
    state->current_hp = 80;
    state->max_hp = 80;
    state->floor = 0;
    strcpy(state->current_room, "NONE");
    init_deck(&state->deck);
    init_relics(&state->relics);
    init_potions(&state->potions);
    init_codex(&state->codex);
    init_defeated_log(&state->defeated_log);
    init_exhaust_list(&state->exhaust_list);

}

void free_game_state(GameState *state){ //made this to free all the memory after exiting the game
    free_deck(&state->deck);
    free_relics(&state->relics);
    free_potions(&state->potions);
    free_codex(&state->codex);
    free_defeated_log(&state->defeated_log);
    free_exhaust_list(&state->exhaust_list);
}

int find_codex_enemy(Codex *c, const char *enemy){
    for(int i = 0; i < c->size; i++){
        if(strcmp(c->records[i].enemy, enemy) == 0){
            return i;
        }
    }
    return -1;
}

void add_codex_enemy(Codex *c, const char *enemy){
    if(c->size >= c->capacity){
        c->capacity *= 2;
        CodexRecord *tmp = realloc(c->records, c->capacity * sizeof(CodexRecord));
        if(tmp == NULL){
            fprintf(stderr, "Memory allocation failed\n");
            exit(1);
        }
        c->records = tmp;

    }

    int i = c->size;
    c->records[i].enemy = strdup(enemy);
    if(c->records[i].enemy == NULL){
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    c->records[i].entry_count = 0;
    c->records[i].entry_capacity = 4;
    c->records[i].entries = malloc(c->records[i].entry_capacity * sizeof(CodexEntry));
    if(c->records[i].entries == NULL){fprintf(stderr, "Memory allocation failed\n"); exit(1); }
    c->size++;

}

void add_codex_entry(CodexRecord *rec, const char *type, const char *item){

    if(rec->entry_count >= rec->entry_capacity){
        rec-> entry_capacity *= 2;
        CodexEntry *tmp = realloc(rec->entries, rec->entry_capacity * sizeof(CodexEntry));
        if(tmp == NULL){
            fprintf(stderr, "Memory allocation failed\n");
            exit(1);
        }
        rec->entries = tmp;

    }

    int i = rec->entry_count;
    rec->entries[i].type = strdup(type);
    rec->entries[i].item = strdup(item);
    if(rec->entries[i].type == NULL || rec->entries[i].item == NULL){
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);  
    }

    rec->entry_count++;

}

int codex_has_entry(CodexRecord *rec, const char *type, const char *item){
    for(int i = 0; i < rec->entry_count; i++){
        if(strcmp(rec->entries[i].type,type) == 0 && strcmp(rec->entries[i].item, item) == 0){
            return 1;
        }
    }
    return 0;
}

int find_relic(Relics *r, const char *name){
    for(int i = 0; i < r->size; i++){
        if(strcmp(r->names[i], name) == 0){
            return i;
        }
    }
    return -1;
}

int find_potion(Potions *p, const char *name){
    for(int i = 0; i < p->size; i++){
        if(strcmp(p->names[i], name) == 0){
            return i;
        }
    }
    return -1;
}

int find_deck_card(Deck *d, const char *name){
    for(int i = 0; i < d->size; i++){
        if(strcmp(d->names[i], name) == 0) return i;
    }
    return -1;
}

void add_defeated_enemy(DefeatedLog *d, const char *enemy){
    if(d->size == d->capacity){
        d->capacity *= 2;
        DefeatedEntry *tmp = realloc(d->entries, d->capacity * sizeof(DefeatedEntry));
        if(tmp == NULL){ fprintf(stderr, "Memory allocation failed\n"); exit(1); }
        d->entries = tmp;
    }

    int i = d->size;
    d->entries[i].enemy = strdup(enemy);
    if(d->entries[i].enemy == NULL){ fprintf(stderr, "Memory allocation failed\n"); exit(1); }
    d->entries[i].count = 0;
    d->size++;
}
