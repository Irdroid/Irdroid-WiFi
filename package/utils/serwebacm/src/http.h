#ifndef HTTP_H
#define HTTP_H

int http_generate_headers(char *f, int status, char *title, int length);
int http_handle_get( char *f, char *out_buf );

#endif