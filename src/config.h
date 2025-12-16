
#pragma once
#include <string>
#include "json.hpp" 
#include "imgui.h"

namespace Config {
	extern nlohmann::json config;

	void load(const std::string& filename);
	void save(const std::string& filename); // Save config to file

	bool getAimbotEnabled();
	float getfov();
	bool getfovcircleenabled();
	float getAimbotFov();
	float getAimbotmaxdist();
	bool getsnaplines();
	bool getsnaplinesall();
	bool getEspNames();
	int gethumanize();

	//vectors
	ImVec4 getenemyespcolor();

	void setAimbotEnabled(bool enabled); 
    void setfov(float newFov);
	void setfovcircleenabled(bool enabled);
	void setaimbotfov(float newAimbotFov);
	void setaimbotdist(float newAimbotdist);
	void setsnaplines(bool enablesnaplines);
	void setsnaplinesall(bool snaplinesaALL);
	void setEspNames(bool espNames);
	void sethumanize(int humanize);
	void setEspColor(ImVec4 EspColor);
}
