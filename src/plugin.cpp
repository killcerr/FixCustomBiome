#include <ll/api/plugin/NativePlugin.h>

#include "Plugin.h"
extern void plugin_init();
namespace FixCustomBiomePlugin {

Plugin::Plugin(ll::plugin::NativePlugin& self) : mSelf(self) {
    mSelf.getLogger().info("loading...");
    // Code for loading the plugin goes here.
}

bool Plugin::enable() {
    mSelf.getLogger().info("enabling...");

    // Code for enabling the plugin goes here.
    return true;
}

bool Plugin::disable() {
    mSelf.getLogger().info("disabling...");

    // Code for disabling the plugin goes here.

    return true;
}
} // namespace FixCustomBiomePlugin
