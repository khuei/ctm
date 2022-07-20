#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#include <curl/curl.h>

#include "address.h"
#include "mailbox.h"
#include "message.h"

int
main(int argc, char *argv[])
{
	curl_global_init(CURL_GLOBAL_SSL);

	if(argc == 1) {
		fprintf(stderr, "Error: empty command\n");
		return -1;
	}

	if (!strcmp(argv[1], "addr")) {
		Address *head = NULL;
		if (!strcmp(argv[2], "create")) {
			if (argv[3][0] != '\0') {
				create_addr(&head, argv[3]);
			} else {
				fprintf(stderr, "Error: no email adddress is provided\n");
				return -1;
			}
		} else if (!strcmp(argv[2], "new")) {
		} else if (!strcmp(argv[2], "delete")) {
		} else if (!strcmp(argv[2], "current")) {
			const char *current_addr = parse_addr();

			if (current_addr != NULL && strcmp(current_addr, "(null)")) {
				printf("%s\n", parse_addr());
			} else {
				fprintf(stderr, "Error: unable to get current email address\n");
				return -1;
			}
		} else {
			fprintf(stderr, "Error: invalid argument\n");
			return -1;
		}
	} else if (!strcmp(argv[1], "refresh")) {
		retrieve_mailbox();
	} else if (!strcmp(argv[1], "list")) {
		Mail *mailbox = parse_mailbox();

		if (mailbox == NULL) {
			printf("Notice: mailbox is empty ... refreshing mailbox");
			retrieve_mailbox();
			printf(" [ Success ]\n");
			return 0;
		}

		for (int i = 1; mailbox != NULL; mailbox = mailbox->next) {
			printf("[%d] Subject: [ %s ] | From: [ %s ] | Date: [ %s ]\n", i,
			       mailbox->subject, mailbox->from, mailbox->date);
			++i;
		}

		free(mailbox);
	} else if (!strcmp(argv[1], "view")) {
		bool is_number = true;
		bool is_existed = false;
		const char *selected_id;
		Mail *mailbox = parse_mailbox();

		for (int i = 0; argv[2][i] != '\0'; ++i) {
			if (!isdigit(argv[2][i]))
				is_number = false;
		}

		if (!is_number) {
			fprintf(stderr, "Error: input is not a number\n");
			return -1;
		}

		for (int i = 1; mailbox != NULL; mailbox = mailbox->next) {
			if (atoi(argv[2]) == i) {
				selected_id = mailbox->id;
				is_existed = true;
			}
			++i;
		}

		if (!is_existed) {
			fprintf(stderr, "Error: invalid message number\n");
			return -1;
		}

		Message *msg = parse_message((char *)selected_id);

		printf("From: %s\nDate: %s\nSubject: %s\n\n%s",
		       msg->from, msg->date, msg->subject,
		       msg->body);

		if (msg->attachments[0] != NULL) {
			puts("\nAttachments: ");
			for (int i = 0; msg->attachments[i] != NULL; ++i)
				printf("[%d] %s\n", i, msg->attachments[i]);
		}

		free(msg);
	} else {
		fprintf(stderr, "Error: invalid argument\n");
		return -1;
	}

	return 0;
}
