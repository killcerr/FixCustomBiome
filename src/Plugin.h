#pragma once

#include <ll/api/plugin/NativePlugin.h>

namespace FixCustomBiomePlugin {

class Plugin {
public:
    explicit Plugin(ll::plugin::NativePlugin& self);

    Plugin(Plugin&&)                 = delete;
    Plugin(const Plugin&)            = delete;
    Plugin& operator=(Plugin&&)      = delete;
    Plugin& operator=(const Plugin&) = delete;

    ~Plugin() = default;

    /// @return True if the plugin is enabled successfully.
    bool enable();

    /// @return True if the plugin is disabled successfully.
    bool disable();

    const ll::Logger& getLogger();

private:
    ll::plugin::NativePlugin& mSelf;
};
static ll::Logger* logger;
} // namespace FixCustomBiomePlugin
