
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

/* TODO: use n */
int snprintf(char * str, size_t n, const char * fmt, ...)
{
	int i, len;
	int fill;
	int width;
	int size;

	va_list list;
	va_start(list, fmt);
	for(i = len = 0; fmt[i] && len < n - 1; i++)
	{
		if(fmt[i] == '%')
		{
			fill = ' ';
			width = 0;
			size = 0;
		retry:
			switch(fmt[++i])
			{
			case '%':
				str[len++] = '%';
				break;
			case '0':
				if(width == 0)
				{
					fill = '0';
					goto retry;
				}
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				width = 10 * width + fmt[i] - '0';
				goto retry;
			case 's':
				{
					char * arg = va_arg(list, char *);
					strcpy(str + len, arg);
					len += strlen(arg);
				}
				break;
			case 'd':
				{
					static char tmp[6];
					int arg = va_arg(list, int);
					int arglen;
					_itoa(arg, tmp, 10);
					arglen = strlen(tmp);
					while(arglen < width)
					{
						str[len++] = fill;
						width --;
					}
					strcpy(str + len, tmp);
					len += arglen;
				}
				break;
			}
		}
		else
		{
			str[len++] = fmt[i];
		}
	}
	str[len] = '\0';
	va_end(list);
	return len;
}

