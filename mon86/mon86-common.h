#ifndef _MON86_COMMON
#define _MON86_COMMON

#include "mon86-config.h"


// Constants

#define TOKEN_LEN_MAX 5


// Scalar types

typedef unsigned char byte_t;
typedef unsigned short word_t;
typedef unsigned long addr_t;

typedef byte_t char_t;
typedef byte_t digit_t;


// Error codes

enum err_e
	{
	E_OK,
	E_LEN,
	E_VALUE,
	E_INDEX,
	E_END,
	E_TRACE,
	E_BREAK
	};

typedef enum err_e err_t;


// Overridables

err_t read_char  (char_t * c);
err_t write_char (char_t c);

err_t read_string  (char_t * s, word_t * len);
err_t write_string (char_t * s, word_t len);


// Library

char_t upcase (char_t c);
char_t lowcase (char_t c);

digit_t hex_to_digit (char_t c);
char_t digit_to_hex (digit_t d);

err_t hex_to_word (char_t * str, byte_t len, word_t * val);
void word_to_hex (word_t val, char_t * str, byte_t * len);

err_t read_token (char_t * str, byte_t * len);
err_t read_error ();
err_t read_word (word_t * val);

err_t write_word (word_t val);
err_t write_error (err_t err);
err_t write_command (byte_t c1, byte_t c2);


#endif // _MON86_COMMON
