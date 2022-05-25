/* Common cURL networking functions. */
#include <curl/curl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "net_requests.h"

// Helper function to work with domains and paths.
CURLU *domain_with_path(char *domain, char *path) {
  CURLU *url = curl_url();
  curl_url_set(url, CURLUPART_URL, domain, 0);
  curl_url_set(url, CURLUPART_PATH, path, 0);
  return url;
}

// Determines which hardcoded URL should be utilized.
// As a repository domain cannot be determined prior to
// loading a list, this function should be used while intitializing.
CURLU *create_hardcoded_url(char *path) {
  if (codemii_backup == false && setting_server == false) {
    return domain_with_path(MAIN_DOMAIN, path);
  } else {
    return domain_with_path(FALLBACK_DOMAIN, path);
  }
}

// Adds a path to the used repository URL.
CURLU *create_repo_url(char *path) {
  // Determine what server we want to request with.
  if (setting_repo == 0) {
    // If the user has specified an alternative repo,
    // we should use its specified domain.
    return domain_with_path(repo_list[setting_repo].domain, path);
  } else {
    // Defer to our default logic.
    return create_hardcoded_url(path);
  }
}

// Adds a query parameter to a CURLU.
void curl_set_query(CURLU *url, char *name, char *value) {
  // Allocate space for name + '=' + value + '\0'
  int size = strlen(name) + strlen(value) + 2;
  char *query_param = (char *)malloc(size);

  // Set the query parameter.
  sprintf(query_param, "%s=%s", name, value);

  curl_url_set(url, CURLUPART_QUERY, query_param, CURLU_APPENDQUERY);

  free(query_param);
}

// Taken from https://curl.se/libcurl/c/getinmemory.html.
struct MemoryStruct {
  char *memory;
  size_t size;
};

// Similarly taken from https://curl.se/libcurl/c/getinmemory.html.
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb,
                                  void *userp) {
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;

  char *ptr = (char *)realloc(mem->memory, mem->size + realsize + 1);
  if (!ptr) {
    /* out of memory! */
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }

  mem->memory = ptr;
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;

  return realsize;
}

// Performs a request to the given URL.
void *handle_get_request(CURLU *url, CURLcode *error) {
  // Allocate our write handler.
  struct MemoryStruct chunk;
  chunk.memory = (char *)malloc(4);
  chunk.size = 0;

  // Set defaults.
  CURL *curl_handle = curl_easy_init();
  // Assume HTTP if a scheme is not specified.
  curl_easy_setopt(curl_handle, CURLOPT_DEFAULT_PROTOCOL, "http");
  curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, USER_AGENT);
  curl_easy_setopt(curl_handle, CURLOPT_CURLU, url);
  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
  curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);

  // Perform!
  CURLcode res = curl_easy_perform(curl_handle);
  // Check if we should store the error.
  if (error != NULL) {
    *error = res;
  }

  if (res != CURLE_OK) {
    return NULL;
  }

  // Clean up our URL and handle.
  curl_url_cleanup(url);
  curl_easy_cleanup(curl_handle);

  return chunk.memory;
}

// Performs a GET request to <configured hbb repo>/<path>.
void *get_request(char *path, CURLcode *error) {
  CURLU *url = create_repo_url(path);
  return handle_get_request(url, error);
}