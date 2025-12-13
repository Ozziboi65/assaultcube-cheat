
#pragma once
#include <string>
#include "json.hpp" 

namespace Config {
	extern nlohmann::json config;

	void load(const std::string& filename);
	void save(const std::string& filename); // Save config to file

	bool getAimbotEnabled();
	float getfov();
	bool getfovcircleenabled();

	void setAimbotEnabled(bool enabled); 
    void setfov(float newFov);
	void setfovcircleenabled(bool enabled);
}
