#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <sys/stat.h>

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

	int exit_code = 0;

	if (!strcmp(argv[1], "addr")) {
		Address *head = parse_addr();

		if (!strcmp(argv[2], "create")) {
			if (argv[3][0] != '\0') {
				create_addr(&head, argv[3]);
			} else {
				fprintf(stderr, "Error: no email adddress is provided\n");
				exit_code = -1;
			}
		} else if (!strcmp(argv[2], "new")) {
			if (argv[3][0] != '\0')
				exit_code = create_rand_addr(&head, argv[3]);
			else
				exit_code = create_rand_addr(&head, "1");
		} else if (!strcmp(argv[2], "delete")) {
			if (argv[3][0] != '\0') {
				exit_code = delete_addr(&head, argv[3]);
			} else {
				fprintf(stderr, "Error: empty input\n");
				exit_code = -1;
			}
		} else if (!strcmp(argv[2], "select")) {
			if (argv[3][0] != '\0') {
				exit_code = select_addr(&head, argv[3]);
			} else {
				fprintf(stderr, "Error: empty input\n");
				exit_code = -1;
			}
		} else if (!strcmp(argv[2], "list")) {
			Address *current = head;

			for (int i = 1; current != NULL; ++i) {
				if (current->is_selected)
					printf("> [%d] - %s\n", i, current->addr);
				else
					printf("[%d] - %s\n", i, current->addr);

				current = current->next;
			}
		} else if (!strcmp(argv[2], "current")) {
			const char *current_addr = parse_current_addr();

			if (current_addr != NULL && strcmp(current_addr, "(null)")) {
				printf("%s\n", parse_current_addr());
			} else {
				fprintf(stderr, "Error: unable to get current email address\n");
				exit_code = -1;
			}
		} else {
			fprintf(stderr, "Error: invalid argument\n");
			exit_code = -1;
		}

		exit_code = store_addr(&head);
	} else if (!strcmp(argv[1], "refresh")) {
		retrieve_mailbox();
	} else if (!strcmp(argv[1], "list")) {
		Mail *mailbox = parse_mailbox();

		if (mailbox == NULL) {
			printf("Notice: mailbox is empty ... refreshing mailbox ...");
			exit_code = retrieve_mailbox();

			if (exit_code == 0)
				printf("[ Success ]\n");
			else
				printf("[ Failed ]\n");

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
			exit_code = -1;
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
			exit_code = -1;
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
	} else if (!strcmp(argv[1], "help")) {
		puts("Usage: ctm addr [command]");
		puts("       ctm [command]");
		puts("");
		puts("Managing Email Address Commands:");
		puts("    create  -- create email address");
		puts("    new     -- create a new email address");
		puts("    current -- display current email address");
		puts("    select  -- select email address");
		puts("    delete  -- delete email address");
		puts("");
		puts("Email Commands:");
		puts("    refresh -- reload mailbox");
		puts("    list    -- show all messages in mailbox");
		puts("    view    -- display specified message");

	} else {
		fprintf(stderr, "Error: invalid argument\n");
		exit_code = -1;
	}

	return exit_code;
}
