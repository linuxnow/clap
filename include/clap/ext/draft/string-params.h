#pragma once

#include "../../plugin.h"
#include "../../string-sizes.h"

/// @page String Params
///
/// This extension lets the plugin expose values which are strings rather than numbers, mostly the
/// path of a file it loads: a sample, an impulse response, a wavetable, a SFZ file, a model. The
/// host can list them, read them and set them without the plugin's GUI.
///
/// This is useful for hosts which have no room for the plugin's GUI, or no display at all:
/// embedded and headless hosts, hardware units, control surfaces and remote controllers. They can
/// then let the user pick a file with their own browser and hand the path to the plugin.
///
/// These are not parameters in the sense of clap.params: they have no range, they can't be
/// automated or modulated, they are never sent as events and they never reach the audio thread
/// through the host. Everything happens on the main-thread; the plugin applies a new value the
/// same way it applies one chosen from its own GUI.
///
/// Values are part of the plugin's state: clap_plugin_state.save() stores them and
/// clap_plugin_state.load() restores them. After a state load, the host reads them again.
///
/// The paths a plugin stores in its state are its own business; the plugin can store them
/// relative to a directory of clap.resource-directory if it uses that extension. The values seen
/// through this extension are always absolute paths.

static CLAP_CONSTEXPR const char CLAP_EXT_STRING_PARAMS[] = "clap.string-params/1";

#ifdef __cplusplus
extern "C" {
#endif

enum clap_string_param_kind {
   // Any UTF-8 text, eg: a name, a URL, an expression.
   CLAP_STRING_PARAM_PLAIN = 0,

   // An absolute path to a file. The host may offer a file browser to choose it.
   CLAP_STRING_PARAM_FILE = 1,

   // An absolute path to a directory. The host may offer a directory browser to choose it.
   CLAP_STRING_PARAM_DIRECTORY = 2,

   // Note: more entries could be added here in the future
};

enum clap_string_param_flags {
   // The value can only be read by the host: the plugin sets it, and set_value() returns false.
   // eg: the name of the patch loaded from the file of another string param.
   CLAP_STRING_PARAM_IS_READONLY = 1 << 0,
};

typedef struct clap_string_param_info {
   // Stable identifier, it must never change. It lives in its own namespace: it may overlap with
   // the ids of clap.params.
   clap_id id;

   // Displayable name, eg: "Sample", "Impulse Response".
   char name[CLAP_NAME_SIZE];

   // see enum clap_string_param_kind
   uint32_t kind;

   // bitmask of clap_string_param_flags
   uint32_t flags;

   // For CLAP_STRING_PARAM_FILE, the file extensions the plugin can load, without the dot and
   // separated by ';', eg: "wav;flac;aiff". Matched without regard to case.
   // Empty if any file can be given, and for the other kinds.
   char file_extensions[CLAP_NAME_SIZE];
} clap_string_param_info_t;

typedef struct clap_plugin_string_params {
   // Returns the number of string params.
   // [main-thread]
   uint32_t(CLAP_ABI *count)(const clap_plugin_t *plugin);

   // Copies the info of the string param at index into info and returns true on success.
   // [main-thread]
   bool(CLAP_ABI *get_info)(const clap_plugin_t      *plugin,
                            uint32_t                  index,
                            clap_string_param_info_t *info);

   // Copies the current value of the string param param_id into value, as a null terminated
   // UTF-8 string of at most capacity bytes, null included; a longer value is truncated.
   //
   // Returns the length of the whole value in bytes, without the null terminator, so a host can
   // call it again with a buffer of the returned length plus one. Returns -1 if param_id is
   // unknown. An empty value, eg: no file loaded, has length 0.
   //
   // The value is the one the plugin uses: after set_value() it is the value just set, even if
   // the plugin is still loading the file in the background.
   // [main-thread]
   int32_t(CLAP_ABI *get_value)(const clap_plugin_t *plugin,
                                clap_id              param_id,
                                char                *value,
                                uint32_t             capacity);

   // Sets the value of the string param param_id. value is a null terminated UTF-8 string; for
   // a file or a directory it is an absolute path, and an empty string clears it.
   //
   // The plugin copies what it needs: value is only valid for the duration of the call and stays
   // owned by the host.
   //
   // This can be called while the plugin is active and processing. The plugin hands the change to
   // its audio thread itself, without blocking it, as it does for a file chosen from its own GUI;
   // the loading may happen later, on another thread.
   //
   // Returns true if the plugin accepted the value. Returns false, and keeps the previous value,
   // if param_id is unknown, if the string param is read-only, or if the plugin refuses the value,
   // eg: a file it can't open or of a format it can't read.
   //
   // A successful call changes the plugin's state. The host made the change and knows it, so the
   // plugin doesn't call clap_host_string_params.value_changed() for it.
   // [main-thread]
   bool(CLAP_ABI *set_value)(const clap_plugin_t *plugin, clap_id param_id, const char *value);
} clap_plugin_string_params_t;

typedef struct clap_host_string_params {
   // Informs the host that the list of string params or their info changed.
   // The host will call count() and get_info() again.
   // [main-thread]
   void(CLAP_ABI *changed)(const clap_host_t *host);

   // Informs the host that the plugin changed the value of param_id by itself, eg: from its GUI,
   // or because the loading failed and it went back to the previous file. The host will call
   // get_value() again.
   //
   // A value change makes the state dirty, so the plugin also calls clap_host_state.mark_dirty().
   // [main-thread]
   void(CLAP_ABI *value_changed)(const clap_host_t *host, clap_id param_id);
} clap_host_string_params_t;

#ifdef __cplusplus
}
#endif
