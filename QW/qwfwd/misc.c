#include "master.h"

int		com_argc;
char	**com_argv;

/*
================
COM_CheckParm

Returns the position (1 to argc-1) in the program's argument list
where the given parameter apears, or 0 if not present
================
*/
int COM_CheckParm (char *parm)
{
	int		i;
	
	for (i=1 ; i<com_argc ; i++)
	{
		if (!com_argv[i])
			continue;		// NEXTSTEP sometimes clears appkit vars.
		if (!strcmp (parm,com_argv[i]))
			return i;
	}
		
	return 0;
}

char	*AdrToString (netadr_t a)
{
	static	char	s[64];
	
	sprintf (s, "%i.%i.%i.%i:%i",
		a.sin_addr.S_un.S_un_b.s_b1,
		a.sin_addr.S_un.S_un_b.s_b2,
		a.sin_addr.S_un.S_un_b.s_b3,
		a.sin_addr.S_un.S_un_b.s_b4,
		ntohs(a.sin_port));

	return s;
}

netadr_t	StringToAdr (char *s)
{
	netadr_t		a;
	int			b1, b2, b3, b4, p;
	
	b1 = b2 = b3 = b4 = p = 0;
	sscanf (s, "%i.%i.%i.%i:%i", &b1, &b2, &b3, &b4, &p);
	
	a.sin_addr.S_un.S_un_b.s_b1 = b1;
	a.sin_addr.S_un.S_un_b.s_b2 = b2;
	a.sin_addr.S_un.S_un_b.s_b3 = b3;
	a.sin_addr.S_un.S_un_b.s_b4 = b4;
	a.sin_port = ntohs(p);

	return a;
}

qboolean	CompareAdr (netadr_t a, netadr_t b)
{
	if (a.sin_addr.s_addr == b.sin_addr.s_addr 
	&& a.sin_port == b.sin_port)
		return true;
	return false;
}


void	mprintf (char *msg, ...)
{
	va_list	argptr;
	
	va_start (argptr, msg);
	vprintf (msg, argptr);
	vfprintf (logfile, msg, argptr);
	va_end (argptr);
		
}

int	msg_readcount;
char	*MSG_ReadString (void)
{
	char	*start;
	
	start = packet_data + msg_readcount;
	
	for ( ; msg_readcount < packet_length ; msg_readcount++)
		if (packet_data[msg_readcount] == '\n'
		|| packet_data[msg_readcount] == 0)
			break;
	
	packet_data[msg_readcount] = 0;
	msg_readcount++;
	
	return start;
}

/*
=====================================================================

  COMMAND LINES

=====================================================================
*/

#define	MAX_ARGS	32
#define	MAX_ARG_LENGTH	1024
int		cmd_argc;
char	cmd_argv[MAX_ARGS][MAX_ARG_LENGTH];

char		com_token[1024];
/*
==============
COM_Parse

Parse a token out of a string
==============
*/
char *COM_Parse (char *data)
{
	int		c;
	int		len;
	
	len = 0;
	com_token[0] = 0;
	
	if (!data)
		return NULL;
		
// skip whitespace
skipwhite:
	while ( (c = *data) <= ' ')
	{
		if (c == 0)
			return NULL;			// end of file;
		data++;
	}
	
// skip // comments
	if (c=='/' && data[1] == '/')
	{
		while (*data && *data != '\n')
			data++;
		goto skipwhite;
	}
	

// handle quoted strings specially
	if (c == '\"')
	{
		data++;
		while (1)
		{
			c = *data++;
			if (c=='\"' || !c)
			{
				com_token[len] = 0;
				return data;
			}
			com_token[len] = c;
			len++;
		}
	}

// parse a regular word
	do
	{
		com_token[len] = c;
		data++;
		len++;
		c = *data;
	} while (c>32);
	
	com_token[len] = 0;
	return data;
}

/*
============
Cmd_Argc
============
*/
int		Cmd_Argc (void)
{
	return cmd_argc;
}

/*
============
Cmd_Argv
============
*/
char	*Cmd_Argv (int arg)
{
	if ( (unsigned)arg >= cmd_argc )
		return "";
	return cmd_argv[arg];	
}

/*
============
Cmd_TokenizeString

Parses the given string into command line tokens.
============
*/
void Cmd_TokenizeString (char *text)
{	
	cmd_argc = 0;
	
	while (1)
	{
// skip whitespace up to a /n
		while (*text && *text <= ' ')
		{
			text++;
		}
		
		if (!*text)
			return;
	
		text = COM_Parse (text);
		if (!text)
			return;

		if (cmd_argc < MAX_ARGS)
		{
			strcpy (cmd_argv[cmd_argc], com_token);
			cmd_argc++;
		}
	}
	
}


/*
=====================================================================

  INFO STRINGS

=====================================================================
*/

/*
===============
Info_ValueForKey

Searches the string (userinfo or masterinfo) for the given
key and returns the associated value, or an empty string.
===============
*/
char *Info_ValueForKey (char *s, char *key)
{
	char	pkey[512];
	static	char value[512];
	char	*o;
	
	if (*s == '\\')
		s++;
	while (1)
	{
		o = pkey;
		while (*s != '\\')
		{
			if (!*s)
				return "";
			*o++ = *s++;
		}
		*o = 0;
		s++;

		o = value;
		while (*s != '\\' && *s)
		{
			if (!*s)
				return "";
			*o++ = *s++;
		}
		*o = 0;

		if (!strcmp (key, pkey) )
			return value;

		if (!*s)
			return "";
		s++;
	}
}

void Info_RemoveKey (char *s, char *key)
{
	char	*start;
	char	pkey[512];
	char	value[512];
	char	*o;

	if (strstr (key, "\\"))
	{
		printf ("Can't use a key with a \\\n");
		return;
	}

	while (1)
	{
		start = s;
		if (*s == '\\')
			s++;
		o = pkey;
		while (*s != '\\')
		{
			if (!*s)
				return;
			*o++ = *s++;
		}
		*o = 0;
		s++;

		o = value;
		while (*s != '\\' && *s)
		{
			if (!*s)
				return;
			*o++ = *s++;
		}
		*o = 0;

		if (!strcmp (key, pkey) )
		{
			strcpy (start, s);	// remove this part
			return;
		}

		if (!*s)
			return;
	}

}

void Info_RemovePrefixedKeys (char *start, char prefix)
{
	char	*s;
	char	pkey[512];
	char	value[512];
	char	*o;

	s = start;
	while (1)
	{
		if (*s == '\\')
			s++;
		o = pkey;
		while (*s != '\\')
		{
			if (!*s)
				return;
			*o++ = *s++;
		}
		*o = 0;
		s++;

		o = value;
		while (*s != '\\' && *s)
		{
			if (!*s)
				return;
			*o++ = *s++;
		}
		*o = 0;

		if (pkey[0] == prefix)
		{
			Info_RemoveKey (start, pkey);
			s = start;
		}

		if (!*s)
			return;
	}

}

void Info_SetValueForKey (char *s, char *key, char *value)
{
	char	new[512];

	if (strstr (key, "\\") || strstr (value, "\\") )
	{
		printf ("Can't use keys or values with a \\\n");
		return;
	}
	if (strstr (key, "\"") || strstr (value, "\"") )
	{
		printf ("Can't use keys or values with a \\\n");
		return;
	}

	Info_RemoveKey (s, key);
	if (!value || !strlen(value))
		return;

	sprintf (new, "\\%s\\%s", key, value);

	if (strlen(new) + strlen(s) > MAX_INFO_STRING)
	{
		printf ("Info string length exceeded\n");
		return;
	}
	strcat (s, new);
}

void Info_Print (char *s)
{
	char	key[512];
	char	value[512];
	char	*o;
	int		l;

	if (*s == '\\')
		s++;
	while (*s)
	{
		o = key;
		while (*s && *s != '\\')
			*o++ = *s++;

		l = o - key;
		if (l < 20)
		{
			memset (o, ' ', 20-l);
			key[20] = 0;
		}
		else
			*o = 0;
		printf ("%s", key);

		if (!*s)
		{
			printf ("MISSING VALUE\n");
			return;
		}

		o = value;
		s++;
		while (*s && *s != '\\')
			*o++ = *s++;
		*o = 0;

		if (*s)
			s++;
		printf ("%s\n", value);
	}
}
