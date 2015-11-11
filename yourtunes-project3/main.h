/* Songs */
typedef struct song {
    char* name;
    char* filepath;
    int size;
} song;

typedef struct {
    song* songs;
    int size;
    int max;
} Song_array;

void initSongArray(Song_array* a, int initialSize) {
    a->songs = malloc(initialSize * sizeof(song));
    a->size = 0;
    a->max = initialSize;
}

void addSong(Song_array* a, song song) {
    if (a->size == a->max) {
        a->max *= 2;
        a->songs = realloc(a->songs, a->max * sizeof(song));
    }
    a->songs[a->size++] = song;
}

void freeSongArray(Song_array* a) {
    free(a->songs);
    a->songs = NULL;
    a->size = a->max = 0;
}

/* Albums */
typedef struct album {
    char* name;
    Song_array song_array;
} album;

typedef struct {
    album* albums;
    int size;
    int max;
} Album_array;

void initAlbumArray(Album_array* a, int initialSize) {
    a->albums = malloc(initialSize * sizeof(album));
    a->size = 0;
    a->max = initialSize;
}

void addAlbum(Album_array* a, album al) {
    if (a->size == a->max) {
        a->max *= 2;
        a->albums = realloc(a->albums, a->max * sizeof(album));
    }
    a->albums[a->size++] = al;
}

void freeAlbumArray(Album_array* a){
    free(a->albums);
    a->albums = NULL;
    a->size = a->max = 0;
}

/* Decades */
typedef struct decade {
    char* year;
    Song_array song_array;
} decade;

typedef struct {
    decade* decades;
    int size;
    int max;
} Decade_array;

void initDecadeArray(Decade_array* a, int initialSize) {
    a->decades = malloc(initialSize * sizeof(decade));
    a->size = 0;
    a->max = initialSize;
}

void addDecade(Decade_array* a, decade d) {
    if (a->size == a->max) {
        a->max *= 2;
        a->decades = realloc(a->decades, a->max * sizeof(decade));
    }
    a->decades[a->size++] = d;
}

void freeDecadeArray(Decade_array* a){
    free(a->decades);
    a->decades = NULL;
    a->size = a->max = 0;
}

