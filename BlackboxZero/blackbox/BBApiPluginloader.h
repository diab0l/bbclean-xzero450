#ifndef __BBAPI_PLUGINLOADER_H__
#define __BBAPI_PLUGINLOADER_H__

#include <windef.h>

#ifndef DLL_EXPORT
    #define DLL_EXPORT __declspec(dllexport)
#endif

#ifndef __cplusplus
  typedef char bool;
  #define false 0
  #define true 1
  #define class struct
#else
extern "C" {
#endif

DLL_EXPORT bool Init();
DLL_EXPORT void Finalize();
DLL_EXPORT const char *GetName();
DLL_EXPORT const char *GetApi();
DLL_EXPORT int LoadPlugin(struct PluginList* plugin, HWND hSlit, char** errorMsg);
DLL_EXPORT int UnloadPlugin(struct PluginList* plugin, char** errorMsg);

#ifdef __cplusplus
}
#endif

#endif