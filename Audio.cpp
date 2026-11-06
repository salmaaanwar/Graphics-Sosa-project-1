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
constexpr std::size_t SOUND_COUNT = static_cast<std::size_t>(SoundId::Count);
constexpr std::size_t MUSIC_COUNT = 5;
constexpr std::size_t MAX_VOICES = 24;
constexpr std::size_t UI_VOICE_RESERVE = 4;
constexpr const char* audioFiles[SOUND_COUNT] = {
    "main-menu.mp3", "intro.mp3", "gameplay.mp3", "victory.mp3", "defeat.mp3",
    "button.mp3", "select-card.mp3", "invalid-placement.mp3", "place-unit.mp3",
    "swatter-swing.mp3", "swatter-hit.mp3", "spray-fire.mp3", "spray-hit.mp3",
    "unit-damage.mp3", "sheet-hit.mp3", "unit-defeated.mp3", "insect-defeated.mp3",
    "jump.mp3", "duck.mp3", "pickup.mp3", "wave.mp3", "approaching.mp3",
    "food-reached.mp3", "pause.mp3", "resume.mp3"
};
struct Voice {
    ma_sound sound{};
    bool gameplay = false;
};
struct Audio {
    ma_engine engine{};
    ma_sound_group musicGroup{}, gameplayGroup{}, uiGroup{};
    std::array<ma_sound, SOUND_COUNT> sounds{};
    std::array<bool, SOUND_COUNT> loaded{};
    std::vector<std::unique_ptr<Voice>> voices;
    std::array<float, SOUND_COUNT> cooldown{};
    AudioDiagnostics diagnostics;
    bool recording = false;
    bool ready = false;
    bool offline = false;
};
std::unique_ptr<Audio> audio;
float volumes[] = {0.35f, 0.75f};
float unmutedVolumes[] = {0.35f, 0.75f};
std::size_t soundIndex(SoundId id) { return static_cast<std::size_t>(id); }
bool isUISound(SoundId id) {
    // FoodReached must remain audible when gameplay stops at defeat.
    return id == SoundId::Button || id == SoundId::Pause || id == SoundId::Resume || id == SoundId::FoodReached;
}
void limitMix(void*, float* samples, ma_uint64 frames) {
    // Stereo output: leave normal levels untouched and smoothly compress peaks.
    // This runs on the audio thread and must not allocate or access game state.
    for (ma_uint64 i = 0; i < frames * 2; ++i) {
        const float value = samples[i];
        const float level = std::fabs(value);
        if (!std::isfinite(value)) {
            samples[i] = 0;
        } else if (level > 0.8f) {
            samples[i] = std::copysign(0.8f + 0.2f * std::tanh((level - 0.8f) / 0.2f), value);
        }
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
    auto& state = *audio;
    auto previous = state.diagnostics.music;
    if (!state.recording && soundIndex(previous) < MUSIC_COUNT && state.loaded[soundIndex(previous)]) {
        ma_sound_stop(&state.sounds[soundIndex(previous)]);
        ma_sound_seek_to_pcm_frame(&state.sounds[soundIndex(previous)], 0);
    }
    state.diagnostics.music = id;
    state.diagnostics.paused = false;
    if (soundIndex(id) >= MUSIC_COUNT) return;
    ++state.diagnostics.musicStarts;
    if (!state.recording && state.loaded[soundIndex(id)]) {
        ma_sound_seek_to_pcm_frame(&state.sounds[soundIndex(id)], 0);
        ma_sound_start(&state.sounds[soundIndex(id)]);
    }
}
}

const char* audioFilename(SoundId id) { return soundIndex(id) < SOUND_COUNT ? audioFiles[soundIndex(id)] : ""; }
bool initializeAudio(const std::string& executableDirectory, AudioMode mode) {
    shutdownAudio();
    audio = std::make_unique<Audio>();
    auto& state = *audio;
    state.recording = mode == AudioMode::Recording;
    state.offline = mode == AudioMode::Offline;
    if (state.recording) {
        state.ready = true;
        return true;
    }
    auto config = ma_engine_config_init();
    config.channels = 2;
    config.onProcess = limitMix;
    if (state.offline) {
        config.noDevice = MA_TRUE;
        config.channels = 2;
        config.sampleRate = 48000;
    }
    if (ma_engine_init(&config, &state.engine) != MA_SUCCESS) {
        std::cerr << "Audio: no playback device; continuing silently.\n";
        return false;
    }
    ma_sound_group* groups[] = {&state.musicGroup, &state.gameplayGroup, &state.uiGroup};
    int initialized = 0;
    for (auto* group : groups) {
        if (ma_sound_group_init(&state.engine, 0, nullptr, group) != MA_SUCCESS) {
            while (initialized) {
                ma_sound_group_uninit(groups[--initialized]);
            }
            ma_engine_uninit(&state.engine);
            std::cerr << "Audio: mixer initialization failed; continuing silently.\n";
            return false;
        }
        ++initialized;
    }
    state.ready = true;
    for (std::size_t i = 0; i < SOUND_COUNT; ++i) {
        auto path = std::filesystem::path(executableDirectory) / "assets" / "audio" / audioFiles[i];
        if (!std::filesystem::exists(path)) {
            path = std::filesystem::path(GAME_ASSET_DIR) / "audio" / audioFiles[i];
        }
        const bool music = i < MUSIC_COUNT;
        const bool loop = i == soundIndex(SoundId::MenuMusic) || i == soundIndex(SoundId::GameplayMusic);
        ma_uint32 flags = MA_SOUND_FLAG_NO_SPATIALIZATION | (music ? MA_SOUND_FLAG_STREAM : MA_SOUND_FLAG_DECODE);
        if (loop) flags |= MA_SOUND_FLAG_LOOPING;
        auto* group = music ? &state.musicGroup : (isUISound(static_cast<SoundId>(i)) ? &state.uiGroup : &state.gameplayGroup);
        state.loaded[i] = ma_sound_init_from_file(&state.engine, path.string().c_str(), flags, group, nullptr, &state.sounds[i]) == MA_SUCCESS;
        if (state.loaded[i]) {
            ++state.diagnostics.loadedSounds;
        } else {
            std::cerr << "Audio: could not load " << path << "; skipping this sound.\n";
        }
    }
    applyVolumes();
    return true;
}
void shutdownAudio() {
    if (!audio) return;
    if (audio->ready && !audio->recording) {
        for (auto& voice : audio->voices) {
            ma_sound_uninit(&voice->sound);
        }
        for (std::size_t i = 0; i < SOUND_COUNT; ++i) {
            if (audio->loaded[i]) ma_sound_uninit(&audio->sounds[i]);
        }
        ma_sound_group_uninit(&audio->uiGroup);
        ma_sound_group_uninit(&audio->gameplayGroup);
        ma_sound_group_uninit(&audio->musicGroup);
        ma_engine_uninit(&audio->engine);
    }
    audio.reset();
}
void updateAudio(float seconds) {
    if (!audio || !audio->ready || !std::isfinite(seconds) || seconds < 0) return;
    for (auto& remaining : audio->cooldown) {
        remaining = std::max(0.0f, remaining - seconds);
    }
    if (audio->recording) return;
    auto& voices = audio->voices;
    for (auto it = voices.begin(); it != voices.end();) {
        if (ma_sound_at_end(&(*it)->sound)) {
            ma_sound_uninit(&(*it)->sound);
            it = voices.erase(it);
        } else {
            ++it;
        }
    }
}
void playSound(SoundId id) {
    if (!audio || !audio->ready || soundIndex(id) < MUSIC_COUNT || soundIndex(id) >= SOUND_COUNT) return;
    auto& state = *audio;
    const auto i = soundIndex(id);
    if (state.cooldown[i] > 0 || (state.diagnostics.paused && !isUISound(id))) return;
    if (!state.recording) {
        if (!state.loaded[i]) return;
        if (!isUISound(id) && state.voices.size() >= MAX_VOICES - UI_VOICE_RESERVE) return;
        if (state.voices.size() >= MAX_VOICES) {
            auto victim = std::find_if(state.voices.begin(), state.voices.end(), [](const auto& voice) {
                return voice->gameplay;
            });
            if (victim == state.voices.end()) victim = state.voices.begin();
            ma_sound_uninit(&(*victim)->sound);
            state.voices.erase(victim);
        }
        auto voice = std::make_unique<Voice>();
        voice->gameplay = !isUISound(id);
        auto* group = voice->gameplay ? &state.gameplayGroup : &state.uiGroup;
        if (ma_sound_init_copy(&state.engine, &state.sounds[i], MA_SOUND_FLAG_NO_SPATIALIZATION,
                               group, &voice->sound) != MA_SUCCESS) return;
        ma_sound_start(&voice->sound);
        state.voices.push_back(std::move(voice));
    }
    state.cooldown[i] = id == SoundId::Approaching ? 0.75f : 0.08f;
    ++state.diagnostics.played[i];
}
void clearGameplayAudio() {
    if (!audio || !audio->ready) return;
    auto& state = *audio;
    if (!state.recording) {
        for (auto it = state.voices.begin(); it != state.voices.end();) {
            if ((*it)->gameplay) {
                ma_sound_uninit(&(*it)->sound);
                it = state.voices.erase(it);
            } else {
                ++it;
            }
        }
        ma_sound_group_start(&state.gameplayGroup);
    }
    state.cooldown.fill(0);
    state.diagnostics.paused = false;
}
void audioGameStateChanged(int previous, int next) {
    if (!audio || !audio->ready || previous == next) return;
    auto& state = *audio;
    if (next == STATE_PAUSED && previous == STATE_PLAYING) {
        state.diagnostics.paused = true;
        if (!state.recording) {
            if (state.loaded[soundIndex(SoundId::GameplayMusic)]) ma_sound_stop(&state.sounds[soundIndex(SoundId::GameplayMusic)]);
            ma_sound_group_stop(&state.gameplayGroup);
        }
        playSound(SoundId::Pause);
    } else if (next == STATE_PLAYING && previous == STATE_PAUSED) {
        state.diagnostics.paused = false;
        if (!state.recording) {
            if (state.loaded[soundIndex(SoundId::GameplayMusic)]) ma_sound_start(&state.sounds[soundIndex(SoundId::GameplayMusic)]);
            ma_sound_group_start(&state.gameplayGroup);
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
    if (volumes[i] > 0) unmutedVolumes[i] = volumes[i];
    applyVolumes();
}
float getAudioVolume(AudioBus bus) { return volumes[static_cast<int>(bus)]; }
void toggleAudioMute(AudioBus bus) {
    setAudioVolume(bus, getAudioVolume(bus) > 0 ? 0 : unmutedVolumes[static_cast<int>(bus)]);
}
AudioDiagnostics getAudioDiagnostics() {
    if (!audio) return {};
    auto result = audio->diagnostics;
    result.activeVoices = audio->voices.size();
    if (audio->ready && !audio->recording && soundIndex(result.music) < MUSIC_COUNT && audio->loaded[soundIndex(result.music)]) {
        ma_uint64 cursor = 0;
        ma_sound_get_cursor_in_pcm_frames(&audio->sounds[soundIndex(result.music)], &cursor);
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
    for (auto* file : audioFiles) {
        ma_decoder decoder{};
        const auto path = (std::filesystem::path(directory) / file).string();
        auto config = ma_decoder_config_init(ma_format_f32, 0, 0);
        if (ma_decoder_init_file(path.c_str(), &config, &decoder) != MA_SUCCESS) {
            std::cerr << "Audio validation: cannot decode " << path << '\n';
            valid = false;
            continue;
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
            std::cerr << "Audio validation: truncated/empty " << path << '\n';
            valid = false;
        }
        ma_decoder_uninit(&decoder);
    }
    return valid;
}
