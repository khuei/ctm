#pragma once

typedef struct Mail {
	const char *id;
	const char *from;
	const char *subject;
	const char *date;
	struct Mail *next;
} Mail;

int retrieve_mailbox(void);
Mail *parse_mailbox(void);
