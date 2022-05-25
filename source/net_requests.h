/* Common cURL networking functions. */
#include "common.h"
#include <curl/curl.h>

#define USER_AGENT "OSC Homebrew Browser/1.0"

// Determines whether the backup server should be utilized.
extern bool codemii_backup;

// TODO(spotlightishere): migrate into unified header
extern struct repo_struct repo_list[200];

CURLU *domain_with_path(char *domain, char *path);
CURLU *create_hardcoded_url(char *path);
CURLU *create_repo_url(char *path);
void curl_set_query(CURLU *url, char *name, char *value);
void *handle_get_request(CURLU *url, CURLcode *error);

// The definition for the callback function for download progress.
// Return false to cancel the request.
typedef bool download_callback(curl_off_t current_size, curl_off_t total_size);
CURLcode handle_download_request(CURLU *url, FILE *f,
                                 download_callback *download_callback);
CURLcode test_head_request(CURLU *url);