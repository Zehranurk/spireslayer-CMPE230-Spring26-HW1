#ifndef QUERIES_H
#define QUERIES_H

#include "state.h"

int handle_gold_query(GameState *state, char *line);
int handle_floor_query(GameState *state, char *line);
int handle_where_query(GameState *state, char *line);
int handle_health_query(GameState *state, char *line);
int handle_deck_size_query(GameState *state, char *line);
int handle_total_card_query(GameState *state, char *line);
int handle_total_upgraded_card_query(GameState *state, char *line);
int handle_defeated_query(GameState *state, char *line);
int handle_exhausts_query(GameState *state, char *line);
int handle_relics_query(GameState *state, char *line);
int handle_potions_query(GameState *state, char *line);
int handle_effective_query(GameState *state, char *line);
int handle_deck_query(GameState *state, char *line);

#endif