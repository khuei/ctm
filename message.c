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
	msg->body = json_object_get_string(json_object_object_get(root, "textBody"));

	char *base_attm_url = "https://www.1secmail.com/api/v1/?action=download&login=";
	char *attm_url = NULL;
	json_object *attachments = json_object_object_get(root, "attachments");
	json_object *attachment = NULL;
	char *filename = NULL;
	char *filetype = NULL;
	int array_len = (int)json_object_array_length(attachments);

	for (int i = 0; i < array_len; ++i) {
		attachment = json_object_array_get_idx(attachments, i);

		filename = realloc(filename, sizeof(char) * strlen(json_object_get_string(json_object_object_get(attachment, "filename"))));
		filename = (char *)json_object_get_string(json_object_object_get(attachment, "filename"));

		attm_url = (char *)malloc(sizeof(char) * 
		                          (strlen(base_attm_url) + strlen(name) +
		                          strlen("&domain=") + strlen(domain) +
		                          strlen("&id=") + strlen(id) +
		                          strlen("&file=") +
		                          strlen(filename)));

		sprintf(attm_url, "%s%s&domain=%s&id=%s&file=%s",
		        base_attm_url, name, domain, id, filename);

		char current_dir[4096];
		getcwd(current_dir, sizeof(current_dir));

		chdir(log_dir);

		if (stat(filename, &st) == 0)
			get_parsed_json(attm_url);

		filetype = get_filetype(filename);
		chdir(current_dir);

		if(strcmp(filetype, "cannot")) {
			msg->attachments[i] = (char *)malloc(sizeof(char) *
			                                     (strlen(filename) +
			                                     strlen(filetype) + strlen(" []")));

			sprintf(msg->attachments[i], "%s [%s]", filename, filetype);
		} else {
			msg->attachments[i] = (char *)malloc(sizeof(char) *
			                                     strlen(filename));
			sprintf(msg->attachments[i], "%s", filename);
		}

		free(filename);
		free(filetype);
	}
	msg->attachments[array_len] = NULL;

	json_object_put(attachment);
	json_object_put(attachments);

	free(conf_dir);
	free(attm_url);

	return msg;
}

char *get_filetype(const char *filename) {
	FILE *pf = NULL;
	char *command = NULL;
	char *output = NULL;
	char data[2048];

	command = (char *)malloc(sizeof(char) * (strlen("file --mimetype ") +
	                         strlen(filename) + strlen(" | cut -d ' ' -f1")));

	sprintf(command, "file --mimetype %s | cut -d ' ' -f2", filename);

	pf = popen(command, "r");
	free(command);

	if (pf != NULL) {
		fgets(data, 2048, pf);

		for (int i = 0; i < strlen(data); ++i)
			if (data[i] == ' ' && data[i + 1] == ' ')
				data[i] = '\0';

		output = (char *)malloc(sizeof(char) * strlen(data));

		strcpy(output, data);

		pclose(pf);
	}

	return output;
}
