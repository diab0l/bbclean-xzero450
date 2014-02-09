#include "../BB.h"
#include "PluginManager.h"
#include "PluginLoaderNative.h"
#include "Types.h"

#define DLL_EXPORT
#include "../BBApiPluginloader.h"

char name[255];

struct PluginLoaderList nativeLoader = {
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,

    Init,
	Finalize,
	
    GetName,
	GetApi,

	LoadPlugin,
	UnloadPlugin
};

bool Init() {
    sprintf(name, "Core Pluginloader %s", GetBBVersion());
    return true;
}

void Finalize() {}

const char *GetName() {
    return name;
}

const char *GetApi() {
    return GetBBVersion();
}

int LoadPlugin(struct PluginList* q, HWND hSlit, char** errorMsg) {
    HINSTANCE hModule = NULL;
    int error = 0;
    bool useslit;
    int r, i;
    char plugin_path[MAX_PATH];

    for (;;)
    {
        //---------------------------------------
        // check for compatibility

#ifndef BBTINY
        if (0 == _stricmp(q->name, "BBDDE"))
        {
            error = error_plugin_is_built_in;
            break;
        }
#endif

        //---------------------------------------
        // load the dll

        if (0 == FindRCFile(plugin_path, q->path, NULL)) {
            error = error_plugin_dll_not_found;
            break;
        }

        r = SetErrorMode(0); // enable 'missing xxx.dll' system message
        hModule = LoadLibrary(plugin_path);
        SetErrorMode(r);

        if (NULL == hModule)
        {
            r = GetLastError();
            // char buff[200]; win_error(buff, sizeof buff);
            // dbg_printf("LoadLibrary::GetLastError %d: %s", r, buff);
            if (ERROR_MOD_NOT_FOUND == r)
                error = error_plugin_dll_needs_module;
            else
                error = error_plugin_does_not_load;
            break;
        }

        //---------------------------------------
        // grab interface functions

        for (i = 0; pluginFunctionNames[i]; ++i)
            ((FARPROC*)&q->beginPlugin)[i] =
            GetProcAddress(hModule, pluginFunctionNames[i]);

        //---------------------------------------
        // check interface presence

        if (NULL == q->endPlugin) {
            error = error_plugin_missing_entry;
            break;
        }

        if (NULL == q->beginPluginEx && NULL == q->beginSlitPlugin)
            ClearFlag(q->flags, Plugin_UseSlit);
            //q->useslit = false;

        useslit = hSlit && CheckFlag(q->flags, Plugin_UseSlit); // q->useslit;

        if (false == useslit && NULL == q->beginPluginEx && NULL == q->beginPlugin) {
            error = error_plugin_missing_entry;
            break;
        }

        //---------------------------------------
        // inititalize plugin

        TRY
        {
            if (useslit) {
                if (q->beginPluginEx)
                    r = q->beginPluginEx(hModule, hSlit);
                else
                    r = q->beginSlitPlugin(hModule, hSlit);
            } else {
                if (q->beginPlugin)
                    r = q->beginPlugin(hModule);
                else
                    r = q->beginPluginEx(hModule, NULL);
            }

            if (BEGINPLUGIN_OK == r) {
                q->hmodule = hModule;
                q->inslit = useslit;
            } else if (BEGINPLUGIN_FAILED_QUIET != r) {
                error = error_plugin_fail_to_load;
            }

        }
        EXCEPT(EXCEPTION_EXECUTE_HANDLER)
        {
            error = error_plugin_crash_on_load;
        }
        break;
    }

    // clean up after error
    if (NULL == q->hmodule && hModule)
        FreeLibrary(hModule);

    return error;
}

int UnloadPlugin(struct PluginList* q, char** errorMsg) {
    int error = 0;
    if (q->hmodule)
    {
        TRY
        {
            q->endPlugin(q->hmodule);
        }
        EXCEPT(EXCEPTION_EXECUTE_HANDLER)
        {
            error = error_plugin_crash_on_unload;
        }

        FreeLibrary(q->hmodule);
        q->hmodule = NULL;
    }

    return error;
}