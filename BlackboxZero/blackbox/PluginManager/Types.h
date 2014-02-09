#ifndef __PluginManager_Types_H__
#define __PluginManager_Types_H__

#define SetFlag(x, f, e) (x = (e) ? ((x) | (f)) : ((x) & ~(f)))
#define ClearFlag(x, f) (x = (x) & ~(f))
#define CheckFlag(x, f) !!((x) & (f))
#define ToggleFlag(x, f) (x ^= (f))

typedef enum {
    Plugin_IsEnabled = 1 << 0,  // plugin should be loaded
    Plugin_UseSlit = 1 << 1,    // plugin should be loaded into slit
    Plugin_InSlit  = 1 << 2,    // plugin is in the slit
} PluginFlags;

enum plugin_errors {
    error_plugin_is_built_in       = 1,
    error_plugin_dll_not_found     ,
    error_plugin_dll_needs_module  ,
    error_plugin_does_not_load     ,
    error_plugin_missing_entry     ,
    error_plugin_fail_to_load      ,
    error_plugin_crash_on_load     ,
    error_plugin_crash_on_unload   ,
    error_plugin_message
};

struct PluginList
{
    struct PluginList *next;

    char *name;     // display name as in the menu, NULL for comments
    char *path;     // as in plugins.rc, entire line for comments

    UINT flags;

    bool inslit;    

    HMODULE hmodule; // as returned by LoadLibrary
    int n_instance; // if the same plugin name is used more than once

    int (*beginPlugin)(HINSTANCE);
    int (*beginPluginEx)(HINSTANCE, HWND);
    int (*beginSlitPlugin)(HINSTANCE, HWND);
    int (*endPlugin)(HINSTANCE);
    const char* (*pluginInfo)(int);
};

// same order as function ptrs above
static const char* const pluginFunctionNames[] = {
    "beginPlugin"       ,
    "beginPluginEx"     ,
    "beginSlitPlugin"   ,
    "endPlugin"         ,
    "pluginInfo"        ,
    NULL
};

struct PluginPtr {
    struct PluginPtr* next;

    struct PluginList* entry;
};

struct PluginLoaderList {
	struct PluginList* parent;
    struct PluginLoaderList* next;
    
	HMODULE module;
	const char* name;
	const char* api;
		
	struct PluginPtr* plugins;
		
	bool (*Init)();
	void (*Finalize)();
		
	const char* (*GetName)();
    const char* (*GetApi)();

	int (*LoadPlugin)(struct PluginList* plugin, HWND hSlit, char** errorMsg);
	int (*UnloadPlugin)(struct PluginList* plugin, char** errorMsg);
};

static const char* const pluginLoaderFunctionNames[] = {
    "Init",
    "Finalize",
    "GetName",
    "GetApi",
    "LoadPlugin",
    "UnloadPlugin",
    NULL
};

#define LoadFunction(pll, fn) (*(FARPROC*)&pll->fn = (pll->fn) ? ((FARPROC)pll->fn) : (GetProcAddress(pll->module, #fn)))
#define FailWithMsg(msgVar, msg) { if(msgVar) \
                                     *msgVar = msg; \
                                   return error_plugin_message; \
                                 }
#endif