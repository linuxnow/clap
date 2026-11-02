#pragma once

#include "../audio-ports.h"

/// @page Audio Ports Side Chain
///
/// This extension lets the plugin tell the host which of its audio inputs are side chain inputs
/// (also called key inputs), and which input each one keys.
///
/// A side chain input carries a signal the plugin listens to in order to process another one: the
/// key of a compressor or a gate, the modulator of a vocoder. Knowing this, the host can offer the
/// port in its side chain routing, show it as a key rather than as one more input, and leave it
/// out when it connects the plugin's inputs by default.
///
/// Not every input that is not the main one is a side chain: the sixteen inputs of an analyzer or
/// the many inputs of a mixer are not. A port is only a side chain when this extension says so.
/// A host which doesn't support this extension sees the ports exactly as clap.audio-ports
/// describes them.
///
/// The extension only describes the ports. Which signal the host routes to a side chain input,
/// and whether it routes one at all, stays the host's and the user's choice. When nothing is
/// routed to it, the host processes the port with silence, as with any other input.

static CLAP_CONSTEXPR const char CLAP_EXT_AUDIO_PORTS_SIDECHAIN[] = "clap.audio-ports-sidechain/1";

#ifdef __cplusplus
extern "C" {
#endif

enum clap_audio_port_sidechain_flags {
   // This side chain input is the one the host should use when it offers a single side chain for
   // the whole plugin, for example a "side chain source" selector in the plugin's slot.
   // At most one input port has this flag.
   CLAP_AUDIO_PORT_SIDECHAIN_IS_PRIMARY = 1 << 0,
};

typedef struct clap_audio_port_sidechain_info {
   // The id of the input port this side chain keys, as in clap_audio_port_info.id, usually the
   // main input. Several side chains may key the same port, and each input of a multi-channel
   // plugin may have its own side chain.
   //
   // CLAP_INVALID_ID if the side chain does not key one particular input: it keys the whole
   // plugin, or the plugin has no main input at all, as with a vocoder whose carrier is generated
   // internally.
   clap_id keyed_port_id;

   // bitmask of clap_audio_port_sidechain_flags
   uint32_t flags;
} clap_audio_port_sidechain_info_t;

typedef struct clap_plugin_audio_ports_sidechain {
   // Returns true if the input port at index is a side chain input, and stores its info into info.
   // Returns false if it is not a side chain, or if index is out of range; info is then left
   // untouched.
   //
   // index is the same index as clap_plugin_audio_ports.get() with is_input set to true. A port
   // with CLAP_AUDIO_PORT_IS_MAIN is never a side chain.
   //
   // The answer can only change when the audio ports themselves change: the host reads it again
   // each time it rescans the audio ports with CLAP_AUDIO_PORTS_RESCAN_LIST or
   // CLAP_AUDIO_PORTS_RESCAN_FLAGS, and like the audio ports scan this is done while the plugin is
   // deactivated.
   // [main-thread]
   bool(CLAP_ABI *get)(const clap_plugin_t              *plugin,
                       uint32_t                          index,
                       clap_audio_port_sidechain_info_t *info);
} clap_plugin_audio_ports_sidechain_t;

#ifdef __cplusplus
}
#endif
