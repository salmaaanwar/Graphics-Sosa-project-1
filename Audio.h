#pragma once
#include <array>
#include <cstddef>
#include <string>

enum class SoundId {
    MenuMusic, IntroMusic, GameplayMusic, VictoryMusic, DefeatMusic,
    Button, SelectCard, InvalidPlacement, PlaceUnit, SwatterSwing, SwatterHit,
    SprayFire, SprayHit, UnitDamage, SheetHit, UnitDefeated, InsectDefeated,
    Jump, Duck, Pickup, Wave, Approaching, FoodReached, Pause, Resume, Count
};
enum class AudioBus { Music, Effects };
// Recording mode requires neither a device nor files; used by event tests.
enum class AudioMode { Playback, Recording, Offline };
bool initializeAudio(const std::string& executableDirectory = {}, AudioMode mode = AudioMode::Playback);
void shutdownAudio();
void updateAudio(float seconds);
void playSound(SoundId sound);
void audioGameStateChanged(int previous, int next);
void clearGameplayAudio();
void setAudioVolume(AudioBus bus, float volume);
float getAudioVolume(AudioBus bus);
void toggleAudioMute(AudioBus bus);
const char* audioFilename(SoundId sound);
// Validate full MP3 decoding, including music, without opening an audio device.
bool validateAudioAssets(const std::string& directory);
struct AudioDiagnostics {
    std::array<unsigned, static_cast<std::size_t>(SoundId::Count)> played{};
    SoundId music = SoundId::Count;
    bool paused = false;
    unsigned musicStarts = 0;
    unsigned long long musicFrame = 0;
    std::size_t activeVoices = 0;
    unsigned loadedSounds = 0;
};
AudioDiagnostics getAudioDiagnostics();
// Offline mode renders stereo float PCM without opening a device.
bool renderOfflineAudio(float* stereoOutput, std::size_t frames);
