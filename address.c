#include <json-c/json.h>

#include "json.h"
#include "address.h"

char **get_domains(void);

const char *create_addr(void) {
	char *api_url = "https://www.1secmail.com/api/v1/?action=genRandomMailbox&count=1";
	json_object *array = NULL;
	json_object *element;
	const char *element_str;

	parsed_json addr_json = get_parsed_json(api_url);
	array = json_tokener_parse(addr_json.ptr);
	
	element = json_object_array_get_idx(array, 0);
	element_str = json_object_get_string(element);

	free(addr_json.ptr);

	return element_str;
}

char **
get_domains(void)
{
	char **domains = NULL;
	json_object *array = NULL;
	json_object *element = NULL;
	const char *element_str = NULL;
	int array_len = 0;

	parsed_json domains_json =
		get_parsed_json("https://www.1secmail.com/api/v1/?action=getDomainList");

	array = json_tokener_parse(domains_json.ptr);
	array_len = (int)json_object_array_length(array);

	domains = (char **)malloc(sizeof(domains_json.ptr));

	for (int i = 0; i < array_len; ++i) {
		element = json_object_array_get_idx(array, i);
		element_str = (char *)json_object_get_string(element);

		domains[i] = (char *)element_str;
	}

	domains[array_len + 1] = NULL;

	free(domains_json.ptr);

	return domains;
}
