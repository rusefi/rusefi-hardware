#define GCC_VERSION ((__GNUC__ * 100) + (__GNUC_MINOR__ * 10) + ( __GNUC_PATCHLEVEL__ ))

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#pragma message("GCC is " STR(__GNUC__)"."STR(__GNUC_MINOR__)"."STR(__GNUC_PATCHLEVEL__))

// Firmware builds require at least GCC 11.3.1
#if (GCC_VERSION < 1131)
// versions 9 and 10 have nasty 'more undefined references to `__cxa_pure_virtual' follow' issue
	#error "GCC compiler >= 11.3.1 required"
#endif
