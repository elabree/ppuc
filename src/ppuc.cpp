#include "PPUC.h"

#include <AL/al.h>
#include <AL/alc.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdlib.h>

#include <chrono>
#include <cstdio>
#include <cstring>
#include <thread>

#include "DMDUtil/Config.h"
#include "DMDUtil/ConsoleDMD.h"
#include "DMDUtil/DMDUtil.h"
#include "SDL3/SDL.h"
#include "SDL3_image/SDL_image.h"
#include "cargs.h"
#include "io-boards/Event.h"
#include "libpinmame.h"

#define MAX_AUDIO_BUFFERS 4
#define MAX_AUDIO_QUEUE_SIZE 10
#define MAX_EVENT_SEND_QUEUE_SIZE 10
#define MAX_EVENT_RECV_QUEUE_SIZE 10

ALuint _audioSource;
ALuint _audioBuffers[MAX_AUDIO_BUFFERS];
std::queue<void*> _audioQueue;
int _audioChannels;
int _audioSampleRate;

DMDUtil::DMD* pDmd;
PPUC* ppuc;

SDL_Window* pWindow;
SDL_Renderer* pRenderer;
SDL_Texture* pTransliteTexture;
SDL_Texture* pTransliteAttractTexture;

bool opt_debug = false;
bool opt_debug_switches = false;
bool opt_debug_coils = false;
bool opt_debug_lamps = false;
bool opt_no_serial = false;
bool opt_no_sound = false;
bool opt_serum = false;
bool opt_pup = false;
bool opt_console_display = false;
const char* opt_rom = NULL;
int game_state = 0;

static struct cag_option options[] = {
    {.identifier = 'c',
     .access_letters = "c",
     .access_name = "config",
     .value_name = "VALUE",
     .description = "Path to config file (required)"},
    {.identifier = 'r',
     .access_letters = "r",
     .access_name = "rom",
     .value_name = "VALUE",
     .description = "Path to ROM file (optional, overwrites setting in config file)"},
    {.identifier = 's',
     .access_letters = "s",
     .access_name = "serial",
     .value_name = "VALUE",
     .description = "Serial device (optional, overwrites setting in config file)"},
    {.identifier = 'n',
     .access_letters = "n",
     .access_name = "no-serial",
     .value_name = NULL,
     .description = "No serial communication to controllers (optional)"},
    {.identifier = 'M', .access_name = "no-sound", .value_name = NULL, .description = "Turn off sound (optional)"},
    {.identifier = 'u',
     .access_letters = "u",
     .access_name = "serum",
     .value_name = NULL,
     .description = "Enable Serum colorization (optional)"},
    {.identifier = 'T',
     .access_name = "serum-timeout",
     .value_name = "VALUE",
     .description = "Serum timeout in milliseconds to ignore unknown frames (optional)"},
    {.identifier = 'P',
     .access_name = "serum-skip-frames",
     .value_name = "VALUE",
     .description = "Serum ignore number of unknown frames (optional)"},
    {.identifier = 'p',
     .access_letters = "p",
     .access_name = "pup",
     .value_name = "VALUE",
     .description = "Enable PUP videos (optional)"},
    {.identifier = 'i',
     .access_letters = "i",
     .access_name = "console-display",
     .value_name = NULL,
     .description = "Enable console display (optional)"},
    {.identifier = 'm',
     .access_letters = "m",
     .access_name = "dump-display",
     .value_name = NULL,
     .description = "Enable display dump (optional)"},
    {.identifier = 'd',
     .access_letters = "d",
     .access_name = "debug",
     .value_name = NULL,
     .description = "Enable all debug output (optional)"},
    {.identifier = 'S',
     .access_name = "debug-switches",
     .value_name = NULL,
     .description = "Enable switches debug output (optional)"},
    {.identifier = 'C',
     .access_name = "debug-coils",
     .value_name = NULL,
     .description = "Enable coils debug output (optional)"},
    {.identifier = 'L',
     .access_name = "debug-lamps",
     .value_name = NULL,
     .description = "Enable lamps debug output (optional)"},
    {.identifier = '0', .access_name = "switch-test", .value_name = NULL, .description = "Run switch test"},
    {.identifier = '1', .access_name = "coil-test", .value_name = NULL, .description = "Run coil test"},
    {.identifier = '2', .access_name = "lamp-test", .value_name = NULL, .description = "Run lamp test"},
    {.identifier = 'D',
     .access_name = "translite",
     .value_name = "VALUE",
     .description = "Translite file, only used for game in play if a attract mode translite is set as well"},
    {.identifier = 'E',
     .access_name = "translite-attract",
     .value_name = "VALUE",
     .description = "Translite file for attract mode"},
    {.identifier = 'F',
     .access_name = "translite-window",
     .value_name = NULL,
     .description = "Show translite in window instead of fullscreen"},
    {.identifier = 'G',
     .access_name = "translite-width",
     .value_name = "VALUE",
     .description = "Translite width, default 1920"},
    {.identifier = 'H',
     .access_name = "translite-height",
     .value_name = "VALUE",
     .description = "Translite height, default 1080"},
    {.identifier = 'I',
     .access_name = "translite-screen",
     .value_name = "VALUE",
     .description = "Show translite on a specific screen"},
    {.identifier = 'h', .access_letters = "h", .access_name = "help", .description = "Show help"}};

void PINMAMECALLBACK Game(PinmameGame* game)
{
  printf(
      "Game(): name=%s, description=%s, manufacturer=%s, year=%s, "
      "flags=%lu, found=%d\n",
      game->name, game->description, game->manufacturer, game->year, (unsigned long)game->flags, game->found);
}

void PINMAMECALLBACK OnStateUpdated(int state, const void* p_userData)
{
  if (opt_debug)
  {
    printf("OnStateUpdated(): state=%d\n", state);
  }

  if (!state)
  {
    exit(1);
  }
  else
  {
    /*
    PinmameMechConfig mechConfig;
    memset(&mechConfig, 0, sizeof(mechConfig));

    mechConfig.sol1 = 11;
    mechConfig.length = 240;
    mechConfig.steps = 240;
    mechConfig.type = PINMAME_MECH_FLAGS_NONLINEAR | PINMAME_MECH_FLAGS_REVERSE | PINMAME_MECH_FLAGS_ONESOL;
    mechConfig.sw[0].swNo = 32;
    mechConfig.sw[0].startPos = 0;
    mechConfig.sw[0].endPos = 5;

    PinmameSetMech(0, &mechConfig);
    */

    game_state = state;
  }
}

void PINMAMECALLBACK OnLogMessage(PINMAME_LOG_LEVEL logLevel, const char* format, va_list args, const void* p_userData)
{
  char buffer[1024];
  vsnprintf(buffer, sizeof(buffer), format, args);

  if (logLevel == PINMAME_LOG_LEVEL_INFO)
  {
    printf("INFO: %s", buffer);
  }
  else if (logLevel == PINMAME_LOG_LEVEL_ERROR)
  {
    printf("ERROR: %s", buffer);
  }
}

void DMDUTILCALLBACK DMDUtilLogCallback(DMDUtil_LogLevel logLevel, const char* format, va_list args)
{
  char buffer[1024];
  vsnprintf(buffer, sizeof(buffer), format, args);
  uint32_t now =
      std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
          .count();

  if (logLevel == DMDUtil_LogLevel_INFO)
  {
    printf("%lu INFO: %s", now, buffer);
  }
  else if (logLevel == DMDUtil_LogLevel_DEBUG)
  {
    printf("%lu DEBUG: %s\n", now, buffer);
  }
  else if (logLevel == DMDUtil_LogLevel_ERROR)
  {
    printf("%lu ERROR: %s", now, buffer);
  }
}

void PINMAMECALLBACK OnDisplayAvailable(int index, int displayCount, PinmameDisplayLayout* p_displayLayout,
                                        const void* p_userData)
{
  if (opt_debug)
  {
    printf(
        "OnDisplayAvailable(): index=%d, displayCount=%d, type=%d, top=%d, "
        "left=%d, width=%d, height=%d, "
        "depth=%d, length=%d\n",
        index, displayCount, p_displayLayout->type, p_displayLayout->top, p_displayLayout->left, p_displayLayout->width,
        p_displayLayout->height, p_displayLayout->depth, p_displayLayout->length);
  }
}

void PINMAMECALLBACK OnDisplayUpdated(int index, void* p_displayData, PinmameDisplayLayout* p_displayLayout,
                                      const void* p_userData)
{
  if (p_displayData == nullptr)
  {
    return;
  }

  if (opt_debug)
  {
    printf(
        "OnDisplayUpdated(): index=%d, type=%d, top=%d, left=%d, width=%d, "
        "height=%d, depth=%d, length=%d\n",
        index, p_displayLayout->type, p_displayLayout->top, p_displayLayout->left, p_displayLayout->width,
        p_displayLayout->height, p_displayLayout->depth, p_displayLayout->length);
  }

  // For DMD games, the ype is PINMAME_DISPLAY_TYPE_DMD.
  // For alphanumeric games that should be shown on a DMD,
  // the type is PINMAME_DISPLAY_TYPE_DMD | PINMAME_DISPLAY_TYPE_DMDSEG.
  if ((p_displayLayout->type & PINMAME_DISPLAY_TYPE_DMD) == PINMAME_DISPLAY_TYPE_DMD)
  {
    pDmd->UpdateData((uint8_t*)p_displayData, p_displayLayout->depth, p_displayLayout->width, p_displayLayout->height,
                     255, 255, 255, opt_rom);
  }
  else
  {
    switch (p_displayLayout->type)
    {
      case PINMAME_DISPLAY_TYPE_SEG16:    // 16 segments
      case PINMAME_DISPLAY_TYPE_SEG16R:   // 16 segments with comma and period
                                          // reversed
      case PINMAME_DISPLAY_TYPE_SEG10:    // 9 segments and comma
      case PINMAME_DISPLAY_TYPE_SEG9:     // 9 segments
      case PINMAME_DISPLAY_TYPE_SEG8:     // 7 segments and comma
      case PINMAME_DISPLAY_TYPE_SEG8D:    // 7 segments and period
      case PINMAME_DISPLAY_TYPE_SEG7:     // 7 segments
      case PINMAME_DISPLAY_TYPE_SEG87:    // 7 segments, comma every three
      case PINMAME_DISPLAY_TYPE_SEG87F:   // 7 segments, forced comma every three
      case PINMAME_DISPLAY_TYPE_SEG98:    // 9 segments, comma every three
      case PINMAME_DISPLAY_TYPE_SEG98F:   // 9 segments, forced comma every three
      case PINMAME_DISPLAY_TYPE_SEG7S:    // 7 segments, small
      case PINMAME_DISPLAY_TYPE_SEG7SC:   // 7 segments, small, with comma
      case PINMAME_DISPLAY_TYPE_SEG16S:   // 16 segments with split top and
                                          // bottom line
      case PINMAME_DISPLAY_TYPE_SEG16N:   // 16 segments without commas
      case PINMAME_DISPLAY_TYPE_SEG16D:   // 16 segments with periods only
      case PINMAME_DISPLAY_TYPE_SEGALL:   // maximum segment definition number
      case PINMAME_DISPLAY_TYPE_IMPORT:   // Link to another display layout
      case PINMAME_DISPLAY_TYPE_SEGMASK:  // Note that CORE_IMPORT must be part of the segmask as well!
        break;

      case PINMAME_DISPLAY_TYPE_VIDEO:  // VIDEO Display
        // @todo
        break;

      case PINMAME_DISPLAY_TYPE_DMD:     // DMD Display
      case PINMAME_DISPLAY_TYPE_DMDSEG:  // @todo
        // handled above, just surpress a warning of missing cases here.
        break;

      case PINMAME_DISPLAY_TYPE_SEGHIBIT:
      case PINMAME_DISPLAY_TYPE_SEGREV:
      case PINMAME_DISPLAY_TYPE_DMDNOAA:
      case PINMAME_DISPLAY_TYPE_NODISP:
        break;
    }
  }
}

int PINMAMECALLBACK OnAudioAvailable(PinmameAudioInfo* p_audioInfo, const void* p_userData)
{
  if (opt_debug)
  {
    printf(
        "OnAudioAvailable(): format=%d, channels=%d, sampleRate=%.2f, "
        "framesPerSecond=%.2f, samplesPerFrame=%d, "
        "bufferSize=%d\n",
        p_audioInfo->format, p_audioInfo->channels, p_audioInfo->sampleRate, p_audioInfo->framesPerSecond,
        p_audioInfo->samplesPerFrame, p_audioInfo->bufferSize);
  }

  _audioChannels = p_audioInfo->channels;
  _audioSampleRate = (int)p_audioInfo->sampleRate;

  for (int index = 0; index < MAX_AUDIO_BUFFERS; index++)
  {
    int bufferSize = p_audioInfo->samplesPerFrame * _audioChannels * sizeof(int16_t);
    void* p_buffer = malloc(bufferSize);
    memset(p_buffer, 0, bufferSize);

    alBufferData(_audioBuffers[index], _audioChannels == 2 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16, p_buffer,
                 bufferSize, _audioSampleRate);
  }

  alSourceQueueBuffers(_audioSource, MAX_AUDIO_BUFFERS, _audioBuffers);
  alSourcePlay(_audioSource);

  return p_audioInfo->samplesPerFrame;
}

int PINMAMECALLBACK OnAudioUpdated(void* p_buffer, int samples, const void* p_userData)
{
  if (opt_no_sound) return 0;

  if (_audioQueue.size() >= MAX_AUDIO_QUEUE_SIZE)
  {
    while (!_audioQueue.empty())
    {
      void* p_destBuffer = _audioQueue.front();

      free(p_destBuffer);
      _audioQueue.pop();
    }
  }

  int bufferSize = samples * _audioChannels * sizeof(int16_t);
  void* p_destBuffer = malloc(bufferSize);
  memcpy(p_destBuffer, p_buffer, bufferSize);

  _audioQueue.push(p_destBuffer);

  ALint buffersProcessed;
  alGetSourcei(_audioSource, AL_BUFFERS_PROCESSED, &buffersProcessed);

  if (buffersProcessed <= 0)
  {
    return samples;
  }

  while (buffersProcessed > 0)
  {
    ALuint buffer = 0;
    alSourceUnqueueBuffers(_audioSource, 1, &buffer);

    if (_audioQueue.size() > 0)
    {
      void* p_destBuffer = _audioQueue.front();

      alBufferData(buffer, _audioChannels == 2 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16, p_destBuffer, bufferSize,
                   _audioSampleRate);

      free(p_destBuffer);
      _audioQueue.pop();
    }

    alSourceQueueBuffers(_audioSource, 1, &buffer);
    buffersProcessed--;
  }

  ALint state;
  alGetSourcei(_audioSource, AL_SOURCE_STATE, &state);

  if (state != AL_PLAYING)
  {
    alSourcePlay(_audioSource);
  }

  return samples;
}

void PINMAMECALLBACK OnSolenoidUpdated(PinmameSolenoidState* p_solenoidState, const void* p_userData)
{
  if (opt_debug || opt_debug_coils)
  {
    printf("OnSolenoidUpdated: solenoid=%d, state=%d\n", p_solenoidState->solNo, p_solenoidState->state);
  }

  ppuc->SetSolenoidState(p_solenoidState->solNo, p_solenoidState->state);

  if (p_solenoidState->solNo == ppuc->GetGameOnSolenoid())
  {
    if (p_solenoidState->state)
    {
      SDL_RenderTexture(pRenderer, pTransliteTexture, nullptr, nullptr);
      SDL_RenderPresent(pRenderer);
    }
    else if (pTransliteAttractTexture)
    {
      SDL_RenderTexture(pRenderer, pTransliteAttractTexture, nullptr, nullptr);
      SDL_RenderPresent(pRenderer);
    }
  }
}

void PINMAMECALLBACK OnMechAvailable(int mechNo, PinmameMechInfo* p_mechInfo, const void* p_userData)
{
  if (opt_debug)
  {
    printf(
        "OnMechAvailable: mechNo=%d, type=%d, length=%d, steps=%d, pos=%d, "
        "speed=%d\n",
        mechNo, p_mechInfo->type, p_mechInfo->length, p_mechInfo->steps, p_mechInfo->pos, p_mechInfo->speed);
  }
}

void PINMAMECALLBACK OnMechUpdated(int mechNo, PinmameMechInfo* p_mechInfo, const void* p_userData)
{
  if (opt_debug)
  {
    printf(
        "OnMechUpdated: mechNo=%d, type=%d, length=%d, steps=%d, pos=%d, "
        "speed=%d\n",
        mechNo, p_mechInfo->type, p_mechInfo->length, p_mechInfo->steps, p_mechInfo->pos, p_mechInfo->speed);
  }
}

void PINMAMECALLBACK OnConsoleDataUpdated(void* p_data, int size, const void* p_userData)
{
  if (opt_debug)
  {
    printf("OnConsoleDataUpdated: size=%d\n", size);
  }
}

int PINMAMECALLBACK IsKeyPressed(PINMAME_KEYCODE keycode, const void* p_userData) { return 0; }

int main(int argc, char* argv[])
{
  char identifier;
  cag_option_context cag_context;
  const char* config_file = NULL;
  const char* opt_serial = NULL;
  uint8_t opt_serum_timeout = 0;
  uint8_t opt_serum_skip_frames = 0;
  bool opt_dump = false;
  bool opt_switch_test = false;
  bool opt_coil_test = false;
  bool opt_lamp_test = false;
  const char* opt_translite = NULL;
  const char* opt_translite_attract = NULL;
  bool opt_translite_window = false;
  uint16_t opt_translite_width = 1920;
  uint16_t opt_translite_height = 1080;
  int8_t opt_translite_screen = -1;

  cag_option_init(&cag_context, options, CAG_ARRAY_SIZE(options), argc, argv);
  while (cag_option_fetch(&cag_context))
  {
    identifier = cag_option_get_identifier(&cag_context);
    switch (identifier)
    {
      case 'c':
        config_file = cag_option_get_value(&cag_context);
        break;
      case 'r':
        opt_rom = cag_option_get_value(&cag_context);
        break;
      case 's':
        opt_serial = cag_option_get_value(&cag_context);
        break;
      case 'n':
        opt_no_serial = true;
        break;
      case 'M':
        opt_no_sound = true;
        break;
      case 'u':
        opt_serum = true;
        break;
      case 't':
        opt_serum_timeout = atoi(cag_option_get_value(&cag_context));
        break;
      case 'P':
        opt_serum_skip_frames = atoi(cag_option_get_value(&cag_context));
        break;
      case 'p':
        opt_pup = true;
        break;
      case 'i':
        opt_console_display = true;
        break;
      case 'm':
        opt_dump = true;
        break;
      case 'd':
        opt_debug = true;
        break;
      case 'S':
        opt_debug_switches = true;
        break;
      case 'C':
        opt_debug_coils = true;
        break;
      case 'L':
        opt_debug_lamps = true;
        break;
      case '0':
        opt_switch_test = true;
        break;
      case '1':
        opt_coil_test = true;
        break;
      case '2':
        opt_lamp_test = true;
        break;
      case 'D':
        opt_translite = cag_option_get_value(&cag_context);
        break;
      case 'E':
        opt_translite_attract = cag_option_get_value(&cag_context);
        break;
      case 'F':
        opt_translite_window = true;
        break;
      case 'G':
        opt_translite_width = atoi(cag_option_get_value(&cag_context));
        break;
      case 'H':
        opt_translite_height = atoi(cag_option_get_value(&cag_context));
        break;
      case 'I':
        opt_translite_screen = atoi(cag_option_get_value(&cag_context));
        break;
      case 'h':
        printf("Usage: ppuc [OPTION]...\n");
        cag_option_print(options, CAG_ARRAY_SIZE(options), stdout);
        return 0;
    }
  }

  if (!config_file)
  {
    printf("No config file provided. Use option -c /path/to/config/file.\n");
    return -1;
  }

  if (opt_translite)
  {
// Set the SDL video driver for Linux framebuffer
#ifdef __linux__
    setenv("SDL_VIDEODRIVER", "KMSDRM", 1);
#endif

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
      printf("SDL_Init Error: %s\n", SDL_GetError());
      return 1;
    }

    if (!SDL_CreateWindowAndRenderer("PPUC", opt_translite_width, opt_translite_height,
                                     opt_translite_window ? SDL_WINDOW_BORDERLESS : SDL_WINDOW_FULLSCREEN, &pWindow,
                                     &pRenderer))
    {
      printf("SDL couldn't create window/renderer: %s", SDL_GetError());
      return SDL_APP_FAILURE;
    }

    SDL_SetWindowPosition(pWindow, 0, 0);
    while (!SDL_SyncWindow(pWindow));

    pTransliteTexture = IMG_LoadTexture(pRenderer, opt_translite);
    if (!pTransliteTexture)
    {
      printf("IMG_LoadTexture Error: %s\n", SDL_GetError());
      SDL_DestroyRenderer(pRenderer);
      SDL_DestroyWindow(pWindow);
      SDL_Quit();
      return 1;
    }

    if (opt_translite_attract)
    {
      pTransliteAttractTexture = IMG_LoadTexture(pRenderer, opt_translite_attract);
      if (!pTransliteAttractTexture)
      {
        printf("IMG_LoadTexture Error: %s\n", SDL_GetError());
        SDL_DestroyTexture(pTransliteTexture);
        SDL_DestroyRenderer(pRenderer);
        SDL_DestroyWindow(pWindow);
        SDL_Quit();
        return 1;
      }
      SDL_RenderTexture(pRenderer, pTransliteAttractTexture, nullptr, nullptr);
    }
    else
    {
      SDL_RenderTexture(pRenderer, pTransliteTexture, nullptr, nullptr);
    }

    SDL_RenderPresent(pRenderer);
  }

  ppuc = new PPUC();

  // Load config file. But options set via command line are preferred.
  ppuc->LoadConfiguration(config_file);

  if (!opt_debug)
  {
    opt_debug = ppuc->GetDebug();
  }
  ppuc->SetDebug(opt_debug);

  if (opt_rom)
  {
    ppuc->SetRom(opt_rom);
  }
  else
  {
    opt_rom = ppuc->GetRom();
  }

  if (opt_serial)
  {
    ppuc->SetSerial(opt_serial);
  }
  else
  {
    // opt_serial will be ignored by ZeDMD later.
    opt_serial = ppuc->GetSerial();
  }

  if (opt_switch_test || opt_coil_test || opt_lamp_test)
  {
    if (!ppuc->Connect())
    {
      printf("Unable to open serial communication to PPUC boards.\n");
      return 1;
    }

    if (opt_lamp_test)
    {
      ppuc->LampTest();
    }

    if (opt_coil_test)
    {
      ppuc->CoilTest();
    }

    if (opt_switch_test)
    {
      ppuc->SwitchTest();
    }

    ppuc->Disconnect();

    return 0;
  }

  // Initialize displays.
  // ZeDMD messes with USB ports. when searching for the DMD.
  // So it is important to start that search before the RS485 BUS gets
  // initialized.
  DMDUtil::Config* dmdConfig = DMDUtil::Config::GetInstance();

  PinmameConfig config = {
      PINMAME_AUDIO_FORMAT_INT16,
      44100,
      "",
      &OnStateUpdated,
      &OnDisplayAvailable,
      &OnDisplayUpdated,
      &OnAudioAvailable,
      &OnAudioUpdated,
      &OnMechAvailable,
      &OnMechUpdated,
      &OnSolenoidUpdated,
      &OnConsoleDataUpdated,
      &IsKeyPressed,
      &OnLogMessage,
      NULL,
  };

#if defined(_WIN32) || defined(_WIN64)
  snprintf((char*)config.vpmPath, PINMAME_MAX_PATH, "%s%s\\pinmame\\", getenv("HOMEDRIVE"), getenv("HOMEPATH"));
#else
  snprintf((char*)config.vpmPath, PINMAME_MAX_PATH, "%s/.pinmame/", getenv("HOME"));
#endif

  if (opt_serum)
  {
    char altcolorPath[PINMAME_MAX_PATH + 10];
#if defined(_WIN32) || defined(_WIN64)
    snprintf(altcolorPath, PINMAME_MAX_PATH + 8, "%saltcolor", config.vpmPath);
#else
    snprintf(altcolorPath, PINMAME_MAX_PATH + 8, "%saltcolor", config.vpmPath);
#endif

    dmdConfig->SetLogCallback(DMDUtilLogCallback);
    dmdConfig->SetLogLevel(DMDUtil_LogLevel_DEBUG);
    dmdConfig->SetAltColorPath(altcolorPath);
    dmdConfig->SetAltColor(true);

    if (opt_serum_timeout)
    {
      dmdConfig->SetIgnoreUnknownFramesTimeout(opt_serum_timeout);
    }
    if (opt_serum_skip_frames)
    {
      dmdConfig->SetMaximumUnknownFramesToSkip(opt_serum_skip_frames);
    }
  }
  else
  {
    dmdConfig->SetAltColor(false);
  }

  if (opt_pup)
  {
    dmdConfig->SetPUPVideosPath(getenv("HOME"));
    dmdConfig->SetPUPCapture(true);
  }

  if (opt_debug)
  {
    printf("Finding displays...\n");
    dmdConfig->SetZeDMDDebug(opt_debug);
  }

  pDmd = new DMDUtil::DMD();
  pDmd->SetRomName(opt_rom);
  pDmd->FindDisplays();
  if (opt_console_display)
  {
    pDmd->CreateConsoleDMD(!opt_debug);
  }
  if (opt_dump)
  {
    pDmd->DumpDMDTxt();
  }

  while (pDmd->IsFinding()) std::this_thread::sleep_for(std::chrono::milliseconds(100));

  if (!opt_no_serial && !ppuc->Connect())
  {
    printf("Unable to open serial communication to PPUC boards.\n");
    return 1;
  }

  ALCdevice* device;
  if (!opt_no_sound)
  {
    // Initialize the sound device
    const ALCchar* defaultDeviceName = alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);
    device = alcOpenDevice(defaultDeviceName);
    if (!device)
    {
      printf("failed to alcOpenDevice for %s\n", defaultDeviceName);
      return 1;
    }

    ALCcontext* context = alcCreateContext(device, NULL);
    if (!context)
    {
      printf("failed call to alcCreateContext\n");
      return 1;
    }

    alcMakeContextCurrent(context);
    alGenSources((ALuint)1, &_audioSource);
    alGenBuffers(MAX_AUDIO_BUFFERS, _audioBuffers);
  }

  PinmameSetConfig(&config);

  PinmameSetDmdMode(PINMAME_DMD_MODE_RAW);
  // PinmameSetSoundMode(PINMAME_SOUND_MODE_ALTSOUND);
  PinmameSetHandleKeyboard(0);
  PinmameSetHandleMechanics(0);

#if defined(_WIN32) || defined(_WIN64)
  // Avoid compile error C2131. Use a larger constant value instead.
  PinmameLampState changedLampStates[256];
#else
  PinmameLampState changedLampStates[PinmameGetMaxLamps()];
#endif

  if (PinmameRun(opt_rom) == PINMAME_STATUS_OK)
  {
    // Pinball machines were slower than modern CPUs. There's no need to
    // update states too frequently at full speed.
    int sleep_us = 1000;
    // Poll I/O boards for events (mainly switches) every 50us.
    int poll_interval_ms = 50;
    int poll_trigger = poll_interval_ms * 1000 / sleep_us;
    int index_recv = 0;

    ppuc->StartUpdates();

    bool running = true;
    while (running)
    {
      std::this_thread::sleep_for(std::chrono::microseconds(sleep_us));

      if (!game_state)
      {
        continue;
      }

      if (--poll_trigger <= 0)
      {
        poll_trigger = poll_interval_ms * 1000 / sleep_us;

        PPUCSwitchState* switchState;
        while ((switchState = ppuc->GetNextSwitchState()) != nullptr)
        {
          if (opt_debug || opt_debug_switches)
          {
            printf("Switch updated: #%d, %d\n", switchState->number, switchState->state);
          }

          // Switches between 200 and 240 are custom switches within the io-boards which should not be sent to pinmame.
          // Switches above 240 will become negative values, for example 243 => -3.
          if (switchState->number < 200 || switchState->number > 241)
          {
            int switchNumber = (switchState->number < 241) ? switchState->number : 240 - switchState->number;
            PinmameSetSwitch(switchNumber, switchState->state);
          }
        }
      }

      int count = PinmameGetChangedLamps(changedLampStates);
      for (int c = 0; c < count; c++)
      {
        uint16_t lampNo = changedLampStates[c].lampNo;
        uint8_t lampState = changedLampStates[c].state == 0 ? 0 : 1;

        if (opt_debug || opt_debug_lamps)
        {
          printf("Lamp updated: #%d, %d\n", lampNo, lampState);
        }

        ppuc->SetLampState(lampNo, lampState);
      }

      SDL_Event event;
      if (SDL_PollEvent(&event))
      {
        switch (event.type)
        {
          case SDL_EVENT_QUIT:
            running = false;
            break;
          case SDL_EVENT_KEY_DOWN:
            if (opt_debug) printf("Key pressed: %d\n", event.key.key);
            switch (event.key.key)
            {
              case SDLK_ESCAPE:
                running = false;
                break;
              case 53:  // 5
                PinmameSetSwitch(4, 1);
                break;
              case 13:  // Enter
                PinmameSetSwitch(3, 1);
                break;
            }
            break;
        }
      }
    }
  }

  if (!opt_no_serial)
  {
    // Close the serial device
    ppuc->Disconnect();
  }

  if (device)
  {
    alcCloseDevice(device);
  }

  delete pDmd;

  // Cleanup SDL
  if (pTransliteTexture)
  {
    SDL_DestroyTexture(pTransliteTexture);
    if (pTransliteAttractTexture)
    {
      SDL_DestroyTexture(pTransliteAttractTexture);
    }
    SDL_DestroyRenderer(pRenderer);
    SDL_DestroyWindow(pWindow);
    SDL_Quit();
  }

  return 0;
}
