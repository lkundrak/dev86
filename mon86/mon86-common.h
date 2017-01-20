#ifndef _MON86_COMMON
#define _MON86_COMMON

#include "mon86-config.h"


// Constants

#define TOKEN_LEN_MAX 4


// Scalar types

typedef unsigned char byte_t;
typedef unsigned short word_t;
typedef unsigned long addr_t;

typedef byte_t char_t;
typedef byte_t digit_t;


// Error codes

enum err_e
	{
	E_OK,      // 0
	E_END,     // 1
	E_LENGTH,  // 2
	E_VALUE,   // 3
	E_INDEX,   // 4
	E_TRACE,   // 5
	E_BREAK,   // 6
	E_EXIT     // 7
	};

typedef enum err_e err_t;


// Target specifics

err_t recv_char (char_t * c);
err_t send_char (char_t c);

err_t send_string (char_t * s, word_t len);


// Library

char_t upcase (char_t c);
char_t lowcase (char_t c);

digit_t hex_to_digit (char_t c);
char_t digit_to_hex (digit_t d);

err_t hex_to_word (char_t * str, byte_t len, word_t * val);
void word_to_hex (word_t val, char_t * str, byte_t * len);


// Tokens

err_t recv_token (char_t * str, byte_t * len);
err_t recv_word (word_t * val);
err_t recv_status ();

err_t send_word (word_t val);
err_t send_status (err_t err);


// Context

// A-F reserved for hex values

#define C_STATUS      'Z'
#define C_OFFSET      'O'
#define C_SEGMENT     'S'
#define C_LENGTH      'L'
#define C_MEM_READ    'R'
#define C_MEM_WRITE   'W'
#define C_REG_READ    'J'
#define C_REG_WRITE   'K'
#define C_PROC        'P'
#define C_TASK        'T'


struct context_s
	{
	word_t offset;   // +0h
	word_t segment;  // +2h
	word_t count;    // +4h
	word_t value;    // +6h

	byte_t length;
	char_t token [TOKEN_LEN_MAX];

	byte_t done;
	};

typedef struct context_s context_t;


err_t recv_context (context_t * context);
err_t send_context (context_t * context);


#endif // _MON86_COMMON
