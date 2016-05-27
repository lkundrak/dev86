// PT1 common code


#define TOKEN_LEN_MAX 4

typedef unsigned char byte_t;
typedef unsigned short word_t;


// TODO: v2: group common code with PT-TARGET

static byte_t upcase (byte_t c)
	{
	if (c >= 'a' && c <= 'z')
		{
		c = c + 'A' - 'a';
		}

	return c;
	}


static byte_t hex_to_digit (byte_t c)
	{
	byte_t d = 0xFF;

	if (c >= '0' && c <= '9') d = c - '0';
	if (c >= 'A' && c <= 'F') d = c - 'A' + 10;

	return d;
	}


static byte_t digit_to_hex (byte_t d)
	{
	byte_t c;

	if (d > 9)
		{
		c = 'A' + d - 10;
		}
	else
		{
		c = '0' + d;
		}

	return c;
	}


static int hex_to_word (byte_t * str, byte_t len, word_t * val)
	{
	int err = 0;

	*val = 0;
	byte_t pos;

	for (pos = 0; pos < len; pos++)
		{
		byte_t d = hex_to_digit (*(str++));
		if (d > 0x0F)
			{
			err = -1;
			break;
			}

		*val = (*val << 4) + (word_t) d;
		}

	return err;
	}


static void word_to_hex (word_t val, char * str, byte_t * len)
	{
	byte_t start = 0;
	word_t mask = 0xF000;
	byte_t shift = 12;

	byte_t pos;

	for (pos = 0; pos < 4; pos++)
		{
		word_t d = (val & mask) >> shift;
		if (d || start || pos == 3)
			{
			start = 1;

			*(str++) = digit_to_hex ((byte_t) d);
			(*len)++;
			}

		shift -= 4;
		mask >>= 4;
		}
	}


static int _fi = -1;


static int recv_char (byte_t * c)
	{
	int err = -1;
	int n = read (_fi, c, 1);
	if (n == 1) err = 0;
	return err;
	}


static int recv_token (byte_t * str, byte_t * len)
	{
	int err = -1;

	byte_t stat = 0;
	byte_t c;

	*len = 0;

	while (1)
		{
		err = recv_char (&c);

		// End of stream

		if (err || c == 0 || c == 0xFF)
			{
			*len = 0;
			break;
			}

		// Ignore heading spaces
		// Other trailers are separators

		if (c <= ' ' || c >= 127)
			{
			if (stat == 0) continue;
			err = 0;
			break;
			}

		// Token begins or continues

		stat = 1;

		if (++(*len) > TOKEN_LEN_MAX) break;

		*(str++) = upcase (c);
		}

	return err;
	}


static int recv_status ()
	{
	int err = -1;

	while (1)
		{
		byte_t len = 0;
		byte_t token [TOKEN_LEN_MAX];

		err = recv_token (token, &len);
		if (err) break;

		if (len != 2 || token [0] != 'S' || token [1] != '0') break;

		err = 0;
		break;
		}

	return err;
	}


static int recv_value (word_t * val)
	{
	int err = -1;

	while (1)
		{
		byte_t len = 0;
		byte_t token [TOKEN_LEN_MAX];

		err = recv_token (token, &len);
		if (err) break;

		err = hex_to_word (token, len, val);
		break;
		}

	return err;
	}


static int _fo = -1;


static int send_value (word_t val)
	{
	int err = -1;

	byte_t len = 0;
	byte_t str [5];
	word_to_hex (val, str, &len);

	str [len++] = 10;  // LF as separator

	int n = write (_fo, str, len);
	if (n == len) err = 0;

	return err;
	}


static int send_command (byte_t c1, byte_t c2)
	{
	int err = -1;

	byte_t str [3];
	str [0] = c1;
	str [1] = c2;
	str [2] = 10;  // LF as separator

	int n = write (_fo, str, 3);
	if (n == 3) err = 0;

	return err;
	}
