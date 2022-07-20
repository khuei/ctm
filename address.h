#pragma once

#include <stdbool.h>

typedef struct Address {
	const char *addr;
	bool is_selected;
	struct Address *next;
} Address;

void create_addr(Address **, char *);
void create_rand_addr(Address **, int);
int select_addr(Address **, const char *);
int delete_addr(Address **, const char *);
Address *parse_addr(void);
const char *parse_current_addr(void);
void store_addr(Address **);
int clear_log(void);
