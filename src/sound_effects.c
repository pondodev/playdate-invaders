#include "sound_effects.h"

static const struct playdate_sound* snd = NULL;

static SoundChannel* channel = NULL;
static BitCrusher* crusher = NULL;
static int muted = 0;

static PDSynth* projectile_sound = NULL;
static PDSynthLFO* projectile_lfo = NULL;

static PDSynth* reload_sound = NULL;

static const float CHANNEL_VOLUME = 1.f;

void init_sound_effects(PlaydateAPI* pd) {
    snd = pd->sound;

    // channel setup
    channel = snd->channel->newChannel();
    crusher = snd->effect->bitcrusher->newBitCrusher();

    snd->effect->bitcrusher->setUndersampling(crusher, 0.75f);
    snd->channel->addEffect(channel, (SoundEffect*)crusher);
    snd->channel->setVolume(channel, CHANNEL_VOLUME);

    snd->addChannel(channel);

    // projectile sound
    projectile_sound = snd->synth->newSynth();
    projectile_lfo = snd->lfo->newLFO(kLFOTypeSawtoothDown);

    snd->synth->setWaveform(projectile_sound, kWaveformSawtooth);
    snd->synth->setAttackTime(projectile_sound, 0.f);
    snd->synth->setDecayTime(projectile_sound, 0.f);
    snd->synth->setSustainLevel(projectile_sound, 1.f);
    snd->synth->setReleaseTime(projectile_sound, 0.1f);
    snd->synth->setVolume(projectile_sound, 0.2f, 0.2f);

    snd->lfo->setRetrigger(projectile_lfo, 1);
    snd->lfo->setRate(projectile_lfo, 2.5f);
    snd->synth->setFrequencyModulator(projectile_sound, (PDSynthSignalValue*)projectile_lfo);

    snd->channel->addSource(channel, (SoundSource*)projectile_sound);

    // reload sound
    reload_sound = snd->synth->newSynth();

    snd->synth->setWaveform(reload_sound, kWaveformTriangle);
    snd->synth->setAttackTime(reload_sound, 0.f);
    snd->synth->setDecayTime(reload_sound, 0.f);
    snd->synth->setSustainLevel(reload_sound, 0.1f);
    snd->synth->setReleaseTime(reload_sound, 0.f);

    snd->channel->addSource(channel, (SoundSource*)reload_sound);
}

void free_sound_effects() {
    snd->synth->freeSynth(projectile_sound);
    snd->synth->freeSynth(reload_sound);
    snd->effect->bitcrusher->freeBitCrusher(crusher);
    snd->lfo->freeLFO(projectile_lfo);
    snd->channel->freeChannel(channel);
}

void mute_sound_effects() {
    muted = 1;
}

void unmute_sound_effects() {
    muted = 0;
}

void play_projectile_sound() {
    if (muted) return;

    const float min = 500.f;
    const float max = 800.f;
    const float freq = lerp(min, max, frand(0.f, 1.f));
    snd->synth->playNote(projectile_sound, freq, 1.f, 0.1f, 0);
}

void play_reload_sound(float percent) {
    if (muted) return;

    const float min = 300.f;
    const float max = 1000.f;
    const float freq = remap(0.f, 100.f, min, max, percent);
    snd->synth->playNote(reload_sound, freq, 1.f, 0.1f, 0);
}
