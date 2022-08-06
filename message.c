#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include <json-c/json_object.h>
#include <json-c/json_tokener.h>
#include <json-c/json_util.h>

#include "json.h"
#include "address.h"
#include "message.h"

static char *get_filetype(const char *);

Message *
parse_message(char *id)
{
	struct stat st = { 0 };

	const char *email_addr = parse_current_addr();
	char *xdg_path = getenv("XDG_CONFIG_HOME");
	char *conf_dir = (char *)malloc(sizeof(char) * 
	                 (strlen(xdg_path) + strlen("/ctm/message/") +
	                  strlen(email_addr) + strlen(id) + strlen("/text.log")
	                  + 2));

	strcpy(conf_dir, xdg_path);
	strcat(conf_dir, "/ctm/");
	strcat(conf_dir, email_addr);

	if (stat(conf_dir, &st) == -1)
		mkdir(conf_dir, 0700);

	char *log_dir = strcat(conf_dir, "/message/");

	if (stat(log_dir, &st) == -1)
		mkdir(log_dir, 0700);

	char *msg_logdir = strcat(conf_dir, id);

	if (stat(msg_logdir, &st) == -1)
		mkdir(msg_logdir, 0700);

	char *log_file = strcat(msg_logdir, "/text.log");

	FILE *file = fopen(log_file, "r");

	Message *msg = (Message *)malloc(sizeof(Message));
	json_object *root = NULL;

	char name[strlen(email_addr)];
	char domain[strlen(email_addr)];

	if (file) {
		root = json_object_from_file(log_file);
		fclose(file);
	} else {
		char *base_url = "https://www.1secmail.com/api/v1/?action=readMessage&login=";
		char *api_url = NULL;

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

		api_url = (char *)malloc(sizeof(char) * (strlen(base_url) + strlen(name) + 
		          strlen(domain) + strlen("&domain=") + strlen("&id=") + strlen(id)));

		sprintf(api_url, "%s%s&domain=%s&id=%s", base_url, name, domain, id);

		parsed_json message_json = get_parsed_json(api_url);

		if (message_json.len == 0) {
			fprintf(stderr, "Error: failed to connect to 1secMail API\n");
			return NULL;
		}

		root = json_tokener_parse(message_json.ptr);

		if (root != NULL) {
			file = fopen(log_file, "w");

			if (file) {
				fprintf(file, "%s\n", json_object_get_string(root));
				fclose(file);
			}
		} else {
			return NULL;
		}

		free(message_json.ptr);
		free(api_url);
	}

	msg->id = id;
	msg->from = json_object_get_string(json_object_object_get(root, "from"));
	msg->subject = json_object_get_string(json_object_object_get(root, "subject"));
	msg->date = json_object_get_string(json_object_object_get(root, "date"));

	const char *textBody = json_object_get_string(json_object_object_get(root, "textBody"));
	const char *htmlBody = json_object_get_string(json_object_object_get(root, "htmlBody"));

	if (htmlBody != NULL && htmlBody[0] != '\0')
		msg->body = htmlBody;
	else
		msg->body = textBody;

	json_object *attachments = json_object_object_get(root, "attachments");
	json_object *attachment = NULL;
	int array_len = (int)json_object_array_length(attachments);

	for (int i = 0; i < array_len; ++i) {
		attachment = json_object_array_get_idx(attachments, i);
		msg->attachments[i] = (char *)json_object_get_string(json_object_object_get(attachment, "filename"));
	}
	msg->attachments[array_len] = NULL;

	free(conf_dir);

	return msg;
}
