#pragma once

typedef struct Message {
	const char *id;
	const char *from;
	const char *subject;
	const char *date;
	const char *body;
	struct Message *next;
	const char *attachments[];
} Message;

Message *parse_message(char *);
