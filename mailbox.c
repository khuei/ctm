#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <json-c/json_object.h>
#include <json-c/json_tokener.h>
#include <json-c/json_util.h>

#include "json.h"
#include "address.h"
#include "mailbox.h"

static void append(Mail **, const char *, const char *, const char *, const char *);

void
retrieve_mailbox(void)
{
	Mail *head = NULL;

	char *base_url = "https://www.1secmail.com/api/v1/?action=getMessages&login=";
	char *api_url = NULL;

	const char *email_addr = parse_addr();
	char name[strlen(email_addr)];
	char domain[strlen(email_addr)];

	int track_index = 0;
	int before_atsign = 1;

	for (int i = 0; i < strlen(email_addr); ++i) {
		if (email_addr[i] != '@' && before_atsign) {
			name[track_index] = email_addr[i];
			++track_index;
		} else if (!before_atsign) {
			domain[track_index] = email_addr[i];
			++track_index;
		} else {
			name[track_index] = '\0';
			before_atsign = 0;
			track_index = 0;
			continue;
		}
	}
	domain[track_index] = '\0';

	api_url = (char *)malloc((strlen(base_url) + strlen(name) + 
	                          strlen(domain) + strlen("&domain=")) * sizeof(char));

	sprintf(api_url, "%s%s&domain=%s", base_url, name, domain);

	parsed_json mailbox_json = get_parsed_json(api_url);
	json_object *array = json_tokener_parse(mailbox_json.ptr);
	int array_len = (int)json_object_array_length(array);

	if (array_len == 0) {
		printf("Notice: Mailbox is empty\n");
		return;
	}

	struct stat st = { 0 };

	char *xdg_path = getenv("XDG_CONFIG_HOME");
	char *conf_dir = (char *)malloc(sizeof(char) * 
	                 (strlen(xdg_path) + strlen("/ctm/mailbox.log") + 1));

	strcpy(conf_dir, xdg_path);
	strcat(conf_dir, "/ctm");

	if (stat(conf_dir, &st) == -1)
		mkdir(conf_dir, 0700);

	char *log_file = strcat(conf_dir, "/mailbox.log");

	FILE *file = fopen(log_file, "w");

	if (file != NULL) {
		fprintf(file, "%s\n", json_object_get_string(array));
		fclose(file);
	}

	json_object_put(array);
	free(api_url);
	free(mailbox_json.ptr);
	free(conf_dir);
}

Mail *
parse_mailbox(void)
{
	struct stat st = { 0 };

	char *xdg_path = getenv("XDG_CONFIG_HOME");
	char *conf_dir = (char *)malloc(sizeof(char) * 
	                 (strlen(xdg_path) + strlen("/ctm/mailbox.log") + 1));

	strcpy(conf_dir, xdg_path);
	strcat(conf_dir, "/ctm");

	if (stat(conf_dir, &st) == -1) {
		mkdir(conf_dir, 0700);
		free(conf_dir);
		return NULL;
	}

	char *log_file = strcat(conf_dir, "/mailbox.log");

	Mail *head = NULL;
	json_object *array = NULL;
	json_object *element = NULL;
	json_object *id = NULL;
	json_object *from = NULL;
	json_object *subject = NULL;
	json_object *date = NULL;
	const char *element_str = NULL;
	int array_len = 0;

	array = json_object_from_file(log_file);
	array_len = (int)json_object_array_length(array);

	for (int i = 0; i < array_len; ++i) {
		element = json_object_array_get_idx(array, i);

		id = json_object_object_get(element, "id");
		from = json_object_object_get(element, "from");
		subject = json_object_object_get(element, "subject");
		date = json_object_object_get(element, "date");
		
		append(&head,
		       json_object_get_string(id),
		       json_object_get_string(from),
		       json_object_get_string(subject),
		       json_object_get_string(date));
	}

	json_object_put(element);
	json_object_put(array);
	json_object_put(id);
	json_object_put(from);
	json_object_put(subject);
	json_object_put(date);
	free(conf_dir);

	return head;
}

void
append(Mail **head, const char *id, const char *from,
       const char *subject, const char *date)
{
	Mail *new = (Mail *)malloc(sizeof(Mail));
	Mail *current = *head;

	new->id = id;
	new->from = from;
	new->subject = subject;
	new->date = date;
	new->next = NULL;

	if (*head == NULL) {
		*head = new;
		return;
	}

	while (current->next != NULL) {
		current = current->next;
	}

	current->next = new;
}
