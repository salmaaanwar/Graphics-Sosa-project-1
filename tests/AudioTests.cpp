#include "Audio.h"
#include "Game.h"
#include "UI.h"
#include "Environment.h"
#include "Player.h"
#include "Enemy.h"
#include "Deployment.h"
#include "PlayerCombat.h"
#include "Collectibles.h"
#include "WaveManager.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <vector>
#include <filesystem>
#include <fstream>

static void check(bool value, const char* message) {
    if (!value) { std::cerr << "FAILED: " << message << '\n'; std::exit(1); }
}
static unsigned count(SoundId id) { return getAudioDiagnostics().played[static_cast<std::size_t>(id)]; }
static void freshRecording() {
    initializeAudio({}, AudioMode::Recording);
    returnToMainMenu();
    restartGame();
    updateAudio(1);
}
static void testStates() {
    initializeAudio({}, AudioMode::Recording);
    returnToMainMenu();
    audioGameStateChanged(-1,STATE_MAIN_MENU);
    check(getAudioDiagnostics().music == SoundId::MenuMusic,"menu music");
    startNewGame();
    check(getAudioDiagnostics().music == SoundId::IntroMusic,"intro replaces menu");
    handleUIMouseClick(nullptr,0.80f,0.86f);
    check(getAudioDiagnostics().music == SoundId::GameplayMusic,"skip replaces intro");
    auto starts = getAudioDiagnostics().musicStarts;
    pauseGame();
    check(getAudioDiagnostics().paused && count(SoundId::Pause) == 1,"pause freezes gameplay and cues once");
    pauseGame();
    auto shots = count(SoundId::SprayFire);
    playSound(SoundId::SprayFire);
    check(count(SoundId::SprayFire) == shots,"combat suppressed while paused");
    playSound(SoundId::Button);
    check(count(SoundId::Button) > 0,"UI audible while paused");
    resumeGame();
    check(!getAudioDiagnostics().paused && count(SoundId::Resume) == 1,"resume cue");
    check(getAudioDiagnostics().musicStarts == starts,"resume retains current track");
    pauseGame(); restartGame();
    check(getAudioDiagnostics().musicStarts == starts+1 && !getAudioDiagnostics().paused,"restart from pause restarts track");
    restartGame();
    check(getAudioDiagnostics().musicStarts == starts+2,"restart while playing restarts track");
    finishGame(true);
    check(getAudioDiagnostics().music == SoundId::VictoryMusic,"victory music");
    restartGame(); finishGame(false);
    check(getAudioDiagnostics().music == SoundId::DefeatMusic,"defeat music");
    returnToMainMenu();
    check(getAudioDiagnostics().music == SoundId::MenuMusic,"return replaces result music");
}
static void testEvents() {
    freshRecording();
    selectPlayerCard(PLAYER_MELEE);
    check(count(SoundId::InvalidPlacement) == 1 && count(SoundId::PlaceUnit) == 0,"missing lane rejected");
    updateAudio(1);
    auto& env = getEnvironment();
    env.handleMouseButton(GLFW_MOUSE_BUTTON_LEFT,GLFW_PRESS,env.getLaneStartX()+0.1f,env.getLaneCenterY(0));
    env.handleMouseButton(GLFW_MOUSE_BUTTON_LEFT,GLFW_RELEASE,0,0);
    selectPlayerCard(PLAYER_MELEE);
    check(count(SoundId::SelectCard) == 1 && count(SoundId::PlaceUnit) == 1,"valid card and deployment");
    updateAudio(1); setCollectibles(0); selectPlayerCard(PLAYER_RANGED);
    check(count(SoundId::InvalidPlacement) == 2 && count(SoundId::PlaceUnit) == 1,"unaffordable/locked card rejected");
    handlePlayerKeyPress(GLFW_KEY_W,GLFW_PRESS);
    handlePlayerKeyPress(GLFW_KEY_W,GLFW_REPEAT);
    check(count(SoundId::Jump) == 1,"jump cue once for accepted pose change");
    handlePlayerKeyPress(GLFW_KEY_S,GLFW_PRESS);
    check(count(SoundId::Duck) == 1,"duck cue");
    damagePlayer(0,1);
    check(count(SoundId::UnitDamage) == 1,"damage cue");
    damagePlayer(0,100);
    check(count(SoundId::UnitDefeated) == 1,"unit death once");
    damagePlayer(0,100);
    check(count(SoundId::UnitDefeated) == 1,"invalid damage has no death cue");
    int sheet = spawnPlayer(PLAYER_DEFENSE,0);
    damagePlayer(sheet,1);
    check(count(SoundId::SheetHit) == 1,"sheet uses absorption cue");
    dropCollectible(getPlayers()[sheet].x,getPlayers()[sheet].y,10);
    updateCollectibles(); updateCollectibles();
    check(count(SoundId::Pickup) == 1,"pickup at actual collection once");
    updateAudio(1);
    auto enemy = spawnEnemy(ENEMY_EASY,0);
    spawnEnemy(ENEMY_EASY,1);
    check(count(SoundId::Approaching) == 1,"spawn cue rate limited");
    damageEnemy(enemy,100); damageEnemy(enemy,100);
    check(count(SoundId::InsectDefeated) == 1,"insect dies once");
    resetEnemies();
    auto id = spawnEnemy(ENEMY_EASY,2);
    (void)id;
    getEnemies()[0].x = env.getHomeBaseRightEdgeX();
    updateEnemies(0.01f);
    check(count(SoundId::FoodReached) == 1 && getGameState() == STATE_LOST,"food cue before defeat");
    freshRecording();
    updateWaves(0.01f); updateWaves(0.01f);
    check(count(SoundId::Wave) == 1,"first wave announced once");
    // A swatter attacks once; the impact is conditional on an actual target.
    resetWaves(); resetEnemies(); resetPlayers(); resetPlayerCombat();
    int melee = spawnPlayer(PLAYER_MELEE,0);
    enemy = spawnEnemy(ENEMY_EASY,0);
    getEnemies()[0].x = getPlayers()[melee].x+0.10f;
    updatePlayerCombat(0.001f);
    check(count(SoundId::SwatterSwing) == 1 && count(SoundId::SwatterHit) == 1,"swing and real hit");
    resetPlayers(); resetEnemies(); resetPlayerCombat(); updateAudio(1);
    int ranged = spawnPlayer(PLAYER_RANGED,0);
    spawnEnemy(ENEMY_EASY,0);
    getEnemies()[0].x = getPlayers()[ranged].x+0.10f;
    updatePlayerCombat(0.1f);
    check(count(SoundId::SprayFire) == 1 && count(SoundId::SprayHit) == 1,"spray fires and collides");
    // Cooldown expires without accumulating a queue of deferred effects.
    freshRecording(); playSound(SoundId::Approaching); playSound(SoundId::Approaching);
    check(count(SoundId::Approaching) == 1,"duplicate suppression");
    updateAudio(0.8f); playSound(SoundId::Approaching);
    check(count(SoundId::Approaching) == 2,"cooldown expires");
}
static void testControls() {
    freshRecording(); returnToMainMenu();
    setAudioVolume(AudioBus::Music,0.35f); setAudioVolume(AudioBus::Effects,0.75f);
    check(handleAudioControlsMouse(0.11f,-0.65f,GLFW_PRESS),"slider click handled");
    updateAudioControlsMouse(0.37f,-0.65f,true);
    check(getAudioVolume(AudioBus::Music) == 1,"drag changes volume");
    handleAudioControlsMouse(0.37f,-0.65f,GLFW_RELEASE);
    handleAudioControlsKey(GLFW_KEY_LEFT,GLFW_PRESS);
    check(std::fabs(getAudioVolume(AudioBus::Music)-0.95f) < 0.001f,"focused slider keyboard adjustment");
    handleAudioControlsMouse(0.665f,-0.81f,GLFW_PRESS);
    check(getAudioVolume(AudioBus::Effects) == 0,"separate effects mute");
    handleAudioControlsKey(GLFW_KEY_SPACE,GLFW_PRESS);
    check(getAudioVolume(AudioBus::Effects) == 0.75f,"unmute restores previous gain");
    restartGame(); pauseGame();
    check(getAudioVolume(AudioBus::Effects) == 0.75f && getAudioVolume(AudioBus::Music) > 0.94f,"settings survive state transitions");
    setAudioVolume(AudioBus::Music,std::numeric_limits<float>::quiet_NaN());
    check(getAudioVolume(AudioBus::Music) > 0.94f,"NaN ignored");
    setAudioVolume(AudioBus::Music,2); check(getAudioVolume(AudioBus::Music) == 1,"volume clamped");
    setAudioVolume(AudioBus::Music,-1); check(getAudioVolume(AudioBus::Music) == 0,"zero mutes");
}
static void testOfflinePlayback(const std::string& executableDirectory) {
    check(initializeAudio(executableDirectory,AudioMode::Offline),"offline engine initializes without device");
    check(getAudioDiagnostics().loadedSounds == 25,"all assets loaded into playback manager");
    setAudioVolume(AudioBus::Music,0.35f); setAudioVolume(AudioBus::Effects,0.75f);
    restartGame();
    std::vector<float> pcm(48000*2);
    check(renderOfflineAudio(pcm.data(),48000),"offline music renders");
    auto before = getAudioDiagnostics().musicFrame;
    check(before > 0,"music cursor advances");
    bool nonzero = false;
    for (float sample : pcm) { check(std::isfinite(sample),"finite PCM"); nonzero |= std::fabs(sample) > 0.00001f; }
    check(nonzero,"decoded music produces audible PCM");
    pauseGame(); renderOfflineAudio(pcm.data(),48000);
    check(getAudioDiagnostics().musicFrame == before,"pause preserves exact playback cursor");
    resumeGame(); renderOfflineAudio(pcm.data(),48000);
    check(getAudioDiagnostics().musicFrame > before,"resume continues playback");
    pauseGame(); restartGame();
    check(getAudioDiagnostics().musicFrame == 0,"restart seeks to beginning");
    // Busy combat cannot starve pause/menu feedback or grow the voice pool.
    for (int batch=0;batch<4;++batch) {
        updateAudio(1);
        for (int id=static_cast<int>(SoundId::SelectCard);id<=static_cast<int>(SoundId::Approaching);++id)
            playSound(static_cast<SoundId>(id));
    }
    check(getAudioDiagnostics().activeVoices <= 20,"combat voice budget bounded");
    check(renderOfflineAudio(pcm.data(),48000),"busy combat mix renders");
    float peak = 0;
    for (float sample : pcm) { check(std::isfinite(sample),"busy mix remains finite"); peak = std::max(peak,std::fabs(sample)); }
    std::cout << "Busy combat mix peak: " << peak << '\n';
    check(peak <= 1.0f,"soft limiter prevents output overload");
    auto pauseCount = count(SoundId::Pause);
    pauseGame();
    check(count(SoundId::Pause)==pauseCount+1 && getAudioDiagnostics().activeVoices<=24,"pause feedback has reserved capacity");
    returnToMainMenu();
    check(getAudioDiagnostics().activeVoices <= 2,"reset removes combat voices");
    setAudioVolume(AudioBus::Music,0); setAudioVolume(AudioBus::Effects,0);
    renderOfflineAudio(pcm.data(),48000); // Flush any volume interpolation.
    renderOfflineAudio(pcm.data(),48000);
    for (float sample : pcm) check(std::fabs(sample) < 0.000001f,"muted output is silent");
    shutdownAudio();
    playSound(SoundId::Button); pauseGame(); resumeGame(); // Safe silent fallback before/after init.
}
static void testBadAsset() {
    // A corrupt packaged file overrides the valid source copy; only this sound
    // should fail, and all other audio and gameplay must remain usable.
    auto root = std::filesystem::current_path()/"audio-corrupt-fixture";
    auto directory = root/"assets"/"audio";
    std::filesystem::create_directories(directory);
    auto file = directory/"button.mp3";
    { std::ofstream output(file,std::ios::binary); output << "not an MP3"; }
    check(initializeAudio(root.string(),AudioMode::Offline),"bad file doesn't prevent engine startup");
    check(getAudioDiagnostics().loadedSounds == 24,"corrupt asset skipped individually");
    playSound(SoundId::Button);
    check(count(SoundId::Button)==0,"missing sound is a safe no-op");
    restartGame();
    std::vector<float> pcm(96000); check(renderOfflineAudio(pcm.data(),48000),"remaining music still renders");
    shutdownAudio();
    std::filesystem::remove(file);
    std::filesystem::remove(directory);
    std::filesystem::remove(root/"assets");
    std::filesystem::remove(root);
}
int main(int argc, char** argv) {
    if (argc == 3 && std::string(argv[1]) == "--validate") return validateAudioAssets(argv[2]) ? 0 : 1;
    if (argc == 2) {
        for (int id=0;id<static_cast<int>(SoundId::Count);++id)
            check(std::filesystem::exists(std::filesystem::path(argv[1])/"assets"/"audio"/audioFilename(static_cast<SoundId>(id))),"packaged audio exists beside executable");
        testOfflinePlayback(argv[1]);
        return 0;
    }
    testStates(); testEvents(); testControls();
    testOfflinePlayback(argc > 1 ? argv[1] : "");
    testBadAsset();
    shutdownAudio();
    std::cout << "Audio state, events, controls, decoding and offline playback checks passed.\n";
}
