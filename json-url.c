#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <curl/curl.h>
#include <json-c/json.h>

#define TEST_URL "http://httpbin.org/get"

struct json_ctx {
  json_object *obj;
  json_tokener *tok;
};

static size_t write_cb(char *ptr, size_t size, size_t nmemb, void *opq);

int
main(int argc, char *argv[])
{
  int rv = 0;
  const char *url;
  enum json_tokener_error jerr;
  struct json_ctx ctx = { NULL, json_tokener_new() };
  CURL *curl;
  CURLcode res;

  assert(ctx.tok);

  curl_global_init(CURL_GLOBAL_ALL);
  curl = curl_easy_init();

  switch (argc) {
    case 2:
      url = argv[1];
      break;
    default:
      url = TEST_URL;
      break;
  }

  curl_easy_setopt(curl, CURLOPT_URL, url);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) &ctx);

  res = curl_easy_perform(curl);
  if (res != CURLE_OK) {
    rv = 1;
    fprintf(stderr, "error: %s\n", curl_easy_strerror(res));
    goto done;
  }

  if ((jerr = json_tokener_get_error(ctx.tok)) != json_tokener_success) {
    rv = 2;
    fprintf(stderr, "json error: %s\n", json_tokener_error_desc(jerr));
    goto done;
  }

  printf("%s\n", json_object_to_json_string(ctx.obj));

done:
  curl_easy_cleanup(curl);
  curl_global_cleanup();

  if (ctx.tok) {
    json_tokener_free(ctx.tok);
  }
  if (ctx.obj) {
    /* strange name for something that decrements a reference counter :) */
    json_object_put(ctx.obj);
  }

  return rv;
}

static size_t
write_cb(char *ptr, size_t size, size_t nmemb, void *opq)
{
  size_t len = size * nmemb;
  struct json_ctx *ctx = (struct json_ctx *) opq;
  ctx->obj = json_tokener_parse_ex(ctx->tok, ptr, len);
  switch (json_tokener_get_error(ctx->tok)) {
    case json_tokener_continue:
    case json_tokener_success:
      return len;
    default:
      return 0; /* error: interrupt the transfer */
  }
}
