#ifndef __DYLOADER_H__
#define __DYLOADER_H__

#ifdef _WIN32
    #include <windows.h>
#else
    #include <dlfcn.h>
#endif

void*       cmusiclab_dlopen(const char* libname);
void*       cmusiclab_dlsym(void* library_handle, const char* symbol_name);
void        cmusiclab_dlclose(void* library_handle);
const char* cmusiclab_dlerror(void);

#endif /* __DYLOADER_H__ */
