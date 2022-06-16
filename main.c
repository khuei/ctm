#include <stdio.h>
#include <string.h>

#include <curl/curl.h>

#include "address.h"

int
main(int argc, char *argv[])
{
	curl_global_init(CURL_GLOBAL_SSL);

	if(argc == 1) {
		fprintf(stderr, "Error: empty command");
		return -1;
	}

	if (!strcmp(argv[1], "create")) {
		const char *email_addr = create_addr();
	}

	return 0;
}
