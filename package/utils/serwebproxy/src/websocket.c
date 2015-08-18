/*
 * websocket.c
 *
 * serial port <-> TCP/IP piping, WebSocket protocol functions
 *
 * (C)2012 serwebproxy 1.1, Lari Temmes
 * 
 * Added to serwebproxy 1.1, most if this stuff was hacked to main.c
 * 
 */
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include "websocket.h"
#include "error.h"

	char combined_key[] = "------------------------258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
	char *token[120];		// max lines in header
	char client_key[24];

int websocket_generate_key( char *f ){

	char	*temp;
	int		i = 0, j = 0;

	DBG_MSG2("server(%d), got WebSocket header from client.\n", port);

	token[0] = strtok(f,"\r\n");
	while(token[i]!= NULL) {   //ensure a pointer was found
			i++;
			token[i] = strtok(NULL, "\r\n"); //continue to tokenize the string
	}
	for(j = 0; j <= i-1; j++) {
			temp = strstr( token[j], "Sec-WebSocket-Key: " );
			if( temp ){
				token[0] = strtok(temp, ": ");
				token[1] = strtok(NULL, ": ");
				memcpy( client_key, token[1], 24 );
				memcpy( combined_key, token[1], 24 );
				// fprintf(stderr, "Combined key is %s, size %d\n", combined_key, strlen( combined_key) ); 
				continue;
			}
			// else DBG_MSG2("headerline %s\n", token[j]); //print out all of the tokens
	}
	return 1;
}

int websocket_mask_data( char *f, char *out_buf, int count ){
	int		mask_index=0,i;
	char	mask_keys[4];

	mask_keys[0] = f[2];
	mask_keys[1] = f[3];
	mask_keys[2] = f[4];
	mask_keys[3] = f[5];

	for( i=0; i<=count; i++ ){
			out_buf[i] = f[i] ^ mask_keys[(mask_index-6) & 3];
			mask_index++;
	}
	return i;
}

int websocket_generate_headers( char *f ){

	unsigned char sha_digest[20];
	int	 message_len = 0;
	char *encoded;

	DBG_MSG2("server(%d), websocket_generate_headers.\n", port);

	SHA1( (char *)combined_key, strlen( combined_key ), sha_digest );
	encoded = base64( sha_digest, 20 );
					
	sprintf( f, "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: %s\r\n", encoded);

	message_len = strlen( f );

	return message_len;
}

char *base64(const unsigned char *input, int length){
  BIO *bmem, *b64;
  BUF_MEM *bptr;
  char *buff;

  b64 = BIO_new(BIO_f_base64());
  bmem = BIO_new(BIO_s_mem());
  b64 = BIO_push(b64, bmem);
  BIO_write(b64, input, length);
  BIO_flush(b64);
  BIO_get_mem_ptr(b64, &bptr);


  buff = (char *)malloc(bptr->length +1);
	memcpy(buff, bptr->data, bptr->length);
	buff[bptr->length] = 0;


  BIO_free_all(b64);

  return buff;
}