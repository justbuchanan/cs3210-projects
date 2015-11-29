#define FUSE_USE_VERSION 26

#include <fstream>
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include <string>
#include <vector>
#include <thread>

#include <iostream>

#include <taglib/fileref.h>
#include <taglib/tag.h>

#include "main.hpp"
#include "json.hpp"

using namespace TagLib;
using json = nlohmann::json;
using namespace std;

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
            return 0;
        } else {
            for( auto grouping : MusicGroupings) {
                if(grouping == pathComponents[0]) {
                    stbuf->st_mode = S_IFDIR | 0755;
                    stbuf->st_nlink = 2;
                    return 0;
                }
            }

            return -ENOENT;
        }
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

static void download_file(string id) {
    string home = string(getenv("HOME"));

    ifstream file;
    file.open(home + "/.ytfs/" + id + ".mp3", ios::binary);
    if(!file.is_open()) {
        printf("Starting thread to download : %s\n", id.c_str());

        string path = download_url + id + ".mp3";
        execute(("curl " + path + " > " + home + "/.ytfs/" + id + ".mp3").c_str());
    }
    else {
        file.close();
    }
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
                    thread t = thread(download_file, song["id"].get<string>());
                    t.detach();
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

    string home = string(getenv("HOME"));

    ifstream file;
    file.open(home + "/.ytfs/" + id + ".mp3", ios::binary);
    if(!file.is_open()) {
        string path = download_url + id + ".mp3";
        if(!execute(("curl " + path + " > " + home + "/.ytfs/" + id + ".mp3").c_str()))
            return -ENOENT;

        file.open(home + "/.ytfs/" + id + ".mp3", ios::binary);
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

static bool is_writing = false;

static int ytfs_write(const char* path, const char* buf, size_t size, off_t offset,
		      struct fuse_file_info *fi) {
    ofstream fstr;
    
    if (is_writing) {
        fstr.open("/tmp/tmp.mp3", ios::out | ios::binary | ios::app);
    } else {
        fstr.open("/tmp/tmp.mp3", ios::out | ios::binary | ios::trunc);
    }
    is_writing = true;
    fstr.write(buf, size);
    fstr.close();
    return size;
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
    if (is_writing) {
        is_writing = false;
        FileRef f("/tmp/tmp.mp3");

        struct stat stat_buf;
        int rc = stat("/tmp/tmp.mp3", &stat_buf);
        int size = (rc == 0 ? stat_buf.st_size : -1);

        string genre = f.tag()->genre().to8Bit();
        replace(genre.begin(), genre.end(), '/', '|');

        stringstream curl;
        curl << "curl --progress-bar --verbose ";
        curl << "-F \"file=@/tmp/tmp.mp3\" ";
        curl << "-F \"title=" << f.tag()->title() << "\" ";
        curl << "-F \"artist=" << f.tag()->artist() << "\" ";
        curl << "-F \"album=" << f.tag()->album() << "\" ";
        curl << "-F \"genre=" << genre << "\" ";
        curl << "-F \"decade=" << f.tag()->year() << "\" ";
        curl << "-F \"size=" << size << "\" ";
        curl << song_url;
        executeAndReloadMetadata(curl.str()); 
    }
    return 0;
}

static int ytfs_unlink(const char* path) {
    
    if (endsWith(path, ".mp3") != 0)
        return -ENOENT;
        
    string id = getSongId(path);
    if (id.empty() || !executeAndReloadMetadata("curl -X DELETE " + song_url + "/" + id)) {
        return -ENOENT;
    }
    
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
    shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe) 
        return false;
    char buffer[128];
    string result = "";
    while (!feof(pipe.get())) {
        if (fgets(buffer, 128, pipe.get()) != NULL)
            result += buffer;
    }

    return true;
}

bool executeAndReloadMetadata(string cmd) {
    cmd += " > metadata.json";
    const char* outCmd = cmd.c_str();

    if (!execute(outCmd))
        return false;

    fstream fstr;
    fstr.open("metadata.json", fstream::in);
    MetadataJson = json::parse(fstr);
    fstr.close();

    return true;
}

void initData(void) {
    executeAndReloadMetadata("curl " + metadata_url);
}

int main(int argc, char *argv[]) {
    initData();
    string home = string(getenv("HOME"));
    mkdir((home + "/.ytfs/").c_str(), 0777);
	return fuse_main(argc, argv, &ytfs_oper, nullptr);
}
