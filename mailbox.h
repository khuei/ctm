#pragma once

typedef struct Mail {
	unsigned int id;
	const char *from;
	const char *subject;
	const char *date;
	struct Mail *next;
} Mail;

void retrieve_mailbox(void);
Mail *parse_mailbox(void);
