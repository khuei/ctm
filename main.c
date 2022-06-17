#include <stdio.h>
#include <string.h>

#include <curl/curl.h>

#include "address.h"

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
		store_addr(email_addr);
	} else if (!strcmp(argv[1], "current")) {
		const char *current_addr = parse_addr();
		
		if (current_addr != NULL) {
			printf("%s\n", parse_addr());
		} else {
			fprintf(stderr, "Error: unable to get current email address\n");
			return -1;
		}
	} else {
		fprintf(stderr, "Error: invalid argument\n");
		return -1;
	}

	return 0;
}