/* ==========================================================================

This file is part of the bbLean source code
Copyright © 2001-2003 The Blackbox for Windows Development Team
Copyright © 2004-2009 grischka

http://bb4win.sourceforge.net/bblean
http://developer.berlios.de/projects/bblean

bbLean is free software, released under the GNU General Public License
(GPL version 2). For details see:

http://www.fsf.org/licenses/gpl.html

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
for more details.

========================================================================== */

#include <regex>
#include <string.h>

#include "../BB.h"
#include "bbrc.h"
#include "PluginManager.h"
#include "PluginLoaderNative.h"
#include "Types.h"

// Private variables
static struct PluginLoaderList *pluginLoaders = &nativeLoader;
static struct PluginList *bbplugins; // list of plugins
static struct PluginList *slitPlugin;

static HWND hSlit;  // BBSlit window
static FILETIME rc_filetime; // plugins.rc filetime

// Forward decls
static void applyPluginStates();
static int loadPlugin(struct PluginList *q, HWND hSlit, char **errorMsg);
static int unloadPlugin(struct PluginList *q, char **errorMsg);
static void free_plugin(struct PluginList **q);
static struct PluginList *parseConfigLine(const char *rcline);
static bool write_plugins(void);
static void showPluginErrorMessage(struct PluginList *q, int error, const char *msg);

static bool isPluginLoader(char *path);
static int loadPluginLoader(struct PluginList *q, char **errorMsg);
static int initPluginLoader(struct PluginLoaderList *pll, char **errorMsg);
static int unloadPluginLoader(struct PluginLoaderList *pll, char **errorMsg);

//===========================================================================

void PluginManager_Init(void)
{
    const char *path;
    FILE *fp;
    char szBuffer[MAX_PATH];

    initPluginLoader(&nativeLoader, NULL);
    bbplugins = c_new(struct PluginList);
    bbplugins->flags = Plugin_IsEnabled;
    bbplugins->name = (char*)c_alloc(sizeof(char) * strlen(nativeLoader.name) + 1);
    bbplugins->path = (char*)c_alloc(sizeof(char));
    strcpy(bbplugins->name, nativeLoader.name);
    nativeLoader.parent = bbplugins;

    path = plugrcPath(NULL);
    get_filetime(path, &rc_filetime);

    // read plugins.rc
    fp = fopen(path,"rb");
    if (fp) {
        while (read_next_line(fp, szBuffer, sizeof szBuffer))
            append_node(&bbplugins, parseConfigLine(szBuffer));

        fclose(fp);
    }

    applyPluginStates();
}

void PluginManager_Exit(void)
{
    struct PluginList *q;

    reverse_list(&bbplugins);
    dolist (q, bbplugins)
        ClearFlag(q->flags, Plugin_IsEnabled);
    applyPluginStates();
    while (bbplugins)
        free_plugin(&bbplugins);
}

int PluginManager_RCChanged(void)
{
    return 0 != diff_filetime(plugrcPath(NULL), &rc_filetime);
}

//===========================================================================
// (API:) EnumPlugins

void EnumPlugins (PLUGINENUMPROC lpEnumFunc, LPARAM lParam)
{
    struct PluginList *q;

    dolist(q, bbplugins)
        if (FALSE == lpEnumFunc(q, lParam))
            return;
}

/// <summary>
///     Parses a line from the config file.
/// </summary>
/// <remarks>
///     Lines containing an invalid path or starting with # or [ are considered comments and loaded as is.
///     Lines containing a path ending in ".dll" are considered plugins.
///     Lines starting with ! are loaded as disabled plugins.
///     Lines starting with & are loaded as slit plugins.
///     Lines starting with ! & or & ! are loaded as disabled slit plugins.
///     Trailing comments are not supported.
/// </remarks>
static struct PluginList *parseConfigLine(const char *rcline) {
    struct PluginList *q = c_new(struct PluginList);

    std::cmatch pathAndName;
    if(!std::regex_match(rcline, pathAndName, std::regex("(?![\\[#])[ &!]*((?:.*\\\\)*(.*)\\.dll)"))) {
        q->path = new_str(rcline);
        return q;
    }

    q->path = new_str(pathAndName[1].str().c_str());

    SetFlag(q->flags, Plugin_IsEnabled, !std::regex_match(rcline, std::regex("[ &]*(!)[ &]*.*")));
    SetFlag(q->flags, Plugin_UseSlit, std::regex_match(rcline, std::regex("[ !]*(&)[ !]*.*")));

    char name[MAX_PATH]; 
    name[MAX_PATH-1] = 0;

    strncpy(name, pathAndName[2].str().c_str(), MAX_PATH-1);

    struct PluginList *q2;
    int n = 0, l = strlen(name);

    dolist (q2, bbplugins)
        if (q2->name && 0 == _memicmp(q2->name, name, l) && (0 == q2->name[l] || '.' == q2->name[l]))
            n++;

    if (n > 0)
        sprintf(name + l, ".%d", 1+n);

    q->n_instance = n;
    q->name = new_str(name);

    if(std::regex_match(name, std::regex("bbslit(\\..*)?", std::tr1::regex_constants::icase)))
        slitPlugin = q;

    return q;
}

//===========================================================================
// run through plugin list and load/unload changed plugins

static void applyPluginState(struct PluginList *q)
{
    int error = 0;
    if (q->hmodule) {
        if (CheckFlag(q->flags, Plugin_IsEnabled) && 
            (CheckFlag(q->flags, Plugin_UseSlit) && hSlit) == q->inslit)
            return;
        error = unloadPlugin(q, NULL);
    }

    if (CheckFlag(q->flags, Plugin_IsEnabled)) {
        if (0 == error)
            error = loadPlugin(q, hSlit, NULL);
        if (NULL == q->hmodule)
            ClearFlag(q->flags, Plugin_IsEnabled);
        if (error)
            write_plugins();
    }

    showPluginErrorMessage(q, error, "(Unknown error)");
}

static void applyPluginStates()
{
    struct PluginList *q;

    hSlit = NULL;

    // load slit first
    if(slitPlugin && CheckFlag(slitPlugin->flags, Plugin_IsEnabled)) {
        applyPluginState(slitPlugin);
        hSlit = FindWindow("BBSlit", NULL);
    }

    // load/unload other plugins
    dolist(q, bbplugins)
        if (q->name && q != slitPlugin)
            applyPluginState(q);

    // unload slit last
    if(slitPlugin && !CheckFlag(slitPlugin->flags, Plugin_IsEnabled))
        applyPluginState(slitPlugin);
}

//===========================================================================
// plugin error message

static void showPluginErrorMessage(struct PluginList *q, int error, const char* msg)
{
    switch (error)
    {
    case 0:
        return;
    case error_plugin_is_built_in      :
        msg = NLS2("$Error_Plugin_IsBuiltIn$",
            "Dont load this plugin with "BBAPPNAME". It is built-in."
            ); break;
    case error_plugin_dll_not_found    :
        msg = NLS2("$Error_Plugin_NotFound$",
            "The plugin was not found."
            ); break;
    case error_plugin_dll_needs_module :
        msg = NLS2("$Error_Plugin_MissingModule$",
            "The plugin cannot be loaded. Possible reason:"
            "\n- The plugin requires another dll that is not there."
            ); break;
    case error_plugin_does_not_load    :
        msg = NLS2("$Error_Plugin_DoesNotLoad$",
            "The plugin cannot be loaded. Possible reasons:"
            "\n- The plugin requires another dll that is not there."
            "\n- The plugin is incompatible with the windows version."
            "\n- The plugin is incompatible with this blackbox version."
            ); break;
    case error_plugin_missing_entry    :
        msg = NLS2("$Error_Plugin_MissingEntry$",
            "The plugin misses the begin- and/or endPlugin entry point. Possible reasons:"
            "\n- The dll is not a plugin for Blackbox for Windows."
            ); break;
    case error_plugin_fail_to_load     :
        msg = NLS2("$Error_Plugin_IniFailed$",
            "The plugin could not be initialized."
            ); break;
    case error_plugin_crash_on_load    :
        msg = NLS2("$Error_Plugin_CrashedOnLoad$",
            "The plugin caused a general protection fault on initializing."
            "\nPlease contact the plugin author."
            ); break;
    case error_plugin_crash_on_unload  :
        msg = NLS2("$Error_Plugin_CrashedOnUnload$",
            "The plugin caused a general protection fault on shutdown."
            "\nPlease contact the plugin author."
            ); break;
        //default:
        //    msg = "(Unknown Error)";
    }
    BBMessageBox(
        MB_OK,
        NLS2("$Error_Plugin$", "Error: %s\n%s"),
        q->path,
        msg
        );
}

//===========================================================================
// load/unload one plugin

static int loadPlugin(struct PluginList *plugin, HWND hSlit, char** errorMsg) {
    if(plugin == nativeLoader.parent)
        return 0;
    
    if(isPluginLoader(plugin->path))
        return loadPluginLoader(plugin, errorMsg);

    struct PluginLoaderList *pll;
    int lastError = 0;

    dolist(pll, pluginLoaders) {
        lastError = pluginLoaders->LoadPlugin(plugin, hSlit, errorMsg);

        if(lastError == 0) {
            struct PluginPtr *pp = c_new(struct PluginPtr);
            pp->entry = plugin;

            append_node(&(pluginLoaders->plugins), pp);
            break;
        }
    }

    return lastError;
}

static int unloadPlugin(struct PluginList *q, char** errorMsg) {
    struct PluginLoaderList *pll;
    struct PluginPtr* pp;

    dolist(pll, pluginLoaders)
        if(q == pll->parent) {
            if(pll == &nativeLoader)
                return 0;

            q->hmodule = NULL;
            return unloadPluginLoader(pll, errorMsg);
        }

    dolist(pll, pluginLoaders) {
        dolist(pp, pll->plugins) {
            if(pp->entry == q) {
                int error = pll->UnloadPlugin(q, errorMsg);
                remove_item(&pll->plugins, pp);
                return error;
            }
        }
    }

    return 0;
}

static void free_plugin(struct PluginList **pp)
{
    struct PluginList *q = *pp;
    struct PluginLoaderList *pll;

    dolist(pll, pluginLoaders) {
        if(pll->parent == q) {
            if(pll == &nativeLoader)
                pll->parent = NULL;
            else
                remove_item(pluginLoaders, pll);

            break;
        }
    }

    free_str(&q->name);
    free_str(&q->path);
    *pp = q->next;
    m_free(q);
}

//===========================================================================
// write back the plugin list to "plugins.rc"

static bool write_plugins(void)
{
    FILE *fp;
    struct PluginList *q;
    const char *rcpath;

    rcpath = plugrcPath(NULL);
    if (NULL == (fp = create_rcfile(rcpath)))
        return false;

    dolist(q, bbplugins) {
        if (q->name) {
            if(!CheckFlag(q->flags, Plugin_IsEnabled))
                fprintf(fp,"! ");
            if(CheckFlag(q->flags, Plugin_UseSlit)) //  false != q->useslit)
                fprintf(fp,"& ");
        }
        fprintf(fp,"%s\n", q->path);
    }

    fclose(fp);
    get_filetime(rcpath, &rc_filetime);
    return true;
}

static bool isPluginLoader(char *path) {
    char ppl_path[MAX_PATH];
    if (0 == FindRCFile(ppl_path, path, NULL))
        return false;

    // HACK: this way of loading the dll is actually not recommended, but it is the cheapest way to work with the image
    auto pll = LoadLibraryEx(path, NULL, DONT_RESOLVE_DLL_REFERENCES);

    // walking the headers, checking consistency as we go
    auto dosHeader = (PIMAGE_DOS_HEADER)pll;

    if(dosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
        FreeLibrary(pll);
        return false;
    }

    auto ntHeader = (PIMAGE_NT_HEADERS)(dosHeader->e_lfanew + (SIZE_T)pll);
    if(ntHeader->Signature != IMAGE_NT_SIGNATURE) {
        FreeLibrary(pll);
        return false;
    }

#ifdef _WIN64
    if(ntHeader->FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64) {
#else
    if(ntHeader->FileHeader.Machine != IMAGE_FILE_MACHINE_I386) {
#endif
        FreeLibrary(pll);
        return false;
    }

    auto optHeader = &ntHeader->OptionalHeader;

    if(optHeader->Magic != IMAGE_NT_OPTIONAL_HDR_MAGIC) {
        FreeLibrary(pll);
        return false;
    }

    // checking whether the module exports the functions which a pluginLoader must define
    auto va = optHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;

    auto exportDir = (PIMAGE_EXPORT_DIRECTORY)(va + (SIZE_T)pll);
    auto numberOfFunctions = exportDir->NumberOfNames;
    auto names = (DWORD*)(exportDir->AddressOfNames + (SIZE_T)pll);

    auto numLoaderFunctions = 0;
    for(auto j = 0; pluginLoaderFunctionNames[j]; j++)
        numLoaderFunctions++;

    auto exportsFound = 0;

    for(unsigned long i = 0; i < numberOfFunctions; i++) {
        auto exportName = (char*)(*names + (SIZE_T)pll);

        for(int j=0; pluginLoaderFunctionNames[j]; j++) {
            if(!strcmp(exportName, pluginLoaderFunctionNames[j]))
                exportsFound++;
        }

        if(exportsFound == numLoaderFunctions)
            break;

        names++;
    }

    FreeLibrary(pll);

    return exportsFound == numLoaderFunctions;
}

static int loadPluginLoader(struct PluginList* plugin, char** errorMsg) {
    char ppl_path[MAX_PATH];
    if (0 == FindRCFile(ppl_path, plugin->path, NULL))
        return error_plugin_dll_not_found;

    auto r = SetErrorMode(0); // enable 'missing xxx.dll' system message
    auto module = LoadLibrary(ppl_path);
    SetErrorMode(r);

    if(module == NULL) {
        r = GetLastError();

        if (ERROR_MOD_NOT_FOUND == r)
            return error_plugin_dll_needs_module;
        else
            return error_plugin_does_not_load;
    }

    struct PluginLoaderList* pll = c_new(struct PluginLoaderList);
    pll->module = module;

    r = initPluginLoader(pll, errorMsg);

    if (0 == r) {
        plugin->hmodule = module;
        
        char* pName = (char *)c_alloc(sizeof(char) * strlen(pll->name) + 1);
        strcpy(plugin->name, pName);

        pll->parent = plugin;
        
        append_node(pluginLoaders, pll);

        return 0;
    }

    FreeLibrary(module);
    c_del(pll);
    return r;
}

static int initPluginLoader(struct PluginLoaderList* pll, char** errorMsg) {
    if(!LoadFunction(pll, Init))
        FailWithMsg(errorMsg, "init not present in pluginLoader");
    
    if(!LoadFunction(pll, Finalize))
        FailWithMsg(errorMsg, "finalize not present in pluginLoader");
    
    if(!LoadFunction(pll, GetName))
        FailWithMsg(errorMsg, "getName not present in pluginLoader");

    if(!LoadFunction(pll, GetApi))
        FailWithMsg(errorMsg, "getApi not present in pluginLoader");

    if(!LoadFunction(pll, LoadPlugin))
        FailWithMsg(errorMsg, "loadPlugin not present in pluginLoader");

    if(!LoadFunction(pll, UnloadPlugin))
        FailWithMsg(errorMsg, "unloadPlugin not present in pluginLoader");
    
    if(!pll->Init())
        FailWithMsg(errorMsg, "PluginLoader could not initialize.");

    pll->name = pll->GetName();
    pll->api = pll->GetApi();
    
    return 0;
}

static int unloadPluginLoader(struct PluginLoaderList *loader, char** errorMsg) {
    if(!loader->module)
        return 0;

    struct PluginPtr *pp;

    int lastError = 0;

    dolist(pp, loader->plugins) {
        lastError = loader->UnloadPlugin(pp->entry, errorMsg);
        remove_item(&loader->plugins, pp);
    }
    
    loader->Finalize();

    FreeLibrary(loader->module);
    loader->module = NULL;

    return lastError;
}

//===========================================================================


void PluginManager_aboutPlugins(void)
{
    int l, x = 0;
    char *msg = (char*)c_alloc(l = 4096);
    const char* (*pluginInfo)(int);
    struct PluginList *q;
    
    dolist(q, bbplugins)
        if (q->hmodule || q == nativeLoader.parent) {
            if (l - x < MAX_PATH + 100)
                msg = (char*)m_realloc(msg, l*=2);
            if (x)
                msg[x++] = '\n';
            pluginInfo = q->pluginInfo;
            if (pluginInfo)
                x += sprintf(msg + x,
                "%s %s %s %s (%s)\t",
                pluginInfo(PLUGIN_NAME),
                pluginInfo(PLUGIN_VERSION),
                NLS2("$About_Plugins_By$", "by"),
                pluginInfo(PLUGIN_AUTHOR),
                pluginInfo(PLUGIN_RELEASE)
                );
            else
                x += sprintf(msg + x, "%s\t", q->name);
        }

        BBMessageBox(MB_OK,
            "#"BBAPPNAME" - %s#%s\t",
            NLS2("$About_Plugins_Title$", "About loaded plugins"),
            x ? msg : NLS1("No plugins loaded.")
            );

        m_free(msg);
}

//===========================================================================
// OpenFileName Dialog to add plugins

#ifndef BBTINY
#include <commdlg.h>
#ifndef OPENFILENAME_SIZE_VERSION_400
#define OPENFILENAME_SIZE_VERSION_400  CDSIZEOF_STRUCT(OPENFILENAME,lpTemplateName)
#endif // (_WIN32_WINNT >= 0x0500)
static UINT_PTR APIENTRY OFNHookProc(HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    if (WM_INITDIALOG == uiMsg)
    {
        // center 'open file' dialog on screen
        RECT r, w;
        if (WS_CHILD & GetWindowLong(hdlg, GWL_STYLE))
            hdlg = GetParent(hdlg);
        GetWindowRect(hdlg, &r);
        GetMonitorRect(hdlg, &w, GETMON_WORKAREA|GETMON_FROM_WINDOW);
        MoveWindow(hdlg,
            (w.right+w.left+r.left-r.right) / 2,
            (w.bottom+w.top+r.top-r.bottom) / 2,
            r.right - r.left,
            r.bottom - r.top,
            TRUE
            );
    }
    return 0;
}

static bool browse_file(char *buffer, const char *title, const char *filters)
{
    OPENFILENAME ofCmdFile;
    static BOOL (WINAPI *pGetOpenFileName)(LPOPENFILENAME lpofn);
    bool ret = false;

    memset(&ofCmdFile, 0, sizeof(OPENFILENAME));
    ofCmdFile.lStructSize = sizeof(OPENFILENAME);
    ofCmdFile.lpstrFilter = filters;
    ofCmdFile.nFilterIndex = 1;
    //ofCmdFile.hwndOwner = NULL;
    ofCmdFile.lpstrFile = buffer;
    ofCmdFile.nMaxFile = MAX_PATH;
    //ofCmdFile.lpstrFileTitle = NULL;
    //ofCmdFile.nMaxFileTitle = 0;
    {
        static bool init;
        char plugin_dir[MAX_PATH];
        if (!init) {
            ofCmdFile.lpstrInitialDir = set_my_path(NULL, plugin_dir, "plugins");
            init = true;
        }
    }
    ofCmdFile.lpstrTitle = title;
    ofCmdFile.Flags = OFN_EXPLORER|OFN_FILEMUSTEXIST|OFN_ENABLEHOOK|OFN_NOCHANGEDIR;
    ofCmdFile.lpfnHook = OFNHookProc;
    if (LOBYTE(GetVersion()) < 5) // win9x/me
        ofCmdFile.lStructSize = OPENFILENAME_SIZE_VERSION_400;
    if ((DWORD)-1 == GetFileAttributes(ofCmdFile.lpstrFile))
        ofCmdFile.lpstrFile[0] = 0;

    if (load_imp(&pGetOpenFileName, "comdlg32.dll", "GetOpenFileNameA"))
        ret = FALSE != pGetOpenFileName(&ofCmdFile);
    return ret;
}

#endif //ndef BBTINY

//===========================================================================
// The plugin configuration menu

static Menu *get_menu(const char *title, char *menu_id, bool pop, struct PluginList **qp, bool b_slit)
{
    struct PluginList *q;
    char *end_id;
    MenuItem *pItem;
    char command[20], label[80], broam[MAX_PATH+80];
    const char *cp;
    Menu *pMenu, *pSub;

    end_id = strchr(menu_id, 0);
    pMenu = MakeNamedMenu(title, menu_id, pop);

    while (NULL != (q = *qp)) {
        *qp = q->next;
        if (q->name) {
            if (0 == b_slit) {
                sprintf(broam, "@BBCfg.plugin.load %s", q->name);
                pItem = MakeMenuItem(pMenu, q->name, broam, CheckFlag(q->flags, Plugin_IsEnabled));
#if 0
                sprintf(end_id, "_opt_%s", q->name);
                pSub = MakeNamedMenu(q->name, menu_id, pop);
                MenuItemOption(pItem, BBMENUITEM_RMENU, pSub);

                MakeMenuItem(pSub, "Load", broam, q->enabled);
                sprintf(broam, "@BBCfg.plugin.inslit %s", q->name);
                MakeMenuItem(pSub, "In Slit", broam, q->useslit);
#endif
            } else if(CheckFlag(q->flags, Plugin_IsEnabled) && (q->beginPluginEx || q->beginSlitPlugin)) {
                sprintf(broam, "@BBCfg.plugin.inslit %s", q->name);
                MakeMenuItem(pMenu, q->name, broam, CheckFlag(q->flags, Plugin_UseSlit)); // q->useslit);
            }
        } else if (0 == b_slit) {
            cp = q->path;
            if (false == get_string_within(command, sizeof command, &cp, "[]"))
                continue;
            get_string_within(label, sizeof label, &cp, "()");
            if (0 == _stricmp(command, "nop"))
                MakeMenuNOP(pMenu, label);
            else if (0 == _stricmp(command, "sep"))
                MakeMenuNOP(pMenu, NULL);
            else if (0 == _stricmp(command, "submenu") && *label) {
                sprintf(end_id, "_%s", label);
                pSub = get_menu(label, menu_id, pop, qp, b_slit);
                MakeSubmenu(pMenu, pSub, NULL);
            } else if (0 == _stricmp(command, "end")) {
                break;
            }
        }
    }
    return pMenu;
}

Menu* PluginManager_GetMenu(const char *text, char *menu_id, bool pop, int mode)
{
    struct PluginList *q = bbplugins->next;
    if (SUB_PLUGIN_SLIT == mode && NULL == hSlit)
        return NULL;
    return get_menu(text, menu_id, pop, &q, SUB_PLUGIN_SLIT == mode);
}

//===========================================================================
// parse a line from plugins.rc to obtain the pluginRC address

bool parse_pluginRC(const char *rcline, const char *name)
{
    bool is_comment = false;

    if ('#' == *rcline || 0 == *rcline)
        is_comment = true;
    else
        if ('!' == *rcline)
            while (' '== *++rcline);

    if ('&' == *rcline)
    {
        while (' '== *++rcline);
    }

    if (!is_comment && IsInString(rcline, name))
    {
        char editor[MAX_PATH];
        char road[MAX_PATH];
        char buffer[2*MAX_PATH];
        char *s = strcpy((char*)name, rcline);
        *(char*)file_extension(s) = 0; // strip ".dll"
        if (IsInString(s, "+"))
            *(char*)get_delim(s, '+') = 0; // strip "+"
        rcline = (const char*)strcat((char*)s, ".rc");
        GetBlackboxEditor(editor);
        sprintf(buffer, "\"%s\" \"%s\"", editor, set_my_path(hMainInstance, road, rcline));
        BBExecute_string(buffer, RUN_SHOWERRORS);
        return true;
    }
    return false;
}

//===========================================================================
// parse a line from plugins.rc to obtain the Documentation address

bool parse_pluginDocs(const char *rcline, const char *name)
{
    bool is_comment = false;

    if ('#' == *rcline || 0 == *rcline)
        is_comment = true;
    else
        if ('!' == *rcline)
            while (' '== *++rcline);

    if ('&' == *rcline)
    {
        while (' '== *++rcline);
    }

    if (!is_comment && IsInString(rcline, name))
    {
        char road[MAX_PATH];
        char *s = strcpy((char*)name, rcline);
        *(char*)file_extension(s) = 0; // strip ".dll"
        // files could be *.html, *.htm, *.txt, *.xml ...
        if (locate_file(hMainInstance, road, s, "html") || locate_file(hMainInstance, road, s, "htm") || locate_file(hMainInstance, road, s, "txt") || locate_file(hMainInstance, road, s, "xml"))
        {
            BBExecute(NULL, "open", road, NULL, NULL, SW_SHOWNORMAL, 		false);
            return true;
        }
        *(char*)get_delim(s, '\\') = 0; // strip plugin name
        rcline = (const char*)strcat((char*)s, "\\readme");
        // ... also the old standby: readme.txt
        if (locate_file(hMainInstance, road, rcline, "txt"))
        {
            BBExecute(NULL, "open", road, NULL, NULL, SW_SHOWNORMAL, 		false);
            return true;
        }
    }
    return false;
}

//===========================================================================
// handle the "@BBCfg.plugin.xxx" bro@ms from the config->plugins menu

int PluginManager_handleBroam(const char *args)
{
    static const char * const actions[] = {
        "add", "remove", "load", "inslit", "edit", "docs", NULL
    };
    enum {
        e_add, e_remove, e_load, e_inslit, e_edit, e_docs
    };

    char buffer[MAX_PATH];
    struct PluginList *q;
    int action;

    NextToken(buffer, &args, NULL);
    action = get_string_index(buffer, actions);
    if (-1 == action)
        return 0;

    NextToken(buffer, &args, NULL);
    if (action > 3)
    {
        //check for multiple loadings
        if (IsInString(buffer, "/"))
        {
            char path[1];
            char *p = strcpy(path, buffer);
            *(char*)get_delim(p, '/') = 0; // strip "/#"
            strcpy(buffer, (const char*)strcat((char*)p, ".dll"));
        }


        char szBuffer[MAX_PATH];
        const char *path=plugrcPath();

        FILE *fp = fopen(path,"rb");
        if (fp)
        {
            if (e_edit == action)
                while (read_next_line(fp, szBuffer, sizeof szBuffer))
                    parse_pluginRC(szBuffer, buffer);
            else
                while (read_next_line(fp, szBuffer, sizeof szBuffer))
                    parse_pluginDocs(szBuffer, buffer);
            fclose(fp);
        }
    }

    if (e_add == action && 0 == buffer[0])
    {
#ifndef BBTINY
        if (false == browse_file(buffer,
            NLS1("Add Plugin"),
            "Plugins (*.dll)\0*.dll\0All Files (*.*)\0*.*\0"))
#endif
            return 1;
    }

    strcpy(buffer, get_relative_path(NULL, unquote(buffer)));

    if (e_add == action) {
        q = parseConfigLine(buffer);
        append_node(&bbplugins, q);

        SetFlag(q->flags, Plugin_UseSlit, true);
        return 1;
    }

    //dolist (q, bbplugins)
    //    if (q->name && !_stricmp(q->name, buffer))
    //        break;

    skipUntil(q, bbplugins, q->name && !_stricmp(q->name, buffer));

    if (q == NULL)
        return 1;

    if (e_remove == action)
        ClearFlag(q->flags, Plugin_IsEnabled);
    else if (e_load == action)
        if(get_false_true(args) == -1)
            ToggleFlag(q->flags, Plugin_IsEnabled);
        else
            SetFlag(q->flags, Plugin_IsEnabled, !!get_false_true(args));
    else if (e_inslit == action)
        if(get_false_true(args) == -1)
            ToggleFlag(q->flags, Plugin_UseSlit);
        else
            SetFlag(q->flags, Plugin_UseSlit, !!get_false_true(args));

    applyPluginStates();

    if (e_remove == action || (e_add == action && !CheckFlag(q->flags, Plugin_IsEnabled)))
        free_plugin((struct PluginList **)member_ptr(&bbplugins, q));

    write_plugins();

    return 1;
}

//===========================================================================
