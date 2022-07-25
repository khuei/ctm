#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <ftw.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <json-c/json_object.h>
#include <json-c/json_tokener.h>

#include "json.h"
#include "address.h"

static char **get_domains(void);
static int write_current_addr(const char *);
static int append(Address **, const char *);
static int append_b(Address **, const char *, bool);
static bool is_number(const char []);
static int unlink_cb(const char *, const struct stat *, int, struct FTW *);

int
create_addr(Address **head, char *addr)
{
	if (!strchr(addr, '@') || !strchr(addr, '.')) {
		fprintf(stderr, "Error: the address \"%s\" is invalid\n", addr);
		return -1;
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
		fprintf(stderr, "Error: the address \"%s\" is invalid\n", name);
		return -1;
	} else if (good_addr && !good_domain) {
		fprintf(stderr, "Error: the domain \"%s\" is invalid\n", domain);
		return -1;
	} else if (!good_addr && !good_domain) {
		fprintf(stderr, "Error: the address \"%s\" is invalid\nError: the domain \"%s\" is invalid\n",
		        name, domain);
		return -1;
	}

	if (is_good) {
		if(!append(head, addr))
			return -1;

		if(!write_current_addr(addr))
			return -1;
	}

	free(avail_domains);

	return 0;
}

int
create_rand_addr(Address **head, int num)
{
	char *base_url = "https://www.1secmail.com/api/v1/?action=genRandomMailbox&count=";
	char *api_url = NULL;

	json_object *array = NULL;
	json_object *element = NULL;
	const char *element_str = NULL;
	int array_len = 0;

	api_url = (char *)malloc((strlen(base_url) + (int)log(num) + 1) * sizeof(char));
	sprintf(api_url, "%s%d", base_url, num);

	parsed_json emails_json = get_parsed_json(api_url);

	array = json_tokener_parse(emails_json.ptr);
	array_len = (int)json_object_array_length(array);

	for (int i = 0; i < array_len; ++i) {
		element = json_object_array_get_idx(array, i);
		element_str = json_object_get_string(element);

		append(head, element_str);

		if (i == (array_len - 1))
			write_current_addr(element_str);
	}

	free(api_url);
	free(emails_json.ptr);

	return 0;
}

Address *
parse_addr(void)
{
	Address *head = NULL;

	char *xdg_path = getenv("XDG_CONFIG_HOME");
	char *log_file = (char *)malloc(sizeof(char) * 
	                 (strlen(xdg_path) + strlen("/ctm/address.log") + 1));
	strcpy(log_file, xdg_path);
	strcat(log_file, "/ctm/address.log");

	FILE *file = fopen(log_file, "r");

	if (file != NULL) {
		char *line = NULL;
		size_t len = 0;

		while (getline(&line, &len, file) != -1) {
			line[strcspn(line, "\r\n")] = '\0';

			char *addr = NULL;
			char *is_selected = NULL;

			addr = strtok(line, " ");
			is_selected = strtok(NULL, " ");

			append_b(&head, addr, strtol(is_selected, NULL, 10));
			line = NULL;
		}

		fclose(file);
	}

	free(log_file);

	return head;
}

const char *
parse_current_addr(void) {
	struct stat st = { 0 };

	char *xdg_path = getenv("XDG_CONFIG_HOME");
	char *conf_dir = (char *)malloc(sizeof(char) * 
	                 (strlen(xdg_path) + strlen("/ctm/current_address.log") + 1));

	strcpy(conf_dir, xdg_path);
	strcat(conf_dir, "/ctm");

	if (stat(conf_dir, &st) == -1) {
		mkdir(conf_dir, 0700);
		free(conf_dir);
		fprintf(stderr, "Warning: address log is empty\n");
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

int
write_current_addr(const char *addr)
{
	struct stat st = { 0 };

	char *xdg_path = getenv("XDG_CONFIG_HOME");
	char *conf_dir = (char *)malloc(sizeof(char) * 
	                 (strlen(xdg_path) + strlen("/ctm/current_address.log") + 1));
	strcpy(conf_dir, xdg_path);
	strcat(conf_dir, "/ctm");

	if (stat(conf_dir, &st) == -1)
		mkdir(conf_dir, 0700);

	char *log_file = strcat(conf_dir, "/current_address.log");

	FILE *file = fopen(log_file, "w");

	if (file != NULL) {
		fprintf(file, "%s\n", addr);
		fclose(file);
	} else {
		fprintf(stderr, "Error: unable to log current address\n");
		return -1;
	}

	free(conf_dir);

	return 0;
}

int
append(Address **head, const char *addr)
{
	Address *check = *head;

	while (check != NULL) {
		if (!strcmp(check->addr, addr)) {
			fprintf(stderr, "Error: address already exists\n");
			return -1;
		}

		check = check->next;
	}

	Address *new = (Address *)malloc(sizeof(Address));
	Address *current = *head;

	new->is_selected = true;
	new->addr = addr;
	new->next = NULL;

	if (*head == NULL) {
		*head = new;
		return 0;
	}

	while (current->next != NULL) {
		current->is_selected = false;
		current = current->next;
	}
	current->is_selected = false;

	current->next = new;

	return 0;
}

int
append_b(Address **head, const char *addr, bool is_selected)
{
	Address *check = *head;

	while (check != NULL) {
		if (!strcmp(check->addr, addr)) {
			fprintf(stderr, "Error: address already exists\n");
			return -1;
		}

		check = check->next;
	}

	Address *new = (Address *)malloc(sizeof(Address));
	Address *current = *head;

	new->is_selected = is_selected;
	new->addr = addr;
	new->next = NULL;

	if (*head == NULL) {
		*head = new;
		return 0;
	}

	while (current->next != NULL) {
		current->is_selected = false;
		current = current->next;
	}
	current->is_selected = false;

	current->next = new;

	return 0;
}

int
select_addr(Address **head, const char *input) {
	if (*head == NULL) {
		fprintf(stderr, "Error: address list is empty\n");
		return -1;
	}

	Address *current = *head;
	const char *email_addr = NULL;

	if (!is_number(input)) {
		bool has_addr = false;
		while (current != NULL) {
			if (!strcmp(current->addr, input))
				has_addr = true;
		}

		if (!has_addr) {
			fprintf(stderr, "Error: address does not exist\n");
			return -1;
		}

		while (current != NULL) {
			if (!strcmp(current->addr, input))
				current->is_selected = true;
			else
				current->is_selected = false;

			current = current->next;
		}
	} else {
		for (int i = 1; current != NULL; ++i) {
			if (i == strtol(input, NULL, 10)) {
				current->is_selected = true;
				email_addr = current->addr;
			} else {
				current->is_selected = false;
			}

			current = current->next;
		}

		if (!email_addr) {
			fprintf(stderr, "Error: address does not exist\n");
			return -1;
		}
	}

	if(!write_current_addr(email_addr))
		return -1;

	return 0;
}

int unlink_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
	return remove(fpath);
}

int
delete_addr(Address **head, const char *input)
{
	if (*head == NULL) {
		fprintf(stderr, "Error: address list is empty\n");
		return -1;
	}

	Address *prev = NULL;
	Address *current = *head;
	Address *next = current->next;

	const char *email_addr = current->addr;;
	char *xdg_path = getenv("XDG_CONFIG_HOME");
	char *log_dir = (char *)malloc(sizeof(char) * (strlen(xdg_path) +
	                                               strlen("/ctm/") +
	                                               strlen(email_addr) + 1));
	strcpy(log_dir, xdg_path);
	strcat(log_dir, "/ctm/");
	strcat(log_dir, email_addr);
	nftw(log_dir, unlink_cb, 64, FTW_DEPTH |  FTW_PHYS);
	free(log_dir);

	if (!is_number(input)) {
		if (current != NULL && !strcmp(current->addr, input)) {
			*head = current->next;
			free(current);
			return 0;
		}

		while (current != NULL && strcmp(current->addr, input)) {
			prev = current;
			current = current->next;
		}

		if (current == NULL) {
			fprintf(stderr, "Error: address does not exist\n");
			return -1;
		}

		prev->next = current->next;

	} else {
		if (strtol(input, NULL, 10) == 1) {
			*head = current->next;
		} else {
			for (int i = 2; current != NULL && 
			     i <= strtol(input, NULL, 10); ++i) {
				prev = current;
				current = current->next;
			}

			if (current == NULL) {
				fprintf(stderr, "Error: address does not exist\n");
				return -1;
			}

			prev->next = current->next;
		}
	}

	free(current);

	return 0;
}

int
store_addr(Address **head)
{
		if (*head == NULL)
			return 0;

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

		Address *current = *head;
		Address *prev = NULL;
		Address *next = NULL;

		if (file != NULL) {
			while (current != NULL) {
				fprintf(file, "%s %d\n", current->addr, current->is_selected);
				next = current->next;
				prev = current;
				current = next;
				free(prev);
			}
			free(current);

			fclose(file);
		} else {
			fprintf(stderr, "Error: unable to log address list\n");
			return -1;
		}

		free(conf_dir);

		return 0;
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

	domains[array_len] = NULL;

	free(domains_json.ptr);

	return domains;
}

bool
is_number(const char number[])
{
	int i = 0;

	if (number[0] == '-')
		i = 1;

	for (; number[i] != 0; i++) {
		if (!isdigit(number[i]))
			return -1;
	}

	return 0;
}
