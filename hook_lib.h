#define AB (
#define AE )
#define AS ,
#define NX
#define IDENT(...) __VA_ARGS__
#define ARG(a, b) a b
#define FST(a, b) a
#define SND(a, b) b
#define NONE(...)
#define STR(x) #x
#define FTNAME(x) ft_ ## x
#define FPNAME(x) fp_ ## x

// F -> ft_ ## name
#define FUNC_TYPENAME(F) \
	F(NONE, FTNAME, NONE, NX, NX, NX)
// F -> fp_ ## name
#define FUNC_PTRVAR(F) \
	F(NONE, FPNAME, NONE, NX, NX, NX)
// F -> "name"
#define FUNC_SNAME(F) F(NONE, STR, NONE, NX, NX, NX)
#define FUNC_T(F) F(IDENT, NONE, NONE, NX, NX, NX)
#define FUNC_ARGS(F) F(NONE, NONE, ARG, AB, AS, AE)
#define FUNC_FWARGS(F) F(NONE, NONE, SND, AB, AS, AE)

/* typedef T (*$FTNAME)(ARGS) */
#define FUNC_TYPEDEF(F) \
	typedef FUNC_T(F) (*FUNC_TYPENAME(F)) F(NONE, NONE, ARG, AB, AS, AE)

#define FUNC_PTRDEF(F) \
	static FUNC_TYPENAME(F) FUNC_PTRVAR(F)

#define FUNC_DLSYM(F) \
	(FUNC_TYPENAME(F))dlsym(RTLD_NEXT, FUNC_SNAME(F))

#define FUNC_DEF(F) \
	F(IDENT, IDENT, ARG, AB, AS, AE)
#define FUNC_FWD(F) \
	F(NONE, FPNAME, SND, AB, AS, AE)

#define HOOK(F) \
	FUNC_PTRVAR(F) = FUNC_DLSYM(F); \
	if (FUNC_PTRVAR(F) == NULL) { \
		fprintf(stderr, "Can't hook '%s': %s\n", FUNC_SNAME(F), dlerror()); \
		exit(EXIT_FAILURE); \
	} \
	fprintf(stderr, "Hooked '%s'\n", FUNC_SNAME(F))

#define HOOK_NF(F) ({ \
	int __hook_res = 1; \
	FUNC_PTRVAR(F) = FUNC_DLSYM(F); \
	if (FUNC_PTRVAR(F) == NULL) { \
		__hook_res = 0; \
	} \
	__hook_res; \
	})
