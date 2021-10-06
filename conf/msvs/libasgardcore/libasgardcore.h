// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the PROJECTASGARD_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// PROJECTASGARD_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef PROJECTASGARD_EXPORTS
#define PROJECTASGARD_API __declspec(dllexport)
#else
#define PROJECTASGARD_API __declspec(dllimport)
#endif

/*

extern PROJECTASGARD_API int dl_open(char *filename, void **handle);

extern PROJECTASGARD_API int dl_close(void *handle);

extern PROJECTASGARD_API int dl_lookup(void *handle, char *function_name, void **function);

*/

/*
// This class is exported from the Project Asgard.dll
class PROJECTASGARD_API CProjectAsgard {
public:
	CProjectAsgard(void);
	// TODO: add your methods here.
};

extern PROJECTASGARD_API int nProjectAsgard;

PROJECTASGARD_API int fnProjectAsgard(void);
*/

#include "core/core.h"