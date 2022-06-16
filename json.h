#pragma once

typedef struct parsed_json {
	char *ptr;
	size_t len;
} parsed_json;

typedef struct json_object json_object;

parsed_json get_parsed_json(const char *);
