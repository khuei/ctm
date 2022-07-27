#pragma once

#include <stdbool.h>

typedef struct Address {
	const char *addr;
	bool is_selected;
	struct Address *next;
} Address;

int create_addr(Address **, char *);
int create_rand_addr(Address **, const char *);
int select_addr(Address **, const char *);
int delete_addr(Address **, const char *);
Address *parse_addr(void);
const char *parse_current_addr(void);
int store_addr(Address **);
int clear_log(void);
