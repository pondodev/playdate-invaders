#include "dooter.h"

static const struct playdate_sys* sys = NULL;
static const struct playdate_sound* snd = NULL;
static const struct playdate_sound_sequence* seq = NULL;
static const struct playdate_sound_synth* synth = NULL;

SoundSequence* music = NULL;

void init_sound_engine(PlaydateAPI* pd) {
    sys = pd->system;
    snd = pd->sound;
    seq = pd->sound->sequence;
    synth = pd->sound->synth;

    music = seq->newSequence();
    if (! seq->loadMidiFile(music, "Sequences/megalovania.mid"))
        sys->error("failed to load sequence");

    seq->setLoops(music, 0, seq->getLength(music), 0);

    PDSynthInstrument* inst = snd->instrument->newInstrument();
    snd->instrument->setVolume(inst, 0.2, 0.2);
    snd->channel->addSource(snd->getDefaultChannel(), (SoundSource*)inst);

    int n = seq->getTrackCount(music);

    for (int i = 0; i < n; ++i) {
        SequenceTrack* track = pd->sound->sequence->getTrackAtIndex(music, i);
        snd->track->setInstrument(track, inst);

        for (int p = snd->track->getPolyphony(track); p > 0; --p) {
            PDSynth* s = synth->newSynth();
            synth->setWaveform(s, kWaveformSawtooth);
            synth->setAttackTime(s, 0);
            synth->setDecayTime(s, 0.1);
            synth->setSustainLevel(s, 0);
            synth->setReleaseTime(s, 0.1);
            snd->instrument->addVoice(inst, s, 0, 127, 0);
        }
    }
}

void play_music() {
    seq->play(music, NULL, NULL);
}

void pause_music() {
    seq->stop(music);
}
