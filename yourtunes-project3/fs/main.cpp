#define FUSE_USE_VERSION 26

#include "main.hpp"

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

static const char *albums_path = "/Albums";
static const char *decades_path = "/Decades";
static const char *delim = "/";

static Album_array album_array;
static Decade_array decade_array;

static int startsWith(const char* str, const char* pre) {
    int lenpre = strlen(pre),
        lenstr = strlen(str);
    return lenstr < lenpre ? 1 : strncmp(pre, str, lenpre);
}

static int endsWith(const char* str, const char* end) {
    int lenstr = strlen(str),
        lenend = strlen(end);
    return strcmp(str + (lenstr - lenend), end);
}

static int ytfs_getattr(const char *path, struct stat *stbuf) {
	memset(stbuf, 0, sizeof(struct stat));

    int i;
    
    // Check if root directory
    if (strcmp(path, delim) == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        return 0;
    }
    
    // TODO - free str before returning
    char* str = strdup(path);
    char* split = strtok(str, delim);
    // Check if directory is part of Albums directory
    if (strcmp(split, albums_path+1) == 0) {
        split = strtok(nullptr, delim);
        
        // Check if path is Albums directory
        if (split == nullptr) {
            stbuf->st_mode = S_IFDIR | 0755;
            stbuf->st_nlink = 2;
            return 0;
        }
        
        // Check if path is a valid album
        album* a = nullptr;
        for (i = 0; i < album_array.size; ++i) {
            if (strcmp(split, album_array.albums[i].name) == 0) {
                a = &album_array.albums[i];
                break;
            }
        }
        if (a == nullptr) return -ENOENT;
        
        split = strtok(nullptr, delim);
        // Check if path is a valid album directory
        if (split == nullptr) {
            stbuf->st_mode = S_IFDIR | 0755;
            stbuf->st_nlink = 2;
            return 0;
        }
        
        // Check if path is a valid song
        for (i = 0; i < a->song_array.size; ++i){
            if (strcmp(split, a->song_array.songs[i].name) == 0) {
                stbuf->st_mode = S_IFREG | 0666;
                stbuf->st_nlink = 1;
                stbuf->st_size = a->song_array.songs[i].size;
                return 0;
            }
        }
        return -ENOENT;
    }
    
    // Check if directory is part of Decades directory
    if (strcmp(split, decades_path+1) == 0) {
        split = strtok(nullptr, delim);
        
        // Check if path is Decades directory
        if (split == nullptr) {
            stbuf->st_mode = S_IFDIR | 0755;
            stbuf->st_nlink = 2;
            return 0;
        }
        
        // Check if path is a valid decade
        decade* d = nullptr;
        for (i = 0; i < decade_array.size; ++i) {
            if (strcmp(split, decade_array.decades[i].year) == 0) {
                d = &decade_array.decades[i];
                break;
            }
        }
        if (d == nullptr) return -ENOENT;
        
        split = strtok(nullptr, delim);
        // Check if path is a valid decade directory
        if (split == nullptr) {
            stbuf->st_mode = S_IFDIR | 0755;
            stbuf->st_nlink = 2;
            return 0;
        }
        
        // Check if path is a valid song
        for (i = 0; i < d->song_array.size; ++i){
            if (strcmp(split, d->song_array.songs[i].name) == 0) {
                stbuf->st_mode = S_IFREG | 0666;
                stbuf->st_nlink = 1;
                stbuf->st_size = d->song_array.songs[i].size;
                return 0;
            }
        }
        return -ENOENT;
    }

    // TODO - fix to make sure .mp3 file is in root and not in any other folder    
    if (endsWith(path, ".mp3") == 0) {
        stbuf->st_mode = S_IFREG | 0666;
        stbuf->st_nlink = 1;
	    stbuf->st_size = 0;
	    return 0;
    }

	return -ENOENT;
}

static int ytfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			 off_t offset, struct fuse_file_info *fi) {

    int i, j;

	if (strcmp(path, "/") == 0) {
        filler(buf, ".", nullptr, 0);
        filler(buf, "..", nullptr, 0);
        filler(buf, albums_path + 1, nullptr, 0);
    	filler(buf, decades_path + 1, nullptr, 0);
    } else if (strcmp(path, albums_path) == 0) {
        filler(buf, ".", nullptr, 0);
        filler(buf, "..", nullptr, 0);
        for (i = 0; i < album_array.size; ++i) {
            filler(buf, album_array.albums[i].name, nullptr, 0);
        }
    } else if (startsWith(path, albums_path) == 0) {
        filler(buf, ".", nullptr, 0);
        filler(buf, "..", nullptr, 0);
        int found = 0;
        for (i = 0; i < album_array.size; ++i) {
            album a = album_array.albums[i];
            char* fullPath = (char*)malloc(sizeof(char) * (strlen(albums_path) + 1 + strlen(a.name)));
            strcpy(fullPath, albums_path);
            strcat(fullPath, "/");
            strcat(fullPath, a.name);
            if (strcmp(path, fullPath) == 0) {
                for (j = 0; j < a.song_array.size; ++j){
                    filler(buf, a.song_array.songs[j].name, nullptr, 0);
                }
                found = 1;
            }
            free(fullPath);
            if (found == 1) break;
        }
        if (found == 0) {
            return -ENOENT;
        }
    } else if (strcmp(path, decades_path) == 0) {
        filler(buf, ".", nullptr, 0);
        filler(buf, "..", nullptr, 0);
        for (i = 0; i < decade_array.size; ++i) {
            filler(buf, decade_array.decades[i].year, nullptr, 0);
        }
    } else if (startsWith(path, decades_path) == 0) {
        filler(buf, ".", nullptr, 0);
        filler(buf, "..", nullptr, 0);
        int found = 0;
        for (i = 0; i < decade_array.size; ++i) {
            decade d = decade_array.decades[i];
            char* fullPath = (char*)malloc(sizeof(char) * (strlen(decades_path) + 1 + strlen(d.year)));
            strcpy(fullPath, decades_path);
            strcat(fullPath, "/");
            strcat(fullPath, d.year);
            if (strcmp(path, fullPath) == 0) {
                for (j = 0; j < d.song_array.size; ++j){
                    filler(buf, d.song_array.songs[j].name, nullptr, 0);
                }
                found = 1;
            }
            free(fullPath);
            if (found == 1) break;
        }
        if (found == 0) {
            return -ENOENT;
        }
    } else {
		return -ENOENT;
    }
	return 0;
}

static int ytfs_open(const char *path, struct fuse_file_info *fi) {

    const char* mp3 = ".mp3";
    printf("<><><><>OPENING FILE %s\n", path);
    if (endsWith(path, mp3) != 0) {      
        return -ENOENT;
    }
    
    // TODO - if opening a file on the root, then start a toggle. This means a write will happen.

	//if (strcmp(path, hello_path) != 0 && strcmp(path, albums_hello) != 0)
	//	return -ENOENT;

//	if ((fi->flags & 3) != O_RDONLY)
//		return -EACCES;

	return 0;
}

static int ytfs_read(const char *path, char *buf, size_t size, off_t offset,
		      struct fuse_file_info *fi) {
    printf("<><><><>READING %s\n", path);
    // TODO - Read from actual file and then write those bytes to buffer
//	size_t len;
    const char* mp3 = ".mp3";
    if (strcmp(path + (strlen(path) - strlen(mp3)), mp3) != 0) {      
        return -ENOENT;
    }
//	if(strcmp(path, hello_path) != 0 && strcmp(path, albums_hello) != 0)
//		return -ENOENT;

/*	len = strlen(hello_str);
	if (offset < len) {
		if (offset + size > len)
			size = len - offset;
		memcpy(buf, hello_str + offset, size);
	} else
		size = 0;*/

	return 0;
}

static int ytfs_write(const char* path, const char* buf, size_t size, off_t offset,
		      struct fuse_file_info *fi) {
    // TODO - check if toggle is on and the path is root, then start writing to mp3 storage (wherever the fuck that is)
    printf("Writing %s to %s of size %d\n", buf, path, (int)size);

    return size;
}

static int ytfs_create(const char* path, mode_t mode, struct fuse_file_info* fi) {
    printf("<><><><>CREATING %s\n", path);
    // TODO - check if creating a new MP3 file. If so, then set a toggle (that will be used by get attrs). If not, then throw error.
    return 0;
}

static int ytfs_chmod(const char* path, mode_t mode) {
    printf("CHMOD %s\n", path);
    return 0;
}

static int ytfs_chown(const char* path, uid_t uid, gid_t gid) {
    printf("CHOWN %s\n", path);
    return 0;
}

static int ytfs_utimens(const char* path, const struct timespec tv[2]) {
    printf("UTIMENS %s\n", path);
    return 0;
}

static int ytfs_truncate(const char* path, off_t offset) {
    printf("TRUNC %s\n", path);
    return 0;
}

static void ytfs_destroy(void* v) {
    int i;

    // Clean up song_array for each album
    for (i = 0; i < album_array.size; ++i) {
        freeSongArray(&album_array.albums[i].song_array);
    }
    freeAlbumArray(&album_array);
    
    // Clean up song_array for each decade
    for (i = 0; i < decade_array.size; ++i) {
        freeSongArray(&decade_array.decades[i].song_array);
    }
    freeDecadeArray(&decade_array);
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
    .destroy    = ytfs_destroy,
};

// Generates sample data for showing on the filesystem
void initData(void) {
    int i, j;

    initAlbumArray(&album_array, 10);
    for (i = 0; i < 5; ++i) {
        char* str = (char*)malloc(sizeof(char) * 10);
        sprintf(str, "Album #%d", i);
        album a;
        a.name = str;
        initSongArray(&a.song_array, 10);
        for (j = 0; j < 3; ++j) {
            char* name = (char*)malloc(sizeof(char) * 20);
            sprintf(name, "Al%d-Song #%d.mp3", i, j);
            song s;
            s.name = name;
            s.size = 0;
            printf("HI\n");
            addSong(&a.song_array, s);
            printf("HI2\n");
        }
        addAlbum(&album_array, a);
    }

    initDecadeArray(&decade_array, 10);
    for (i = 0; i < 5; ++i) {
        char* str = (char*)malloc(sizeof(char) * 10);
        sprintf(str, "200%d", i);
        decade d;
        d.year = str;
        initSongArray(&d.song_array, 10);
        for (j = 0; j < 3; ++j) {
            char* name = (char*)malloc(sizeof(char) * 20);
            sprintf(name, "200%d-Song #%d.mp3", i, j);
            song s;
            s.name = name;
            s.size = 0;
            addSong(&d.song_array, s);
        }
        addDecade(&decade_array, d);
    }
    
}

int main(int argc, char *argv[]) {
    initData();
	return fuse_main(argc, argv, &ytfs_oper, nullptr);
}
