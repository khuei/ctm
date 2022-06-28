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

	if (!strcmp(argv[1], "create")) {
		const char *email_addr = create_addr();

		if (email_addr == NULL || !strcmp(email_addr, "(null)")) {
			fprintf(stderr, "Error: unable to create new email address");
			return -1;
		}

		printf("%s\n", email_addr);
	} else if (!strcmp(argv[1], "current")) {
		const char *current_addr = parse_addr();
		
		if (current_addr != NULL && strcmp(current_addr, "(null)")) {
			printf("%s\n", parse_addr());
		} else {
			fprintf(stderr, "Error: unable to get current email address\n");
			return -1;
		}
	} else if (!strcmp(argv[1], "refresh")) {
		retrieve_mailbox();
	} else if (!strcmp(argv[1], "list")) {
		Mail *mailbox = parse_mailbox();

		if (mailbox == NULL)
			return 0;

		for (int i = 1; mailbox != NULL; mailbox = mailbox->next)
			printf("[%d] subject: %s | from: %s | %s\n", i,
			       mailbox->subject, mailbox->from, mailbox->date);

		free(mailbox);
	} else if (!strcmp(argv[1], "view")) {
		bool is_number = true;

		for (int i = 0; argv[2][i] != '\0'; ++i) {
			if (!isdigit(argv[2][i]))
				is_number = false;
		}

		if (!is_number) {
			fprintf(stderr, "Error: invalid email ID\n");
			return -1;
		}

		Message *msg = parse_message(argv[2]);

		printf("From: %s\n\
		        Date: %s\n\n\
		        %s\n\n\
		        Attachment: ", msg->from, msg->subject, msg->body);

		for (int i = 0; msg->attachments[i] != NULL; ++i)
			printf("%s ", msg->attachments[i]);

		puts("\n");
		free(msg);
	} else {
		fprintf(stderr, "Error: invalid argument\n");
		return -1;
	}

	return 0;
}
