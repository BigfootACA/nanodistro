#ifndef BUILTIN_H
#define BUILTIN_H
#ifndef ASM
#ifndef __cplusplus
typedef struct builtin_file builtin_file;
#endif
struct builtin_file{
	const char*name;
	const char*path;
	unsigned long long size;
	char data[1];
};
#else
#define BUILTIN_FILE(_name,_path) \
	.section .data,"aw"; \
	.globl _name; \
	.type _name, %object; \
	.balign 8; \
	_name:; \
		.quad .L##_name##_xname; \
		.quad .L##_name##_xpath; \
		.quad (.L##_name##_end - .L##_name##_start); \
	.L##_name##_start:; \
		.incbin _path; \
	.L##_name##_end:; \
		.byte 0; \
		.balign 8; \
	.L##_name##_xname:; \
		.ascii #_name; \
		.byte 0; \
		.balign 8; \
	.L##_name##_xpath:; \
		.ascii _path; \
		.byte 0; \
		.balign 8; \
	.size _name, (. - _name);
#endif
#endif
