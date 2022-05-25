/* Common cURL networking functions. */
#include <curl/curl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "net_requests.h"

// Helper function to work with domains and paths.
CURLU *domain_with_path(char *domain, char *path) {
  CURLU *url = curl_url();
  curl_url_set(url, CURLUPART_SCHEME, "http", 0);
  curl_url_set(url, CURLUPART_HOST, domain, 0);
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
    // Defer to our default logic.
    return create_hardcoded_url(path);
  } else {
    // If the user has specified an alternative repo,
    // we should use its specified domain.
    return domain_with_path(repo_list[setting_repo].domain, path);
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
static size_t curl_memory_callback(void *contents, size_t size, size_t nmemb,
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
  curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, USER_AGENT);
  curl_easy_setopt(curl_handle, CURLOPT_CURLU, url);
  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, curl_memory_callback);
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

// Writes the current received chunk from curl to the given file handle.
// We return the amount written so that curl can manage various errors for us.
static size_t curl_file_callback(void *ptr, size_t size, size_t chunk_size,
                                 void *data) {
  return fwrite(ptr, size, chunk_size, (FILE *)data);
}

// Notifies the passed function about callback state.
static size_t curl_file_progress(void *dl_cb, curl_off_t dl_total,
                                 curl_off_t dl_now, curl_off_t up_total,
                                 curl_off_t up_now) {
  // We only call with download progress, as we are not uploading.
  bool should_continue = ((download_callback *)dl_cb)(dl_now, dl_total);

  if (should_continue) {
    return 0;
  } else {
    return -1;
  }
}

// Downloads a file at the given URL to the given file handle.
// Remember to close the handle yourself upon completion.
CURLcode handle_download_request(CURLU *url, FILE *f,
                                 download_callback *dl_cb) {
  CURL *curl_handle = curl_easy_init();
  // Assume HTTP if a scheme is not specified.
  curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, USER_AGENT);
  curl_easy_setopt(curl_handle, CURLOPT_CURLU, url);
  // Though libcurl defaults to calling fwrite, it can't hurt to specify.
  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, curl_file_callback);
  curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, f);
  // Allow the invoking function to know our progress.
  curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 0);
  curl_easy_setopt(curl_handle, CURLOPT_XFERINFOFUNCTION, curl_file_progress);
  curl_easy_setopt(curl_handle, CURLOPT_XFERINFODATA, dl_cb);

  // Perform!
  CURLcode res = curl_easy_perform(curl_handle);

  // Success!
  curl_url_cleanup(url);
  curl_easy_cleanup(curl_handle);

  // Pass on any error we may have encountered.
  return res;
}

// Performs a HEAD request to the primary HBB list path, returning its response.
CURLcode test_head_request(CURLU *url) {
  CURL *curl_handle = curl_easy_init();
  curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, USER_AGENT);
  curl_easy_setopt(curl_handle, CURLOPT_CURLU, url);

  // We are a HEAD request.
  curl_easy_setopt(curl_handle, CURLOPT_NOBODY, 1);

  // We should not fail.
  curl_easy_setopt(curl_handle, CURLOPT_FAILONERROR, 1);
  // 5 seconds should be our maximum.
  curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 5);

  CURLcode res = curl_easy_perform(curl_handle);
  curl_url_cleanup(url);
  curl_easy_cleanup(curl_handle);
  return res;
}