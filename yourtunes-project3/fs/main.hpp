#pragma once

#include <cstdlib>

static std::string base_url = "mediafs.azurewebsites.net";
static std::string metadata_url = base_url + "/api/metadata";
static std::string song_url = base_url + "/api/song";
static std::string download_url = "https://cs3210mediafs.blob.core.windows.net/media/";

bool execute(const char* cmd);
bool executeAndReloadMetadata(std::string);
std::string getSongId(const char* path);
