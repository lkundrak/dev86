
#include "mon86-common.h"


//-----------------------------------------------------------------------------
// Conversion functions
//-----------------------------------------------------------------------------

char_t upcase (char_t c)
	{
	return (c >= 'a' && c <= 'z') ? c + 'A' - 'a' : c;
	}


char_t lowcase (char_t c)
	{
	return (c >= 'A' && c <= 'Z') ? c + 'a' - 'A' : c;
	}


digit_t hex_to_digit (char_t c)
	{
	digit_t d;

	while (1)
		{
		if (c >= '0' && c <= '9')
			{
			d = c - '0';
			break;
			}

		if (c >= 'A' && c <= 'F')
			{
			d = c - 'A' + 10;
			break;
			}

		d = 0xFF;
		break;
		}

	return d;
	}


char_t digit_to_hex (digit_t d)
	{
	return (d > 9) ? 'A' + d - 10 : '0' + d;
	}


err_t hex_to_word (char_t * str, byte_t len, word_t * val)
	{
	err_t err;
	byte_t pos;

	*val = 0;

	while (1)
		{
		if (!len || len > 4)
			{
			err = E_LENGTH;
			break;
			}

		pos = 0;

		while (1)
			{
			digit_t d = hex_to_digit (*str);
			if (d > 0x0F)
				{
				err = E_VALUE;
				break;
				}

			*val += (word_t) d;

			if (++pos >= len)
				{
				err = E_OK;
				break;
				}

			*val = (*val << 4);
			str++;
			}

		break;
		}

	return err;
	}


void word_to_hex (word_t val, char_t * str, byte_t * len)
	{
	char_t buf [4];
	byte_t pos;

	pos = 3;

	while (1)
		{
		buf [pos] = digit_to_hex ((digit_t) val & 0x000F);

		val >>= 4;
		if (!val) break;

		pos--;
		}

	*len = 4 - pos;

	while (pos < 4)
		{
		*(str++) = buf [pos++];
		}
	}


//-----------------------------------------------------------------------------
// Read functions
//-----------------------------------------------------------------------------

err_t recv_token (char_t * str, byte_t * len)
	{
	err_t err;

	byte_t stat;
	char_t c;

	*len = 0;
	stat = 0;

	while (1)
		{
		err = recv_char (&c);
		if (err) break;

		// Ignore heading spaces
		// Other trailers are separators

		if (c <= ' ' || c >= 127)
			{
			if (stat == 0) continue;

			err = E_OK;
			break;
			}

		// Token begins or continues

		stat = 1;

		if (++(*len) > TOKEN_LEN_MAX) break;

		*(str++) = upcase (c);
		}

	return err;
	}


err_t recv_word (word_t * val)
	{
	err_t err;

	byte_t token [TOKEN_LEN_MAX];
	byte_t len;

	len = 0;

	while (1)
		{
		err = recv_token (token, &len);
		if (err) break;

		err = hex_to_word (token, len, val);
		break;
		}

	return err;
	}


err_t recv_status ()
	{
	err_t err;

	while (1)
		{
		byte_t len = 0;
		byte_t token [TOKEN_LEN_MAX];

		err = recv_token (token, &len);
		if (err) break;

		if (len != 2)
			{
			err = E_LENGTH;
			break;
			}

		if (token [0] != 'Z')
			{
			err = E_VALUE;
			break;
			}

		err = (err_t) (token [1] - '0');
		break;
		}

	return err;
	}


err_t recv_context (context_t * context)
	{
	err_t err;

	char_t c;

	while (1)
		{
		context->done = 0;

		err = recv_token (context->token, &context->length);
		if (err == E_OK && context->length)
			{
			c = context->token [0];
			if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F'))
				{
				err = hex_to_word (context->token, context->length, &context->value);
				context->done = 1;
				break;
				}

			switch (c)
				{
				// Set offset

				case C_OFFSET:
					if (context->length != 1)
						{
						err = E_LENGTH;
						break;
						}

					context->offset = context->value;
					context->done = 1;
					break;

				// Set segment

				case C_SEGMENT:
					if (context->length != 1)
						{
						err = E_LENGTH;
						break;
						}

					context->segment = context->value;
					context->done = 1;
					break;

				// Set length

				case C_LENGTH:
					if (context->length != 1)
						{
						err = E_LENGTH;
						break;
						}

					context->count = context->value;
					context->done = 1;
					break;

				}
			}

		break;
		}

	return err;
	}


//-----------------------------------------------------------------------------
// Write functions
//-----------------------------------------------------------------------------

err_t send_word (word_t val)
	{
	byte_t str [5];
	byte_t len;

	word_to_hex (val, str, &len);

	str [len] = ' ';

	return send_string (str, len + 1);
	}


err_t send_status (err_t err)
	{
	char_t str [4];

	str [0] = 'Z';
	str [1] = digit_to_hex ((digit_t) err & 0xF);
	str [2] = 13;  // carriage return
	str [3] = 10;  // line feed

	return send_string (str, 4);
	}


err_t send_context (context_t * context)
	{
	err_t err;

	while (1)
		{
		err = send_word (context->offset);
		if (err) break;

		err = recv_status ();
		if (err) break;

		err = send_string ("O ", 2);
		if (err) break;

		err = recv_status ();
		if (err) break;

		err = send_word (context->segment);
		if (err) break;

		err = recv_status ();
		if (err) break;

		err = send_string ("S ", 2);
		if (err) break;

		err = recv_status ();
		if (err) break;

		err = E_OK;
		break;
		}

	return err;
	}


//-----------------------------------------------------------------------------
