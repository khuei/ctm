#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <json-c/json_object.h>
#include <json-c/json_tokener.h>
#include <json-c/json_util.h>

#include "json.h"
#include "address.h"
#include "message.h"

Message *
parse_message(char *id)
{
	struct stat st = { 0 };

	char *xdg_path = getenv("XDG_CONFIG_HOME");
	char *conf_dir = (char *)malloc(sizeof(char) * 
	                 (strlen(xdg_path) + strlen("/ctm/message/") +
	                  strlen(id) + strlen("/text.log") + 1));

	strcpy(conf_dir, xdg_path);
	strcat(conf_dir, "/ctm");

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

	Message *msg = NULL;
	json_object *root = NULL;
	json_object *from = NULL;
	json_object *subject = NULL;
	json_object *date = NULL;
	json_object *attachments = NULL;
	json_object *body = NULL;

	if (file) {
		root = json_object_from_file(log_file);
		fclose(file);
	} else {
		char *base_url = "https://www.1secmail.com/api/v1/?action=readMessage&login=";
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

		api_url = (char *)malloc(sizeof(char) * (strlen(base_url) + strlen(name) + 
		          strlen(domain) + strlen("&domain=") + strlen("&id=") + strlen(id)));

		snprintf(api_url, sizeof(api_url), "%s%s&domain=%s&id=%s", base_url, name, domain, id);

		parsed_json message_json = get_parsed_json(api_url);
		root = json_tokener_parse(message_json.ptr);
		free(message_json.ptr);
	}

	from = json_object_object_get(root, "from");
	subject = json_object_object_get(root, "subject");
	date = json_object_object_get(root, "date");
	attachments = json_object_object_get(root, "attachments");
	body = json_object_object_get(root, "body");

	msg->id = id;
	msg->from = json_object_get_string(from);
	msg->subject = json_object_get_string(subject);
	msg->date = json_object_get_string(date);
	msg->attachments = json_object_get_string(attachments);
	msg->body = json_object_get_string(body);

	from = json_object_object_get(root, "from");
	subject = json_object_object_get(root, "subject");
	date = json_object_object_get(root, "date");
	attachments = json_object_object_get(root, "attachments");
	body = json_object_object_get(root, "body");


	json_object_put(root);
	json_object_put(from);
	json_object_put(subject);
	json_object_put(date);
	json_object_put(attachments);
	json_object_put(body);
	free(conf_dir);

	return msg;
}
