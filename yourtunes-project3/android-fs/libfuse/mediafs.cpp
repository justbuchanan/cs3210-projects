#define FUSE_USE_VERSION 26

#include <fstream>
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include <string>
#include <vector>

#include <sstream>
#include <iostream>

#include <cstdlib>

#include "json.hpp"

using json = nlohmann::json;
using namespace std;

static string base_url = "mediafs.azurewebsites.net";
static string metadata_url = base_url + "/api/metadata";
static string song_url = base_url + "/api/song";
static string download_url = "https://cs3210mediafs.blob.core.windows.net/media/";

bool execute(const char* cmd);
bool executeAndReloadMetadata(string);
string getSongId(const char* path);

string exec(const char* cmd);
void initData(void);

static json MetadataJson;

const string MusicGroupings[] = { string("Albums"), string("Decades"), string("Artists"), string("Genres")};

static int endsWith(const char* str, const char* end) {
    int lenstr = strlen(str),
        lenend = strlen(end);
    return strcmp(str + (lenstr - lenend), end);
}

// example: splitPath("/Albums/Test/Song.mp3") -> ["Albums", "Test", "Song.mp3"]
// example: splitPath("/") -> []
vector<string> splitPath(string str) {
    vector<string> components;

    if (str[0] == '/') str = str.substr(1);

    stringstream ss(str);
    string part;
    while (getline(ss, part, '/')) components.push_back(part);

    return components;
}

std::string getSongId(const char* path) {
    vector<string> pathComponents = splitPath(string(path));
    string category = pathComponents[0];
    string subpath = pathComponents[1];
    string songName = pathComponents[2].substr(0, pathComponents[2].find_last_of("."));
    for (auto collection : MetadataJson[category]) {
        string collectionName = collection["title"].get<string>();
        if (subpath == collectionName) {
            for (auto song : collection["songs"]) {
                if (song["title"].get<string>() == songName) {
                    string id = song["id"].get<string>();
                    return id;
                }
            }
        }
    }
    return "";
}

static int ytfs_getattr(const char *path, struct stat *stbuf) {
    memset(stbuf, 0, sizeof(struct stat));

    vector<string> pathComponents = splitPath(string(path));

    // Check if root directory
    if (pathComponents.size() == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        return 0;
    }

    // See if it's /Albums or /Decades or /Artists or /Genres
    if (pathComponents.size() == 1) {
        // For mp3s being copied to the root directory
        if (endsWith(path, ".mp3") == 0) {
            stbuf->st_mode = S_IFREG | 0666;
            stbuf->st_nlink = 1;
            stbuf->st_size = 0;
        } else {
            stbuf->st_mode = S_IFDIR | 0755;
            stbuf->st_nlink = 2;
        }
        return 0;
    }

    auto grouping = MetadataJson[pathComponents[0]];

    for (auto folder : grouping) {
        if (folder["title"].get<string>() == pathComponents[1]) {
            if (pathComponents.size() == 2) {
                stbuf->st_mode = S_IFDIR | 0755;
                stbuf->st_nlink = 2;
                return 0;
            }

            for (auto song : folder["songs"]) {
                string fileWithoutExt = pathComponents[2].substr(0, pathComponents[2].size() - 4);
                if (song["title"].get<string>() == fileWithoutExt) {
                    stbuf->st_mode = S_IFREG | 0666;
                    stbuf->st_nlink = 1;
                    stbuf->st_size = song["size"].get<off_t>();
                    return 0;
                }
            }
        }
    }

    return -ENOENT;
}

static int ytfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			 off_t offset, struct fuse_file_info *fi) {
    vector<string> pathComponents = splitPath(string(path));

    filler(buf, ".", nullptr, 0);
    filler(buf, "..", nullptr, 0);

	if (pathComponents.size() == 0) {
        for (auto cat : MusicGroupings) filler(buf, cat.c_str(), nullptr, 0);
    } else if (pathComponents.size() == 1) {
        // /Albums or /Decades or /Artists or /Genres
        string category = pathComponents[0];
        for (auto subdir : MetadataJson[category]) {
            filler(buf, subdir["title"].get<std::string>().c_str(), nullptr, 0);
        }
    } else if (pathComponents.size() == 2) {
        string category = pathComponents[0];
        bool found = false;
        string subpath = pathComponents[1];
        for (auto collection : MetadataJson[category]) {
            string collectionName = collection["title"].get<string>();
            if (subpath == collectionName) {
                for (auto song : collection["songs"]) {
                    filler(buf, (song["title"].get<string>() + ".mp3").c_str(), nullptr, 0);
                }
                found = true;
                break;
            }
        }

        if (!found) return -ENOENT;
    }

    return 0;
}

static int ytfs_open(const char *path, struct fuse_file_info *fi) {

    const char* mp3 = ".mp3";
    if (endsWith(path, mp3) != 0) {      
        return -ENOENT;
    }

	return 0;
}

static int ytfs_read(const char *path, char *buf, size_t size, off_t offset,
		      struct fuse_file_info *fi) {

    if (endsWith(path, ".mp3") != 0) {      
        return -ENOENT;
    }

    string id = getSongId(path);

    ifstream file;
    file.open("/sdcard/MediaFS/" + id + ".mp3", ios::binary);
    if(!file.is_open()) {
        string path = download_url + id + ".mp3";
        if(!execute(("curl " + path + " > " + "/sdcard/MediaFS/" + id + ".mp3").c_str()))
            return -ENOENT;

        file.open("/sdcard/MediaFS/" + id + ".mp3", ios::binary);
        if(!file.is_open())
            return -ENOENT;
    }

    file.seekg(0, file.end);
    int count = 0;
    if(file.tellg() > offset) {
        file.seekg(offset);
        file.read(buf, size);
        count = file.gcount();
    }
    file.close();
    return count;
}

static int ytfs_write(const char* path, const char* buf, size_t size, off_t offset,
		      struct fuse_file_info *fi) {
    return 0;
}

static int ytfs_create(const char* path, mode_t mode, struct fuse_file_info* fi) {
    return 0;
}

static int ytfs_chmod(const char* path, mode_t mode) {
    return 0;
}

static int ytfs_chown(const char* path, uid_t uid, gid_t gid) {
    return 0;
}

static int ytfs_utimens(const char* path, const struct timespec tv[2]) {
    return 0;
}

static int ytfs_truncate(const char* path, off_t offset) {
    return 0;
}

static int ytfs_flush(const char* path, struct fuse_file_info* fi) {
    return 0;
}

static int ytfs_unlink(const char* path) {
    return 0;
}

static void ytfs_destroy(void* v) {
    
}

static struct fuse_operations ytfs_oper = {
	.getattr	= ytfs_getattr,
	.readdir	= ytfs_readdir,
	.open		= ytfs_open,
	.read		= ytfs_read,
    .write      = ytfs_write,
    .create     = ytfs_create,
    .chmod      = ytfs_chmod,
    .chown      = ytfs_chown,
    .utimens    = ytfs_utimens,
    .truncate   = ytfs_truncate,
    .flush      = ytfs_flush,
    .unlink     = ytfs_unlink,
    .destroy    = ytfs_destroy,
};

bool execute(const char* cmd) {
    int res = system(cmd);

    return res == 0;
}

bool executeAndReloadMetadata(string cmd) {
    cmd += " > /sdcard/MediaFS/metadata.json";
    const char* outCmd = cmd.c_str();

    if (!execute(outCmd))
        return false;

    //string json;

    fstream fstr;
    fstr.open("/sdcard/MediaFS/metadata.json", fstream::in);
    
    //fstr.seekg(0, ios::end);   
    //json.reserve(fstr.tellg());
    //fstr.seekg(0, ios::beg);

    //json.assign((istreambuf_iterator<char>(fstr)), istreambuf_iterator<char>());
    //MetadataJson = Json(json);
    MetadataJson = json::parse(fstr);
    fstr.close();

    return true;
}

void initData(void) {
    executeAndReloadMetadata("curl " + metadata_url);
}

int main(int argc, char *argv[]) {
    initData();
	return fuse_main(argc, argv, &ytfs_oper, nullptr);
}
