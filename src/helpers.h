#ifndef HELPERS_H
#define HELPERS_H
#include "state.h"

int is_reserved_keyword(const char *word);
int is_valid_name(const char *name);
int is_valid_positive_integer(const char *s);
int starts_with(const char *str, const char *prefix);
int is_exhaust_tagged(ExhaustList *list, const char *name);
void join_tokens(char *dest, char **tokens, int start, int end);
void add_card_to_deck(GameState *state, const char *card_name);
void add_relic(GameState *state, const char *relic_name);
void add_potion_to_belt(GameState *state, const char *potion_name);
const char *find_token_start(const char *s, int n);
int has_double_space(const char *s);
int tokenize(char *buf, char **tokens, int max_tokens);
#endif