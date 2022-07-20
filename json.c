#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include <curl/curl.h>

#include "json.h"

static void init_json_struct(parsed_json *);
static size_t write_json_struct(void *, size_t, size_t, parsed_json *);

parsed_json
get_parsed_json(const char *url)
{
	parsed_json s;

	init_json_struct(&s);

	CURL *handle = curl_easy_init();

	curl_easy_setopt(handle, CURLOPT_URL, url);
	curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_json_struct);
	curl_easy_setopt(handle, CURLOPT_WRITEDATA, &s);
	curl_easy_perform(handle);
	curl_easy_cleanup(handle);

	return s;
}

void
init_json_struct(parsed_json *s)
{
	s->len = 0;
	s->ptr = (char *)malloc(s->len + 1);
	s->ptr[0] = '\0';
}

size_t
write_json_struct(void *ptr, size_t size, size_t nmemb, parsed_json *s)
{
	size_t new_len = s->len + size * nmemb;
	s->ptr = (char *)realloc(s->ptr, new_len + 1);
	memcpy(s->ptr + s->len, ptr, size * nmemb);
	s->ptr[new_len] = '\0';
	s->len = new_len;

	return size * nmemb;
}
