#pragma once

#include <cstdlib>

static std::string base_url = "mediafs.azurewebsites.net";
static std::string metadata_url = base_url + "/api/metadata";
static std::string song_url = base_url + "/api/song";

bool execute(const char* cmd);
bool executeAndReloadMetadata(std::string);
std::string getSongId(const char* path);
