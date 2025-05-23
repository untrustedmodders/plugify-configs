#include "config.hpp"
#include "plugin.hpp"
#include <memory>
#include <string_view>

#include <plugify-configs/plugify-configs.hpp>
#include <plugify_configs_export.h>

namespace pcf {
	extern ConfigsPlugin plugin;

	std::unique_ptr<pcf::Config> ReadConfig(std::string_view path) {
		return pcf::plugin.ReadConfig(path);
	}

	std::unique_ptr<pcf::Config> ReadConfigs(std::span<const std::string_view> paths) {
		return pcf::plugin.ReadConfigs(paths);
	}

	std::unique_ptr<pcf::Config> MakeConfig() {
		return pcf::Config::Detail::MakeConfig();
	}

	void SetError(std::string_view error) {
		pcf::plugin.SetError(error);
	}

	std::string_view GetError() {
		return pcf::plugin.GetError();
	}
}
