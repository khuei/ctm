#pragma once

#include <stdbool.h>

typedef struct Address {
	const char *addr;
	bool is_selected;
	struct Address *next;
} Address;

void create_addr(Address **, char *);
void create_rand_addr(Address **, int);
const char *parse_addr(void);
int clear_log(void);
