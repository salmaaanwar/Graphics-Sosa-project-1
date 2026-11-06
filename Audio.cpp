#include "Audio.h"
#include "Game.h"
#define MINIAUDIO_IMPLEMENTATION
#include "external/miniaudio.h"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <memory>
#include <vector>

namespace {
constexpr std::size_t COUNT = static_cast<std::size_t>(SoundId::Count);
constexpr std::size_t MUSIC_COUNT = 5;
constexpr std::size_t MAX_VOICES = 24;
constexpr const char* files[COUNT] = {
    "main-menu.mp3", "intro.mp3", "gameplay.mp3", "victory.mp3", "defeat.mp3",
    "button.mp3", "select-card.mp3", "invalid-placement.mp3", "place-unit.mp3",
    "swatter-swing.mp3", "swatter-hit.mp3", "spray-fire.mp3", "spray-hit.mp3",
    "unit-damage.mp3", "sheet-hit.mp3", "unit-defeated.mp3", "insect-defeated.mp3",
    "jump.mp3", "duck.mp3", "pickup.mp3", "wave.mp3", "approaching.mp3",
    "food-reached.mp3", "pause.mp3", "resume.mp3"
};
struct Voice { ma_sound sound{}; bool gameplay = false; };
struct Audio {
    ma_engine engine{};
    ma_sound_group musicGroup{}, gameplayGroup{}, uiGroup{};
    std::array<ma_sound, COUNT> sounds{};
    std::array<bool, COUNT> loaded{};
    std::vector<std::unique_ptr<Voice>> voices;
    std::array<float, COUNT> cooldown{};
    AudioDiagnostics diagnostics;
    bool recording = false, ready = false, offline = false;
};
std::unique_ptr<Audio> audio;
float volumes[] = {0.35f, 0.75f};
float unmuted[] = {0.35f, 0.75f};
std::size_t index(SoundId id) { return static_cast<std::size_t>(id); }
bool uiSound(SoundId id) {
    return id == SoundId::Button || id == SoundId::Pause || id == SoundId::Resume || id == SoundId::FoodReached;
}
void limitMix(void*, float* samples, ma_uint64 frames) {
    // Stereo output: leave normal levels untouched and smoothly compress peaks.
    // This runs on the audio thread and must not allocate or access game state.
    for (ma_uint64 i = 0; i < frames*2; ++i) {
        const float value = samples[i], level = std::fabs(value);
        if (!std::isfinite(value)) samples[i] = 0;
        else if (level > 0.8f)
            samples[i] = std::copysign(0.8f+0.2f*std::tanh((level-0.8f)/0.2f),value);
    }
}
void applyVolumes() {
    if (!audio || !audio->ready || audio->recording) return;
    ma_sound_group_set_volume(&audio->musicGroup, volumes[0]);
    ma_sound_group_set_volume(&audio->gameplayGroup, volumes[1]);
    ma_sound_group_set_volume(&audio->uiGroup, volumes[1]);
}
void startMusic(SoundId id) {
    if (!audio || !audio->ready) return;
    auto& a = *audio;
    auto previous = a.diagnostics.music;
    if (!a.recording && index(previous) < MUSIC_COUNT && a.loaded[index(previous)]) {
        ma_sound_stop(&a.sounds[index(previous)]);
        ma_sound_seek_to_pcm_frame(&a.sounds[index(previous)], 0);
    }
    a.diagnostics.music = id;
    a.diagnostics.paused = false;
    if (index(id) >= MUSIC_COUNT) return;
    ++a.diagnostics.musicStarts;
    if (!a.recording && a.loaded[index(id)]) {
        ma_sound_seek_to_pcm_frame(&a.sounds[index(id)], 0);
        ma_sound_start(&a.sounds[index(id)]);
    }
}
}

const char* audioFilename(SoundId id) { return index(id) < COUNT ? files[index(id)] : ""; }
bool initializeAudio(const std::string& executableDirectory, AudioMode mode) {
    shutdownAudio();
    audio = std::make_unique<Audio>();
    auto& a = *audio;
    a.recording = mode == AudioMode::Recording;
    a.offline = mode == AudioMode::Offline;
    if (a.recording) { a.ready = true; return true; }
    auto config = ma_engine_config_init();
    config.channels = 2;
    config.onProcess = limitMix;
    if (a.offline) { config.noDevice = MA_TRUE; config.channels = 2; config.sampleRate = 48000; }
    if (ma_engine_init(&config, &a.engine) != MA_SUCCESS) {
        std::cerr << "Audio: no playback device; continuing silently.\n";
        return false;
    }
    ma_sound_group* groups[] = {&a.musicGroup, &a.gameplayGroup, &a.uiGroup};
    int initialized = 0;
    for (auto* group : groups) {
        if (ma_sound_group_init(&a.engine, 0, nullptr, group) != MA_SUCCESS) {
            while (initialized) ma_sound_group_uninit(groups[--initialized]);
            ma_engine_uninit(&a.engine);
            std::cerr << "Audio: mixer initialization failed; continuing silently.\n";
            return false;
        }
        ++initialized;
    }
    a.ready = true;
    for (std::size_t i = 0; i < COUNT; ++i) {
        auto path = std::filesystem::path(executableDirectory) / "assets" / "audio" / files[i];
        if (!std::filesystem::exists(path)) path = std::filesystem::path(GAME_ASSET_DIR) / "audio" / files[i];
        const bool music = i < MUSIC_COUNT;
        const bool loop = i == index(SoundId::MenuMusic) || i == index(SoundId::GameplayMusic);
        ma_uint32 flags = MA_SOUND_FLAG_NO_SPATIALIZATION | (music ? MA_SOUND_FLAG_STREAM : MA_SOUND_FLAG_DECODE);
        if (loop) flags |= MA_SOUND_FLAG_LOOPING;
        auto* group = music ? &a.musicGroup : (uiSound(static_cast<SoundId>(i)) ? &a.uiGroup : &a.gameplayGroup);
        a.loaded[i] = ma_sound_init_from_file(&a.engine, path.string().c_str(), flags, group, nullptr, &a.sounds[i]) == MA_SUCCESS;
        if (a.loaded[i]) ++a.diagnostics.loadedSounds;
        if (!a.loaded[i]) std::cerr << "Audio: could not load " << path << "; skipping this sound.\n";
    }
    applyVolumes();
    return true;
}
void shutdownAudio() {
    if (!audio) return;
    if (audio->ready && !audio->recording) {
        for (auto& v : audio->voices) ma_sound_uninit(&v->sound);
        for (std::size_t i = 0; i < COUNT; ++i) if (audio->loaded[i]) ma_sound_uninit(&audio->sounds[i]);
        ma_sound_group_uninit(&audio->uiGroup);
        ma_sound_group_uninit(&audio->gameplayGroup);
        ma_sound_group_uninit(&audio->musicGroup);
        ma_engine_uninit(&audio->engine);
    }
    audio.reset();
}
void updateAudio(float seconds) {
    if (!audio || !audio->ready || !std::isfinite(seconds) || seconds < 0) return;
    for (auto& c : audio->cooldown) c = std::max(0.0f, c - seconds);
    if (audio->recording) return;
    auto& voices = audio->voices;
    for (auto it = voices.begin(); it != voices.end();) {
        if (ma_sound_at_end(&(*it)->sound)) {
            ma_sound_uninit(&(*it)->sound);
            it = voices.erase(it);
        } else ++it;
    }
}
void playSound(SoundId id) {
    if (!audio || !audio->ready || index(id) < MUSIC_COUNT || index(id) >= COUNT) return;
    auto& a = *audio;
    const auto i = index(id);
    if (a.cooldown[i] > 0 || (a.diagnostics.paused && !uiSound(id))) return;
    if (!a.recording) {
        if (!a.loaded[i]) return;
        // Reserve four voices for UI/result cues even during heavy combat.
        if (!uiSound(id) && a.voices.size() >= MAX_VOICES-4) return;
        if (a.voices.size() >= MAX_VOICES) {
            auto victim = std::find_if(a.voices.begin(), a.voices.end(), [](const auto& v) { return v->gameplay; });
            if (victim == a.voices.end()) victim = a.voices.begin();
            ma_sound_uninit(&(*victim)->sound);
            a.voices.erase(victim);
        }
        auto v = std::make_unique<Voice>();
        v->gameplay = !uiSound(id);
        auto* group = v->gameplay ? &a.gameplayGroup : &a.uiGroup;
        if (ma_sound_init_copy(&a.engine, &a.sounds[i], MA_SOUND_FLAG_NO_SPATIALIZATION, group, &v->sound) != MA_SUCCESS) return;
        ma_sound_start(&v->sound);
        a.voices.push_back(std::move(v));
    }
    a.cooldown[i] = id == SoundId::Approaching ? 0.75f : 0.08f;
    ++a.diagnostics.played[i];
}
void clearGameplayAudio() {
    if (!audio || !audio->ready) return;
    auto& a = *audio;
    if (!a.recording) {
        for (auto it = a.voices.begin(); it != a.voices.end();) {
            if ((*it)->gameplay) {
                ma_sound_uninit(&(*it)->sound);
                it = a.voices.erase(it);
            } else ++it;
        }
        ma_sound_group_start(&a.gameplayGroup);
    }
    a.cooldown.fill(0);
    a.diagnostics.paused = false;
}
void audioGameStateChanged(int previous, int next) {
    if (!audio || !audio->ready || previous == next) return;
    auto& a = *audio;
    if (next == STATE_PAUSED && previous == STATE_PLAYING) {
        a.diagnostics.paused = true;
        if (!a.recording) {
            if (a.loaded[index(SoundId::GameplayMusic)]) ma_sound_stop(&a.sounds[index(SoundId::GameplayMusic)]);
            ma_sound_group_stop(&a.gameplayGroup);
        }
        playSound(SoundId::Pause);
    } else if (next == STATE_PLAYING && previous == STATE_PAUSED) {
        a.diagnostics.paused = false;
        if (!a.recording) {
            if (a.loaded[index(SoundId::GameplayMusic)]) ma_sound_start(&a.sounds[index(SoundId::GameplayMusic)]);
            ma_sound_group_start(&a.gameplayGroup);
        }
        playSound(SoundId::Resume);
    } else {
        clearGameplayAudio();
        SoundId music = SoundId::Count;
        switch (next) {
        case STATE_MAIN_MENU: music = SoundId::MenuMusic; break;
        case STATE_CUTSCENE: music = SoundId::IntroMusic; break;
        case STATE_PLAYING: music = SoundId::GameplayMusic; break;
        case STATE_WON: music = SoundId::VictoryMusic; break;
        case STATE_LOST: music = SoundId::DefeatMusic; break;
        }
        startMusic(music);
    }
}
void setAudioVolume(AudioBus bus, float value) {
    if (!std::isfinite(value)) return;
    const auto i = static_cast<int>(bus);
    volumes[i] = std::clamp(value, 0.0f, 1.0f);
    if (volumes[i] > 0) unmuted[i] = volumes[i];
    applyVolumes();
}
float getAudioVolume(AudioBus bus) { return volumes[static_cast<int>(bus)]; }
void toggleAudioMute(AudioBus bus) { setAudioVolume(bus, getAudioVolume(bus) > 0 ? 0 : unmuted[static_cast<int>(bus)]); }
AudioDiagnostics getAudioDiagnostics() {
    if (!audio) return {};
    auto result = audio->diagnostics;
    result.activeVoices = audio->voices.size();
    if (audio->ready && !audio->recording && index(result.music) < MUSIC_COUNT && audio->loaded[index(result.music)]) {
        ma_uint64 cursor = 0;
        ma_sound_get_cursor_in_pcm_frames(&audio->sounds[index(result.music)], &cursor);
        result.musicFrame = cursor;
    }
    return result;
}
bool renderOfflineAudio(float* output, std::size_t frames) {
    return audio && audio->ready && audio->offline && output &&
        ma_engine_read_pcm_frames(&audio->engine, output, frames, nullptr) == MA_SUCCESS;
}
bool validateAudioAssets(const std::string& directory) {
    bool valid = true;
    for (auto* file : files) {
        ma_decoder decoder{};
        const auto path = (std::filesystem::path(directory) / file).string();
        auto config = ma_decoder_config_init(ma_format_f32, 0, 0);
        if (ma_decoder_init_file(path.c_str(), &config, &decoder) != MA_SUCCESS) {
            std::cerr << "Audio validation: cannot decode " << path << '\n'; valid = false; continue;
        }
        std::vector<float> buffer(4096 * decoder.outputChannels);
        ma_uint64 total = 0, read = 0;
        ma_result result;
        do {
            // Decode as floats regardless of the source's native sample format.
            result = ma_decoder_read_pcm_frames(&decoder, buffer.data(), 4096, &read);
            total += read;
        } while (result == MA_SUCCESS && read > 0);
        if (!total || (result != MA_SUCCESS && result != MA_AT_END)) {
            std::cerr << "Audio validation: truncated/empty " << path << '\n'; valid = false;
        }
        ma_decoder_uninit(&decoder);
    }
    return valid;
}
