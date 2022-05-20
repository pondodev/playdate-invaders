#include "dooter.h"

static const struct playdate_sys* sys = NULL;
static const struct playdate_sound* snd = NULL;
static const struct playdate_sound_sequence* seq = NULL;
static const struct playdate_sound_synth* synth = NULL;

SoundSequence* music = NULL;
PDSynthInstrument* instrument = NULL;
static const float MUSIC_VOLUME = 1.f;

// if we have more than 100 tracks or synths, we probably have other problems
size_t track_count = 0;
SequenceTrack* tracks[100];
size_t synth_count = 0;
PDSynth* synths[100];

void init_music(PlaydateAPI* pd) {
    sys = pd->system;
    snd = pd->sound;
    seq = pd->sound->sequence;
    synth = pd->sound->synth;

    music = seq->newSequence();
    if (! seq->loadMidiFile(music, "Sequences/megalovania.mid"))
        sys->error("failed to load sequence");

    seq->setLoops(music, 0, seq->getLength(music), 0);

    instrument = snd->instrument->newInstrument();
    snd->instrument->setVolume(instrument, MUSIC_VOLUME, MUSIC_VOLUME);
    snd->channel->addSource(snd->getDefaultChannel(), (SoundSource*)instrument);

    int n = seq->getTrackCount(music);

    for (; track_count < n; ++track_count) {
        SequenceTrack* track = pd->sound->sequence->getTrackAtIndex(music, track_count);
        snd->track->setInstrument(track, instrument);

        tracks[track_count] = track;

        int polyphony = snd->track->getPolyphony(track);
        for (int i = 0; i < polyphony; ++i) {
            PDSynth* s = synth->newSynth();
            synth->setWaveform(s, kWaveformSawtooth);
            synth->setAttackTime(s, 0.f);
            synth->setDecayTime(s, 0.1f);
            synth->setSustainLevel(s, 0.f);
            synth->setReleaseTime(s, 0.1f);
            snd->instrument->addVoice(instrument, s, 0, 127, 0);

            synths[synth_count] = s;
            ++synth_count;
        }
    }
}

void free_music() {
    seq->freeSequence(music);
    snd->instrument->freeInstrument(instrument);
    for (int i = 0; i < track_count; ++i) {
        snd->track->freeTrack(tracks[i]);
    }
    for (int i = 0; i < synth_count; ++i) {
        synth->freeSynth(synths[i]);
    }
}

void play_music() {
    seq->play(music, NULL, NULL);
}

void pause_music() {
    seq->stop(music);
}
