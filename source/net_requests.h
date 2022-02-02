/* Common cURL networking functions. */
#include <curl/curl.h>

#define USER_AGENT "OSC Homebrew Browser/1.0"

// Used to identify whether the backup server
// is being utilized.
extern bool codemii_backup;

CURLU* create_repo_url(char* path);
void curl_set_query(CURLU* url, char* name, char* value);
void* handle_get_request(CURLU* url, CURLcode* error);
void* get_request(char* path, CURLcode* error);