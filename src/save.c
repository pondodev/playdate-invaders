#include "save.h"

static const SaveData DEFAULT_SAVE = {
    .music_enabled = 1,
    .sound_effects_enabled = 1,
};

static const size_t MAX_SAVE_FILE_SIZE = 1024; // should be more than enough
static const size_t MAX_PROPERTY_COUNT = 10;
static const size_t MAX_PROPERTY_LENGTH = MAX_SAVE_FILE_SIZE / MAX_PROPERTY_COUNT;
static const size_t MAX_PROPERTY_KEY_LENGTH = (MAX_PROPERTY_LENGTH / 10) * 1;
static const size_t MAX_PROPERTY_VALUE_LENGTH = (MAX_PROPERTY_LENGTH / 10) * 9;
static const char* SAVE_FILE_NAME = "savefile.sav";

void save_data(PlaydateAPI* pd, SaveData data) {
    char buffer[MAX_SAVE_FILE_SIZE];
    size_t actual_size = 0;

    char music[MAX_PROPERTY_LENGTH];
    snprintf(music, MAX_PROPERTY_LENGTH, "music:%s\n", data.music_enabled ? "true" : "false");
    strncat(buffer, music, MAX_PROPERTY_LENGTH);
    actual_size += strnlen(music, MAX_PROPERTY_LENGTH);
    pd->system->logToConsole("music: %s", music);

    char fx[MAX_PROPERTY_LENGTH];
    snprintf(fx, MAX_PROPERTY_LENGTH, "fx:%s\n", data.sound_effects_enabled ? "true" : "false");
    strncat(buffer, fx, MAX_PROPERTY_LENGTH);
    actual_size += strnlen(fx, MAX_PROPERTY_LENGTH);
    pd->system->logToConsole("fx: %s", fx);

    SDFile* save_file = pd->file->open(SAVE_FILE_NAME, kFileWrite);
    if (! save_file)
        pd->system->error("failed to open file %s for writing", SAVE_FILE_NAME);

    pd->system->logToConsole("OUTPUT: %s", buffer);

    pd->file->write(save_file, buffer, actual_size);
    pd->file->close(save_file);
}

SaveData load_data(PlaydateAPI* pd) {
    SaveData to_return = DEFAULT_SAVE;

    SDFile* save_file = pd->file->open(SAVE_FILE_NAME, kFileReadData);
    if (! save_file) return to_return;

    char buffer[MAX_SAVE_FILE_SIZE];
    pd->file->read(save_file, buffer, MAX_SAVE_FILE_SIZE);

    char* prop = strtok(buffer, "\n");
    char* properties[MAX_PROPERTY_COUNT];
    size_t count = 0;

    while (prop) {
        properties[count] = malloc(sizeof(char) * MAX_PROPERTY_LENGTH);
        strncpy(properties[count], prop, MAX_PROPERTY_LENGTH);
        prop = strtok(NULL, "\n");

        ++count;
    }

    for (int i = 0; i < count; ++i) {
        char* key = strtok(properties[i], ":");
        char* value = strtok(NULL, ":");

        if (strncmp(key, "music", MAX_PROPERTY_KEY_LENGTH) == 0) { // music enabled
            to_return.music_enabled = strncmp(value, "true", MAX_PROPERTY_VALUE_LENGTH) == 0;
        } else if (strncmp(key, "fx", MAX_PROPERTY_KEY_LENGTH) == 0) { // sound effects enabled
            to_return.sound_effects_enabled = strncmp(value, "true", MAX_PROPERTY_VALUE_LENGTH) == 0;
        }

        free(properties[i]);
    }

    pd->system->logToConsole("music: %d", to_return.music_enabled);
    pd->system->logToConsole("fx: %d", to_return.sound_effects_enabled);

    pd->file->close(save_file);

    return to_return;
}
