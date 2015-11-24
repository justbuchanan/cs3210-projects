#pragma once

#include <cstdlib>


/* Songs */
typedef struct {
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
    a->songs = (song*)malloc(initialSize * sizeof(song));
    a->size = 0;
    a->max = initialSize;
}

void addSong(Song_array* a, song s) {
    if (a->size == a->max) {
        a->max *= 2;
        a->songs = (song*)realloc(a->songs, a->max * sizeof(song));
    }
    a->songs[a->size++] = s;
}

void freeSongArray(Song_array* a) {
    free(a->songs);
    a->songs = nullptr;
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
    a->albums = (album*)malloc(initialSize * sizeof(album));
    a->size = 0;
    a->max = initialSize;
}

void addAlbum(Album_array* a, album al) {
    if (a->size == a->max) {
        a->max *= 2;
        a->albums = (album*)realloc(a->albums, a->max * sizeof(album));
    }
    a->albums[a->size++] = al;
}

void freeAlbumArray(Album_array* a){
    free(a->albums);
    a->albums = nullptr;
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
    a->decades = (decade*)malloc(initialSize * sizeof(decade));
    a->size = 0;
    a->max = initialSize;
}

void addDecade(Decade_array* a, decade d) {
    if (a->size == a->max) {
        a->max *= 2;
        a->decades = (decade*)realloc(a->decades, a->max * sizeof(decade));
    }
    a->decades[a->size++] = d;
}

void freeDecadeArray(Decade_array* a){
    free(a->decades);
    a->decades = nullptr;
    a->size = a->max = 0;
}

