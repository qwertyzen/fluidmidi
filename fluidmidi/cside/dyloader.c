#include "dyloader.h"
#include "utils.h"

#ifdef _WIN32
    static char last_error_msg[512] = {0};

    void* cmusiclab_dlopen(const char* libname) {
        // Extract directory from libname
        char path_buffer[MAX_PATH];
        strncpy(path_buffer, libname, MAX_PATH - 1);
        path_buffer[MAX_PATH - 1] = '\0'; // Ensure null termination

        char* last_sep = strrchr(path_buffer, '\\');
        if (last_sep) {
            *last_sep = '\0'; // Truncate at last backslash
            SetDllDirectoryA(path_buffer);
        }

        // Now load the library
        HMODULE handle = LoadLibraryA(libname);
        if (!handle) {
            FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                           NULL, GetLastError(), 0, last_error_msg, sizeof(last_error_msg), NULL);
            LOGERR("Error loading library: %s\n", last_error_msg);
        }
        return (void*)handle;
    }
    void* cmusiclab_dlsym(void* library_handle, const char* symbol_name) {
        FARPROC symbol = GetProcAddress((HMODULE)library_handle, symbol_name);
        if (!symbol) {
            FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                           NULL, GetLastError(), 0, last_error_msg, sizeof(last_error_msg), NULL);
            LOGERR("Error finding symbol: %s\n", last_error_msg);
        }
        return (void*)symbol;
    }

    void cmusiclab_dlclose(void* library_handle) {
        if (library_handle) {
            FreeLibrary((HMODULE)library_handle);
        }
    }

    const char* cmusiclab_dlerror(void) {
        return last_error_msg;
    }

#else  // macOS / POSIX

    void* cmusiclab_dlopen(const char* libname) {
        void* handle = dlopen(libname, RTLD_LAZY | RTLD_GLOBAL);
        if (!handle) {
            LOGERR("Error loading library: %s\n", dlerror());
        }
        return handle;
    }

    void* cmusiclab_dlsym(void* library_handle, const char* symbol_name) {
        void* symbol = dlsym(library_handle, symbol_name);
        if (!symbol) {
            LOGERR("Error finding symbol: %s\n", dlerror());
        }
        return symbol;
    }

    void cmusiclab_dlclose(void* library_handle) {
        if (library_handle) {
            dlclose(library_handle);
        }
    }

    const char* cmusiclab_dlerror(void) {
        return dlerror();
    }

#endif
