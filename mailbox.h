#pragma once

typedef struct Mail {
	unsigned int id;
	const char *from;
	const char *subject;
	const char *date;
	struct Mail *next;
} Mail;

Mail *retrieve_mailbox(const char *);
void store_mailbox(Mail *);
