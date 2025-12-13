#include "ect/json.hpp"
#include <fstream>
#include <iostream>
#include "config.h"

namespace Config {
	nlohmann::json config;

	void load(const std::string& filename) {
		std::ifstream file(filename);
		if (file.is_open()) {
			try {
				file >> config;
			} catch (const std::exception& e) {
				std::cerr << "failed to parse cfg: " << e.what() << std::endl;
			}
		} else {
			std::cerr << "cant open config: " << filename << std::endl;
		}
	}

	void save(const std::string& filename) {
		std::ofstream file(filename);
		if (file.is_open()) {
			file << config.dump(4); // Pretty print with 4 spaces
		} else {
			std::cerr << "cant write config: " << filename << std::endl;
		}
	}

	bool getAimbotEnabled() {
		if (config.contains("aimbot_enabled")) {
			return config["aimbot_enabled"].get<bool>();
		}
		return false; 
	}

	void setAimbotEnabled(bool enabled) {
		config["aimbot_enabled"] = enabled;
	}

    void setfov(float newFov){
        config["FOV"] = newFov;
    }

	float getfov(){
		if (config.contains("FOV")) {
			return config["FOV"].get<float>();
		}

		return 115; //115 for almost default fov
	}

	bool getfovcircleenabled() {
		if (config.contains("fov_circle_enabled")) {
			return config["fov_circle_enabled"].get<bool>();
		}
		return false; 
	}

	void setfovcircleenabled(bool value) {
		config["fov_circle_enabled"] = value;
	}
}