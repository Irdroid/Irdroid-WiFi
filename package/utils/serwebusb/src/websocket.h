#ifndef WEBSOCKET_H
#define WEBSOCKET_H

int websocket_generate_key( char *f );
int websocket_mask_data( char *f, char *out_buf, int count );
int websocket_generate_headers( char *f );
char *base64(const unsigned char *input, int length);

#endif