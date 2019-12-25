#ifndef __WIN32__
extern "C" {

#if !(__x86_64__)
// 32bit
// exp2 & exp2f undefined (?) in old glibc:i686
//__asm__(".symver __real_exp2,exp2@GLIBC_2.0");
__asm__(".symver __real_exp,exp@GLIBC_2.0");
__asm__(".symver __real_pow,pow@GLIBC_2.0");
__asm__(".symver __real_log,log@GLIBC_2.0");
//__asm__(".symver __real_exp2f,exp2f@GLIBC_2.0");
__asm__(".symver __real_expf,expf@GLIBC_2.0");
__asm__(".symver __real_powf,powf@GLIBC_2.0");
__asm__(".symver __real_logf,logf@GLIBC_2.0");
__asm__(".symver __real_fcntl,fcntl@GLIBC_2.0");
#else
__asm__(".symver __real_exp2,exp2@GLIBC_2.2.5");
__asm__(".symver __real_exp,exp@GLIBC_2.2.5");
__asm__(".symver __real_pow,pow@GLIBC_2.2.5");
__asm__(".symver __real_log,log@GLIBC_2.2.5");
__asm__(".symver __real_exp2f,exp2f@GLIBC_2.2.5");
__asm__(".symver __real_expf,expf@GLIBC_2.2.5");
__asm__(".symver __real_powf,powf@GLIBC_2.2.5");
__asm__(".symver __real_logf,logf@GLIBC_2.2.5");
__asm__(".symver __real_fcntl,fcntl@GLIBC_2.2.5");
#endif

double __real_exp2(double);
double __real_exp(double, double);
double __real_pow(double, double);
double __real_log(double, double);
float __real_exp2f(float);
float __real_expf(float, float);
float __real_powf(float, float);
float __real_logf(float, float);
int __real_fcntl(int fd, int cmd, ...);

int __wrap_fcntl(int fd, int cmd, ...);
int __wrap_fcntl64(int fd, int cmd, ...);


double __wrap_exp2(double a) {
#if !(__x86_64__)
	return __real_pow(2, a);
#endif
	return __real_exp2(a);
}
double __wrap_exp(double a, double b) {
	return __real_exp(a, b);
}
double __wrap_pow(double a, double b) {
	return __real_pow(a, b);
}
double __wrap_log(double a, double b) {
	return __real_log(a, b);
}

float __wrap_exp2f(float a) {
#if !(__x86_64__)
	return __real_powf(2, a);
#endif
	return __real_exp2f(a);
}
float __wrap_expf(float a, float b) {
	return __real_expf(a, b);
}
float __wrap_powf(float a, float b) {
	return __real_powf(a, b);
}
float __wrap_logf(float a, float b) {
	return __real_logf(a, b);
}

} //extern "C"


#include <fcntl.h>
#include <stdarg.h> //va_list
#include <stdio.h>  //fprintf, stderr

#include "utils/scope_exit.h"

int __wrap_fcntl(int fd, int cmd, ...) {
	va_list va;
	va_start(va, cmd);
	ScopeExit se([&]() { va_end(va); });

	switch (cmd) {
	    case F_DUPFD: goto takes_int;
	    case F_DUPFD_CLOEXEC: goto takes_int;

	    case F_GETFD: goto takes_void;
	    case F_SETFD: goto takes_int;

	    case F_GETFL: goto takes_void;
	    case F_SETFL: goto takes_int;

	    case F_SETLK: goto takes_ptr;
	    case F_SETLKW: goto takes_ptr;
	    case F_GETLK: goto takes_ptr;

	    case F_OFD_SETLK: goto takes_ptr;
	    case F_OFD_SETLKW: goto takes_ptr;
	    case F_OFD_GETLK: goto takes_ptr;

	    case F_GETOWN: goto takes_void;
	    case F_SETOWN: goto takes_int;
	    case F_GETOWN_EX: goto takes_ptr;
	    case F_SETOWN_EX: goto takes_ptr;
	    case F_GETSIG: goto takes_void;
	    case F_SETSIG: goto takes_int;

	    case F_SETLEASE: goto takes_int;
	    case F_GETLEASE: goto takes_void;

	    case F_NOTIFY: goto takes_int;

	    case F_SETPIPE_SZ: goto takes_int;
	    case F_GETPIPE_SZ: goto takes_void;

	    case F_ADD_SEALS: goto takes_int;
	    case F_GET_SEALS: goto takes_void;

	    case F_GET_RW_HINT: goto takes_ptr;
	    case F_SET_RW_HINT: goto takes_ptr;
	    case F_GET_FILE_RW_HINT: goto takes_ptr;
	    case F_SET_FILE_RW_HINT: goto takes_ptr;

	    default:
		    fprintf(stderr, "fcntl workaround got unknown F_XXX constant (value: %i)\n", cmd);
	}

takes_ptr:
	return __real_fcntl(fd, cmd, va_arg(va, void*));

takes_int:
	return __real_fcntl(fd, cmd, va_arg(va, int));

takes_void:
	return __real_fcntl(fd, cmd);
}

int __wrap_fcntl64(int fd, int cmd, ...) {
	va_list va;
	va_start(va, cmd);
	ScopeExit se([&]() { va_end(va); });

	switch (cmd) {
	    case F_DUPFD: goto takes_int;
	    case F_DUPFD_CLOEXEC: goto takes_int;

	    case F_GETFD: goto takes_void;
	    case F_SETFD: goto takes_int;

	    case F_GETFL: goto takes_void;
	    case F_SETFL: goto takes_int;

	    case F_SETLK: goto takes_ptr;
	    case F_SETLKW: goto takes_ptr;
	    case F_GETLK: goto takes_ptr;

	    case F_OFD_SETLK: goto takes_ptr;
	    case F_OFD_SETLKW: goto takes_ptr;
	    case F_OFD_GETLK: goto takes_ptr;

	    case F_GETOWN: goto takes_void;
	    case F_SETOWN: goto takes_int;
	    case F_GETOWN_EX: goto takes_ptr;
	    case F_SETOWN_EX: goto takes_ptr;
	    case F_GETSIG: goto takes_void;
	    case F_SETSIG: goto takes_int;

	    case F_SETLEASE: goto takes_int;
	    case F_GETLEASE: goto takes_void;

	    case F_NOTIFY: goto takes_int;

	    case F_SETPIPE_SZ: goto takes_int;
	    case F_GETPIPE_SZ: goto takes_void;

	    case F_ADD_SEALS: goto takes_int;
	    case F_GET_SEALS: goto takes_void;

	    case F_GET_RW_HINT: goto takes_ptr;
	    case F_SET_RW_HINT: goto takes_ptr;
	    case F_GET_FILE_RW_HINT: goto takes_ptr;
	    case F_SET_FILE_RW_HINT: goto takes_ptr;

	    default:
		    fprintf(stderr, "fcntl64 workaround got unknown F_XXX constant (value: %i)\n", cmd);
	}

takes_ptr:
	return __real_fcntl(fd, cmd, va_arg(va, void*));

takes_int:
	return __real_fcntl(fd, cmd, va_arg(va, int));

takes_void:
	return __real_fcntl(fd, cmd);
}

#endif //ifndef __WIN32__
