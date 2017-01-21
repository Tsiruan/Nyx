#include <stdarg.h>
#include <string.h>


int ut_strmatch(const char *str, int count, ...) {
	va_list args_ptr;
	va_start(args_ptr, count);

	int i;
	for (i = 0; i < count; i++) {
		if (strcmp(str, va_arg(args_ptr, const char *)) == 0)
			return 1;
	}
	return 0;
}