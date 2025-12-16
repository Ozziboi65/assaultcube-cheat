#include "ect/json.hpp"
#include <fstream>
#include <iostream>
#include "config.h"
#include "imgui.h"

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

    void sethumanize(int value){
        config["humanize"] = value;
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

	void setsnaplines(bool enablesnaplines) {
		config["enablesnaplines"] = enablesnaplines;
	}

	void setsnaplinesall(bool value) {
		config["snaplinesALL"] = value;
	}


	void setEspNames(bool value) {
		config["EspNames"] = value;
	}



	void setaimbotfov(float newAimbotFov){
		config["aimbotfov"] = newAimbotFov;
	}

	void setaimbotdist(float newAimbotdist){
		config["aimbotdist"] = newAimbotdist;
	}

	void setEspColor(ImVec4 EspColor) {
		config["enemyEspColor"] = { EspColor.x, EspColor.y, EspColor.z, EspColor.w };
	}

	float getAimbotFov(){
		if (config.contains("aimbotfov")) {
			return config["aimbotfov"].get<float>();
		}
		return 90; // default aimbot fov if fail
	}

	float getAimbotmaxdist(){
		if (config.contains("aimbotdist")) {
			return config["aimbotdist"].get<float>();
		}

		return 750; //750 for  aim dist if fail
	}

	int gethumanize(){
		if (config.contains("humanize")) {
			return config["humanize"].get<int>();
		}

		return 0;
	}

	ImVec4 getenemyespcolor(){
		if (config.contains("enemyEspColor") && config["enemyEspColor"].is_array() && config["enemyEspColor"].size() == 4) {
			const auto& arr = config["enemyEspColor"];
			return ImVec4(
				arr[0].get<float>(),
				arr[1].get<float>(),
				arr[2].get<float>(),
				arr[3].get<float>()
			);
		}
		return ImVec4(2.55f, 0.0f, 0.0f, 1.00f); // default ESP color if fail
	}


	bool getsnaplines() {
		if (config.contains("enablesnaplines")) {
			return config["enablesnaplines"].get<bool>();
		}
		return true; 
	}

	bool getsnaplinesall() {
		if (config.contains("snaplinesALL")) {
			return config["snaplinesALL"].get<bool>();
		}
		return true; 
	}

	bool getEspNames() {
		if (config.contains("EspNames")) {
			return config["EspNames"].get<bool>();
		}
		return true; 
	}


}