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
void append(Address **, const char *, bool);

void
create_addr(Address **head, char *addr)
{
	if (!strchr(addr, '@') || !strchr(addr, '.')) {
		fprintf(stderr, "Error: the address \"%s\" is invalid", addr);
		return;
	}

	bool is_good = false;

	char *blacklist_addr[] = { "abuse",      "webmaster",  "contact",
		                   "postmaster", "hostmaster", "admin" };
	char **avail_domains = get_domains();

	bool good_addr = true;
	bool good_domain = false;

	int track_index = 0;
	int before_at = 1;
	char name[strlen(addr)];
	char domain[strlen(addr)];

	for (int i = 0; i < strlen(addr); ++i) {
		if (addr[i] != '@' && before_at) {
			name[track_index] = addr[i];
			++track_index;
		} else if (!before_at) {
			domain[track_index] = addr[i];
			++track_index;
		} else {
			name[track_index] = '\0';
			before_at = 0;
			track_index = 0;
			continue;
		}
	}
	domain[track_index] = '\0';

	for (int i = 0; i < (sizeof(blacklist_addr) / sizeof(blacklist_addr[0])); ++i) {
		if (!strcmp(name, blacklist_addr[i]))
			good_addr = false;
	}

	for (int i = 0; avail_domains[i] != NULL; ++i) {
		if (!strcmp(domain, avail_domains[i]))
			good_domain = true;
	}

	if (good_addr && good_domain) {
		is_good = true;
	} else if (!good_addr && good_domain) {
		fprintf(stderr, "Error: the address \"%s\" is invalid", name);
		return;
	} else if (good_addr && !good_domain) {
		fprintf(stderr, "Error: the domain \"%s\" is invalid", domain);
		return;
	} else if (!good_addr && !good_domain) {
		fprintf(stderr, "Error: the address \"%s\" is invalid\nError: the domain \"%s\" is invalid",
		        name, domain);
		return;
	}

	if (is_good)
		append(head, addr, true);

	free(avail_domains);
}


const char *
parse_addr(void) {
	struct stat st = { 0 };

	char *xdg_path = getenv("XDG_CONFIG_HOME");
	char *conf_dir = (char *)malloc(sizeof(char) * 
	                 (strlen(xdg_path) + strlen("/ctm/current_address.log") + 1));

	strcpy(conf_dir, xdg_path);
	strcat(conf_dir, "/ctm");

	if (stat(conf_dir, &st) == -1) {
		mkdir(conf_dir, 0700);
		free(conf_dir);
		return NULL;
	}

	char *log_file = strcat(conf_dir, "/current_address.log");

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

void
append(Address **head, const char *addr, bool selected)
{
	Address *check = *head;

	while (check != NULL) {
		if (!strcmp(check->addr, addr))
			return;

		check = check->next;
	}

	Address *new = (Address *)malloc(sizeof(Address));
	Address *current = *head;

	new->is_selected = selected;
	new->addr = addr;
	new->next = NULL;

	if (*head == NULL) {
		*head = new;
		return;
	}

	while (current->next != NULL) {
		if (!strcmp(current->addr, addr))
			return;

		current = current->next;
	}

	current->next = new;
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
