#pragma once

typedef struct Message {
	const char *id;
	const char *from;
	const char *subject;
	const char *date;
	const char *attachments;
	const char *body;
	struct Message *next;
} Message;

Message *parse_message(char *);
