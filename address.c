#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ftw.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <json-c/json_object.h>
#include <json-c/json_tokener.h>

#include "json.h"
#include "address.h"

char **get_domains(void);

const char *
create_addr(void)
{
	char *api_url = "https://www.1secmail.com/api/v1/?action=genRandomMailbox&count=1";
	json_object *array = NULL;
	json_object *element;
	const char *element_str;

	parsed_json addr_json = get_parsed_json(api_url);
	array = json_tokener_parse(addr_json.ptr);
	
	element = json_object_array_get_idx(array, 0);
	element_str = json_object_get_string(element);

	struct stat st = { 0 };

	char *xdg_path = getenv("XDG_CONFIG_HOME");
	char *conf_dir = (char *)malloc(sizeof(char) * 
	                 (strlen(xdg_path) + strlen("/ctm/address.log") + 1));

	strcpy(conf_dir, xdg_path);
	strcat(conf_dir, "/ctm");

	if (stat(conf_dir, &st) == -1)
		mkdir(conf_dir, 0700);

	char *log_file = strcat(conf_dir, "/address.log");

	FILE *file = fopen(log_file, "w");
	if (file != NULL) {
		fprintf(file, "%s\n", element_str);
		fclose(file);
	}

	json_object_put(array);
	json_object_put(element);
	free(addr_json.ptr);
	free(conf_dir);

	return element_str;
}


const char *
parse_addr(void) {
	struct stat st = { 0 };

	char *xdg_path = getenv("XDG_CONFIG_HOME");
	char *conf_dir = (char *)malloc(sizeof(char) * 
	                 (strlen(xdg_path) + strlen("/ctm/address.log") + 1));

	strcpy(conf_dir, xdg_path);
	strcat(conf_dir, "/ctm");

	if (stat(conf_dir, &st) == -1) {
		mkdir(conf_dir, 0700);
		free(conf_dir);
		return NULL;
	}

	char *log_file = strcat(conf_dir, "/address.log");

	FILE *file = fopen(log_file, "r");
	char *line = NULL;
	size_t len = 0;

	if (file != NULL) {
		getline(&line, &len, file);

		if (line[strlen(line) - 1] == '\n')
			line[strlen(line) - 1] = '\0';

		fclose(file);
	}

	free(conf_dir);

	return line;
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
	json_object_put(element);
	json_object_put(array);

	return domains;
}

int unlink_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
	return remove(fpath);
}

int
clear_log(void)
{
	const char *email_addr = parse_addr();
	char *xdg_path = getenv("XDG_CONFIG_HOME");
	char *log_dir = (char *)malloc(sizeof(char) * (strlen(xdg_path) +
	                                                strlen("/ctm/") +
	                                                strlen(email_addr) + 1));

	strcpy(log_dir, xdg_path);
	strcat(log_dir, "/ctm/");
	strcat(log_dir, email_addr);

	int rm_msg = nftw(log_dir, unlink_cb, 64, FTW_DEPTH |  FTW_PHYS);

	free(log_dir);

	if (!rm_msg)
		return 0;
	else
		return -1;
}
