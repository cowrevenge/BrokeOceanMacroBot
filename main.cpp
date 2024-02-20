#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <string>
#include <chrono>
#include <thread>
#include <future>
#include <condition_variable>
#include <random>
#include <map>
#include <array>
#include <atomic>
#include "sdl_usbhid_scan_codes.h"

#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>

using namespace std::chrono_literals;
std::string Champ = "None";


const int WINDOW_WIDTH = 1024;
const int WINDOW_HEIGHT = 1080-64;
const int BUTTON_WIDTH = 120;
const int BUTTON_HEIGHT = 42;

const SDL_Color GREEN_COLOR = {0, 255, 0, 255};
const SDL_Color BLUE_COLOR = {0, 0, 255, 255};
const SDL_Color YELLOW_COLOR = {255, 255, 0, 255};
const SDL_Color RED_COLOR = {255, 0, 0, 255};

const SDL_Rect START_STOP_RECT =    {6, 100, BUTTON_WIDTH, BUTTON_HEIGHT}; //231
const SDL_Rect COUNTER_RECT =       {6, 100+BUTTON_HEIGHT+5, BUTTON_WIDTH, BUTTON_HEIGHT}; //359
const SDL_Rect RESET_RECT =         {6, 100+((BUTTON_HEIGHT+5)*2), BUTTON_WIDTH, BUTTON_HEIGHT}; //
const SDL_Rect CLOSE_APP_RECT =     {6, 100+((BUTTON_HEIGHT+5)*3), BUTTON_WIDTH, BUTTON_HEIGHT};

const SDL_Rect LAHN_BUTTON_RECT =     {6, 100+((BUTTON_HEIGHT+5)*5), BUTTON_WIDTH, BUTTON_HEIGHT};
const SDL_Rect WOOSA_BUTTON_RECT =    {6, 100+((BUTTON_HEIGHT+5)*6), BUTTON_WIDTH, BUTTON_HEIGHT};
const SDL_Rect SCHOLAR_BUTTON_RECT =  {6, 100+((BUTTON_HEIGHT+5)*7), BUTTON_WIDTH, BUTTON_HEIGHT};
const SDL_Rect WITCH_BUTTON_RECT =    {6, 100+((BUTTON_HEIGHT+5)*8), BUTTON_WIDTH, BUTTON_HEIGHT};
const SDL_Rect GUARDIAN_SUCC_BUTTON_RECT =    {6, 100+((BUTTON_HEIGHT+5)*9), BUTTON_WIDTH, BUTTON_HEIGHT};
const SDL_Rect GUARDIAN_AWAKE_BUTTON_RECT =    {6, 100+((BUTTON_HEIGHT+5)*10), BUTTON_WIDTH, BUTTON_HEIGHT};

const SDL_Color LAHN_COLOR = {128, 0, 128, 255}; // Purple
const SDL_Color WOOSA_COLOR = {160, 32, 240, 255}; // Lighter, distinct Purple
const SDL_Color SCHOLAR_COLOR = {255, 165, 0, 255}; // Orange
const SDL_Color WITCH_COLOR = {0, 128, 0, 255}; // Green
const SDL_Color GUARDIAN_SUCC_COLOR = {0, 191, 255, 255}; // Deep Sky Blue
const SDL_Color GUARDIAN_AWAKE_COLOR = {30, 144, 255, 255}; // Dodger Blue

std::atomic<bool> isRunning{ false };

struct Button {
    SDL_Rect rect;
    SDL_Color color;
};

bool is_inside(SDL_Point p, SDL_Rect r) {
    return (p.x > r.x && p.x < r.x + r.w && p.y > r.y && p.y < r.y + r.h);
}

struct MouseButton {
    std::array<unsigned char, 4> LMB = {1, 0, 0, 0};
    std::array<unsigned char, 4> RMB = {2, 0, 0, 0};
    std::array<unsigned char, 4> Both = {3, 0, 0, 0};
};

MouseButton mouseButton;

// TextBox structure definition
struct TextBox {
    SDL_Rect rect;       // Rectangle position and size
    std::string text;    // Text inside the textbox
    bool hasFocus;       // Whether the textbox currently has focus
};

// Define Variable Input Displays
std::map<std::string, TextBox> textBoxes;

int startX = 350; // Starting x position
int startY = 100; // Starting y position
int spacing = 30; // Spacing between text boxes
int width = 200; // Width of text boxes
int height = 18; // Height of text boxes

std::vector<std::string> variableNames = {
    "attack1Duration", "attack2Duration", "attack3Duration", "attack4Duration", "attack5Duration",
    "attack6Duration", "attack7Duration", "attack8Duration", "attack9Duration", "attack10Duration",
    "attack11Duration", "attack12Duration",

    "attack1Key", "attack2Key", "attack3Key", "attack4Key", "attack5Key",
    "attack6Key", "attack7Key", "attack8Key", "attack9Key", "attack10Key",
    "attack11Key", "attack12Key",

    "attack1MouseButton", "attack2MouseButton", "attack3MouseButton", "attack4MouseButton", "attack5MouseButton",
    "attack6MouseButton", "attack7MouseButton", "attack8MouseButton", "attack9MouseButton", "attack10MouseButton",
    "attack11MouseButton", "attack12MouseButton",

    "attack1SleepTimer", "attack2SleepTimer", "attack3SleepTimer", "attack4SleepTimer", "attack5SleepTimer",
    "attack6SleepTimer", "attack7SleepTimer", "attack8SleepTimer", "attack9SleepTimer", "attack10SleepTimer",
    "attack11SleepTimer", "attack12SleepTimer"
};

std::map<std::string, std::string> initializeVariablesMap() {
    std::map<std::string, std::string> variablesMap;

    // Initialize variablesMap with placeholders
    for (int i = 1; i <= 12; ++i) {
        variablesMap["attack" + std::to_string(i) + "Duration"] = "PlaceHolder";
        variablesMap["attack" + std::to_string(i) + "SleepTimer"] = "PlaceHolder";
        variablesMap["attack" + std::to_string(i) + "Key"] = "PlaceHolder";
        variablesMap["attack" + std::to_string(i) + "MouseButton"] = "PlaceHolder";
    }

    return variablesMap;
}

void renderTextBox(SDL_Renderer* renderer, TTF_Font* font, const TextBox& textBox) {
    // Render text box background
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    SDL_RenderFillRect(renderer, &textBox.rect);

    // Render text
    SDL_Color textColor = {0, 0, 0}; // Black color
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, textBox.text.c_str(), textColor);

    if (!textSurface) {
        std::cerr << "Unable to render text. TTF error: " << TTF_GetError() << std::endl;
        return;
    }

    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (!textTexture) {
        std::cerr << "Unable to create text texture. SDL error: " << SDL_GetError() << std::endl;
        SDL_FreeSurface(textSurface);
        return;
    }

    SDL_Rect renderQuad = {
        textBox.rect.x,
        textBox.rect.y,
        textSurface->w,
        textSurface->h
    };

    SDL_RenderCopy(renderer, textTexture, NULL, &renderQuad);

    // Clean up
    SDL_DestroyTexture(textTexture);
    SDL_FreeSurface(textSurface);
}

void renderButton(SDL_Renderer* renderer, TTF_Font* font, SDL_Rect buttonRect, SDL_Color buttonColor, const std::string& buttonText) {
    // Set the button color
    SDL_SetRenderDrawColor(renderer, buttonColor.r, buttonColor.g, buttonColor.b, buttonColor.a);
    SDL_RenderFillRect(renderer, &buttonRect);

    // Render the button text
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, buttonText.c_str(), {0, 0, 0}); // Black text
    if (!textSurface) {
        std::cerr << "Unable to create text surface. Error: " << TTF_GetError() << std::endl;
        return;
    }
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (!textTexture) {
        std::cerr << "Unable to create text texture. Error: " << SDL_GetError() << std::endl;
        SDL_FreeSurface(textSurface);
        return;
    }

    // Calculate text position to center it on the button
    int textWidth = textSurface->w;
    int textHeight = textSurface->h;
    SDL_FreeSurface(textSurface);
    SDL_Rect textRect = {
        buttonRect.x + (buttonRect.w - textWidth) / 2,
        buttonRect.y + (buttonRect.h - textHeight) / 2,
        textWidth,
        textHeight
    };

    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_DestroyTexture(textTexture);
}

struct Label {
    SDL_Rect rect;
    std::string text;
};

std::map<std::string, Label> textBoxLabels;
std::map<std::string, std::string> variableValues;

Label currentChampLabel;

void initializeTextBoxes(const std::map<std::string, std::string>& variablesMap) {
    for (size_t i = 0; i < variableNames.size(); i++) {
        const std::string& varName = variableNames[i];

        // Calculate row and column for positioning
        int row = i / 2; // Integer division to get row number
        int col = i % 2; // Remainder to get column (0 for left, 1 for right)

        // Adjust starting X based on column
        int adjustedStartX = startX + col * (width + spacing);

        if (col == 1) {
            adjustedStartX += 200;
        }

        // Initialize labels
        Label label;
        label.rect = { adjustedStartX - 220, startY + row * spacing, 70, height };
        label.text = varName;
        textBoxLabels[varName] = label;

        // Initialize text boxes with the value from variablesMap
        TextBox box;
        box.rect = { adjustedStartX, startY + row * spacing, width, height };

        // Check if the variable name exists in variablesMap
        auto it = variablesMap.find(varName);
        if (it != variablesMap.end()) {
            box.text = it->second; // Directly assign the string value to box.text
        } else {
        // Handle the case where varName is not found in variablesMap
        box.text = "Value Not Found"; // Set a default value or error message
        }

        box.hasFocus = false;
        textBoxes[varName] = box;
    }
}

void renderLabel(SDL_Renderer* renderer, TTF_Font* font, const Label& label) {
    // Render label text
    SDL_Color textColor = {255, 255, 255}; // White color
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, label.text.c_str(), textColor);

    if (!textSurface) {
        std::cerr << "Unable to render label. TTF error: " << TTF_GetError() << std::endl;
        return;
    }

    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (!textTexture) {
        std::cerr << "Unable to create label texture. SDL error: " << SDL_GetError() << std::endl;
        SDL_FreeSurface(textSurface);
        return;
    }

    SDL_Rect renderQuad = {
        label.rect.x,
        label.rect.y,
        textSurface->w,
        textSurface->h
    };

    SDL_RenderCopy(renderer, textTexture, NULL, &renderQuad);

    // Clean up
    SDL_DestroyTexture(textTexture);
    SDL_FreeSurface(textSurface);
}

std::random_device rd; // obtain a random number from hardware
std::mt19937 generator(rd()); // seed the generator
std::uniform_int_distribution<> simultaneousPressDistr(2, 5); // Tiny delay

const int TEXT_OFFSET = 10;
const int TEXT_SPACING = 30;

std::ofstream hidFileKeyboard;  // Declare the file stream object globally
std::ofstream hidFileMouse;    // Declare hidFileMouse at global scope

const size_t keyBufferSize = 7;
const size_t mouseBufferSize = 4;

unsigned char keyboardBuffer[keyBufferSize] = {0, 0, 0, 0, 0, 0, 0}; // reserved, 7 buttons
size_t keyboardBufferIndex = 0;

unsigned char finalKeyboardBuffer[8] = {0, 0, 0, 0, 0, 0, 0, 0}; // mods, reserved, 7 buttons
unsigned char emptyKey[8] = {0, 0, 0, 0, 0, 0, 0, 0}; // Empty Keyboard


unsigned char keyboardModifiers = 0;

int mouseDeltaX = 0;
int mouseDeltaY = 0;

unsigned char finalMouseBuffer[mouseBufferSize] = {0, 0, 0, 0}; // buttons, x, y, scroll
unsigned char emptyMouse[mouseBufferSize] = {0, 0, 0, 0};

std::atomic<bool> isMacroActive{ false };  // Flag to track if any macro key is pressed
const SDL_Keycode macroKey = SDLK_e;
const SDL_Keycode ZBuffKey = SDLK_z;

class AttackEvent {
public:
    int id;
    std::atomic<bool> ready;
    std::array<unsigned char, 8> key;
    std::array<unsigned char, 4> mouseButton;
    std::chrono::milliseconds sleepTimer;
    std::chrono::milliseconds duration;

    // Default Constructor
    AttackEvent()
        : id(0), ready(false), key{0}, mouseButton{0}, sleepTimer(0ms), duration(0ms) {}

    // Parameterized Constructor
    AttackEvent(int attackId, bool rdy, std::chrono::milliseconds dur, std::chrono::milliseconds slpTimer,
                std::array<unsigned char, 8> k, std::array<unsigned char, 4> mouseBtn)
        : id(attackId), ready(rdy), key(k), mouseButton(mouseBtn), sleepTimer(slpTimer), duration(dur) {}

    // Copy Constructor
    AttackEvent(const AttackEvent& other)
        : id(other.id), ready(other.ready.load()), key(other.key), mouseButton(other.mouseButton),
          sleepTimer(other.sleepTimer), duration(other.duration) {}

    // Move Constructor
    AttackEvent(AttackEvent&& other) noexcept
        : id(other.id), ready(other.ready.load()), key(std::move(other.key)), mouseButton(std::move(other.mouseButton)),
          sleepTimer(other.sleepTimer), duration(other.duration) {}

    // Copy Assignment Operator
    AttackEvent& operator=(const AttackEvent& other) {
        id = other.id;
        ready.store(other.ready.load());
        key = other.key;
        mouseButton = other.mouseButton;
        sleepTimer = other.sleepTimer;
        duration = other.duration;
        return *this;
    }

    // Move Assignment Operator
    AttackEvent& operator=(AttackEvent&& other) noexcept {
        id = other.id;
        ready.store(other.ready.load());
        key = std::move(other.key);
        mouseButton = std::move(other.mouseButton);
        sleepTimer = other.sleepTimer;
        duration = other.duration;
        return *this;
    }
};

std::array<AttackEvent, 12> attacks;

void initializeAttacks() {
    for (int i = 0; i < 12; ++i) {
        attacks[i] = AttackEvent();
        attacks[i].id = i + 1;
        attacks[i].ready = true;
        attacks[i].duration = std::chrono::milliseconds(0);
        attacks[i].sleepTimer = std::chrono::milliseconds(0);
        attacks[i].key.fill(0);
        attacks[i].mouseButton.fill(0);
    }
    // Attack 1
    attacks[0] = AttackEvent(1, true, std::chrono::milliseconds(1100), std::chrono::milliseconds(7000), {0, 0, 0, 0, 0, 0, 0, 0}, {2, 0, 0, 0});

    // Attack 2
    attacks[1] = AttackEvent(2, true, std::chrono::milliseconds(1000), std::chrono::milliseconds(7000), {0, 0, 0, 0, 0, 0, 0, 0}, {3, 0, 0, 0});

    // Attack 3
    attacks[2] = AttackEvent(3, true, std::chrono::milliseconds(1000), std::chrono::milliseconds(6000), {0, 0, 22, 0, 0, 0, 0, 0}, {2, 0, 0, 0});

    // Attack 4
    attacks[3] = AttackEvent(4, true, std::chrono::milliseconds(1000), std::chrono::milliseconds(10000), {0, 0, 0, 0, 0, 0, 0, 0}, {2, 0, 0, 0});

    // Attack 5
    attacks[4] = AttackEvent(5, true, std::chrono::milliseconds(800), std::chrono::milliseconds(7000), {2, 0, 0, 0, 0, 0, 0, 0}, {2, 0, 0, 0});

    // Attack 6
    attacks[5] = AttackEvent(6, true, std::chrono::milliseconds(600), std::chrono::milliseconds(12000), {0, 0, 0, 0, 0, 0, 0, 0}, {2, 0, 0, 0});

    // Attack 7
    attacks[6] = AttackEvent(7, true, std::chrono::milliseconds(1000), std::chrono::milliseconds(6000), {0, 0, 26, 0, 0, 0, 0, 0}, {2, 0, 0, 0});

    // Attack 8
    attacks[7] = AttackEvent(8, true, std::chrono::milliseconds(1000), std::chrono::milliseconds(4000), {0, 0, 9, 0, 0, 0, 0, 0}, {0, 0, 0, 0});

    // Attack 9
    attacks[8] = AttackEvent(9, true, std::chrono::milliseconds(1000), std::chrono::milliseconds(10000), {2, 0, 0, 0, 0, 0, 0, 0}, {1, 0, 0, 0});

    // Attack 10
    attacks[9] = AttackEvent(10, true, std::chrono::milliseconds(1000), std::chrono::milliseconds(10000), {0, 0, 0, 0, 0, 0, 0, 0}, {1, 0, 0, 0});

    // Attack 11
    attacks[10] = AttackEvent(11, false, std::chrono::milliseconds(0), std::chrono::milliseconds(0), {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0});

    // Attack 12
    attacks[11] = AttackEvent(12, false, std::chrono::milliseconds(0), std::chrono::milliseconds(0), {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0});
}

// Define a structure to hold attack configuration for each character
struct AttackConfig {
    int id;
    std::chrono::milliseconds duration;
    std::chrono::milliseconds sleepTimer;
    std::array<unsigned char, 8> key;
    std::array<unsigned char, 4> mouseButton;
};

std::array<unsigned char, 8> hexStringToArray(const std::string& hexStr) {
    std::array<unsigned char, 8> arr = {};
    std::istringstream stream(hexStr);
    int byteVal;
    size_t index = 0;

    while (stream >> std::hex >> byteVal) {
        if (index < arr.size()) {
            arr[index++] = static_cast<unsigned char>(byteVal);
        } else {
            break; // Prevent overflow if more than 8 values are provided
        }
    }

    return arr;
}

// Define the attack configurations for each character
std::array<AttackEvent, 12> lahnAttacks = {
    // Attack 1
    AttackEvent(1, true, std::chrono::milliseconds(1100), std::chrono::milliseconds(7000), {0, 0, 0, 0, 0, 0, 0, 0}, {2, 0, 0, 0}),

    // Attack 2
    AttackEvent(2, true, std::chrono::milliseconds(1000), std::chrono::milliseconds(7000), {0, 0, 0, 0, 0, 0, 0, 0}, {3, 0, 0, 0}),

    // Attack 3
    AttackEvent(3, true, std::chrono::milliseconds(1000), std::chrono::milliseconds(6000), {0, 0, 22, 0, 0, 0, 0, 0}, {2, 0, 0, 0}),

    // Attack 4
    AttackEvent(4, true, std::chrono::milliseconds(1000), std::chrono::milliseconds(10000), {0, 0, 0, 0, 0, 0, 0, 0}, {2, 0, 0, 0}),

    // Attack 5
    AttackEvent(5, true, std::chrono::milliseconds(800), std::chrono::milliseconds(7000), {2, 0, 0, 0, 0, 0, 0, 0}, {2, 0, 0, 0}),

    // Attack 6
    AttackEvent(6, true, std::chrono::milliseconds(600), std::chrono::milliseconds(12000), {0, 0, 0, 0, 0, 0, 0, 0}, {2, 0, 0, 0}),

    // Attack 7
    AttackEvent(7, true, std::chrono::milliseconds(1000), std::chrono::milliseconds(6000), {0, 0, 26, 0, 0, 0, 0, 0}, {2, 0, 0, 0}),

    // Attack 8
    AttackEvent(8, true, std::chrono::milliseconds(1000), std::chrono::milliseconds(4000), {0, 0, 9, 0, 0, 0, 0, 0}, {0, 0, 0, 0}),

    // Attack 9
    AttackEvent(9, true, std::chrono::milliseconds(1000), std::chrono::milliseconds(10000), {2, 0, 0, 0, 0, 0, 0, 0}, {1, 0, 0, 0}),

    // Attack 10
    AttackEvent(10, true, std::chrono::milliseconds(1000), std::chrono::milliseconds(10000), {0, 0, 0, 0, 0, 0, 0, 0}, {1, 0, 0, 0}),

    // Attack 11
    AttackEvent(11, false, std::chrono::milliseconds(0), std::chrono::milliseconds(0), {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0}),

    // Attack 12
    AttackEvent(12, false, std::chrono::milliseconds(0), std::chrono::milliseconds(0), {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0})
};

std::array<AttackEvent, 12> woosaAttacks = {
    // Attack 1
    AttackEvent(1, true, std::chrono::milliseconds(9999), std::chrono::milliseconds(9999), {0, 0, 0, 0, 0, 0, 0, 0}, {2, 0, 0, 0}),

    // Attack 2
    AttackEvent(2, true, std::chrono::milliseconds(9999), std::chrono::milliseconds(9999), {0, 0, 0, 0, 0, 0, 0, 0}, {3, 0, 0, 0}),

    // Attack 3
    AttackEvent(3, true, std::chrono::milliseconds(9999), std::chrono::milliseconds(6000), {0, 0, 22, 0, 0, 0, 0, 0}, {2, 0, 0, 0}),

    // Attack 4
    AttackEvent(4, true, std::chrono::milliseconds(9999), std::chrono::milliseconds(10000), {0, 0, 0, 0, 0, 0, 0, 0}, {2, 0, 0, 0}),

    // Attack 5
    AttackEvent(5, true, std::chrono::milliseconds(9999), std::chrono::milliseconds(7000), {2, 0, 0, 0, 0, 0, 0, 0}, {2, 0, 0, 0}),

    // Attack 6
    AttackEvent(6, true, std::chrono::milliseconds(9999), std::chrono::milliseconds(12000), {0, 0, 0, 0, 0, 0, 0, 0}, {2, 0, 0, 0}),

    // Attack 7
    AttackEvent(7, true, std::chrono::milliseconds(9999), std::chrono::milliseconds(6000), {0, 0, 26, 0, 0, 0, 0, 0}, {2, 0, 0, 0}),

    // Attack 8
    AttackEvent(8, true, std::chrono::milliseconds(9999), std::chrono::milliseconds(4000), {0, 0, 9, 0, 0, 0, 0, 0}, {0, 0, 0, 0}),

    // Attack 9
    AttackEvent(9, true, std::chrono::milliseconds(9999), std::chrono::milliseconds(10000), {2, 0, 0, 0, 0, 0, 0, 0}, {1, 0, 0, 0}),

    // Attack 10
    AttackEvent(10, true, std::chrono::milliseconds(9999), std::chrono::milliseconds(10000), {0, 0, 0, 0, 0, 0, 0, 0}, {1, 0, 0, 0}),

    // Attack 11
    AttackEvent(11, true, std::chrono::milliseconds(9999), std::chrono::milliseconds(0), {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0}),

    // Attack 12
    AttackEvent(12, true, std::chrono::milliseconds(0), std::chrono::milliseconds(0), {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0})
};

std::array<AttackEvent, 12> scholarAttacks = {
    // Attack 1 Shift LMB Hammer Smash
    AttackEvent(1, true, std::chrono::milliseconds(500), std::chrono::milliseconds(9450), {2, 0, 0, 0, 0, 0, 0, 0}, {1, 0, 0, 0}),

    // Attack 2 C Core Fusion
    AttackEvent(2, true, std::chrono::milliseconds(1000), std::chrono::milliseconds(9450), {0, 0, 6, 0, 0, 0, 0, 0}, {0, 0, 0, 0}),

    // Attack 3 W E Hammer in the Knee
    AttackEvent(3, false, std::chrono::milliseconds(800), std::chrono::milliseconds(9450),
            {0, 0, static_cast<unsigned char>(0x1A), 8, 0, 0, 0, 0}, {0, 0, 0, 0}),


    // Attack 4 E
    AttackEvent(4, false, std::chrono::milliseconds(1600), std::chrono::milliseconds(9450), {0, 0, 8, 0, 0, 0, 0, 0}, {0, 0, 0, 0}),

    // Attack 5 S RMB
    AttackEvent(5, false, std::chrono::milliseconds(1200), std::chrono::milliseconds(9450), {0, 0, 22, 0, 0, 0, 0, 0}, {2, 0, 0, 0}),

    // Attack 6 S F
    AttackEvent(6, false, std::chrono::milliseconds(1600), std::chrono::milliseconds(9450), {0, 0, 22, 9, 0, 0, 0, 0}, {0, 0, 0, 0}),

    // Attack 7 Shift F
    AttackEvent(7, false, std::chrono::milliseconds(1800), std::chrono::milliseconds(9450), {2, 0, 9, 0, 0, 0, 0, 0}, {0, 0, 0, 0}),

    // Attack 8 Shift C
    AttackEvent(8, false, std::chrono::milliseconds(1200), std::chrono::milliseconds(9450), {2, 0, 6, 0, 0, 0, 0, 0}, {0, 0, 0, 0}),

    // Attack 9
    AttackEvent(9, false, std::chrono::milliseconds(0), std::chrono::milliseconds(0), {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0}),

    // Attack 10
    AttackEvent(10, false, std::chrono::milliseconds(0), std::chrono::milliseconds(0), {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0}),

    // Attack 11
    AttackEvent(11, false, std::chrono::milliseconds(0), std::chrono::milliseconds(0), {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0}),

    // Attack 12
    AttackEvent(12, false, std::chrono::milliseconds(0), std::chrono::milliseconds(0), {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0})
};

std::array<AttackEvent, 12> witchAttacks = {
    // Attack 1 Buff Shift E
    AttackEvent(1, true, std::chrono::milliseconds(1200), std::chrono::milliseconds(180000), {2, 0, 8, 0, 0, 0, 0, 0}, {0, 0, 0, 0}),

    // Attack 2 Toxic Flood E
    AttackEvent(2, true, std::chrono::milliseconds(2000), std::chrono::milliseconds(15000), {0, 0, 8, 0, 0, 0, 0, 0}, {0, 0, 0, 0}),

    // Attack 3 Equil Break Shift LMB
    AttackEvent(3, true, std::chrono::milliseconds(1600), std::chrono::milliseconds(6000), {2, 0, 0, 0, 0, 0, 0, 0}, {1, 0, 0, 0}),

    // Attack 4 Voltaic Pulse Shift F
    AttackEvent(4, true, std::chrono::milliseconds(2000), std::chrono::milliseconds(6000), {2, 0, 9, 0, 0, 0, 0, 0}, {0, 0, 0, 0}),

    // Attack 5 Barrage Light W RMB
    AttackEvent(5, false, std::chrono::milliseconds(2000), std::chrono::milliseconds(17000),
            {0, 0, static_cast<unsigned char>(0x1A), 0, 0, 0, 0, 0}, {2, 0, 0, 0}),

    // Attack 6 Yoke Ordeel Shift RMB
    AttackEvent(6, true, std::chrono::milliseconds(1000), std::chrono::milliseconds(13000), {2, 0, 0, 0, 0, 0, 0, 0}, {2, 0, 0, 0}),

    // Attack 7 Fissure Wave IV S LMB RMB
    AttackEvent(7, true, std::chrono::milliseconds(1400), std::chrono::milliseconds(3000), {0, 0, 22, 0, 0, 0, 0, 0}, {3, 0, 0, 0}),

    // Attack 8 Thunder Storm S F
    AttackEvent(8, true, std::chrono::milliseconds(1600), std::chrono::milliseconds(11000), {0, 0, 22, 9, 0, 0, 0, 0}, {0, 0, 0, 0}),

    // Attack 9 detonation flow W F
    AttackEvent(9, false, std::chrono::milliseconds(1200), std::chrono::milliseconds(9000),
            {0, 0, static_cast<unsigned char>(0x1A), 9, 0, 0, 0, 0}, {0, 0, 0, 0}),

    // Attack 10 something dumb S RMB
    AttackEvent(10, true, std::chrono::milliseconds(1200), std::chrono::milliseconds(10000), {0, 0, 22, 0, 0, 0, 0, 0}, {2, 0, 0, 0}),

    // Attack 11
    AttackEvent(11, false, std::chrono::milliseconds(0), std::chrono::milliseconds(0), {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0}),

    // Attack 12
    AttackEvent(12, false, std::chrono::milliseconds(0), std::chrono::milliseconds(0), {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0})
};

std::array<AttackEvent, 12> guardianSuccAttacks = {
    // Attack 1 Shift E {SDLK_e, 0x08}
    AttackEvent(1, true, std::chrono::milliseconds(1500), std::chrono::milliseconds(180000),
            {2, 0, static_cast<unsigned char>(0x08), 0, 0, 0, 0, 0}, {0, 0, 0, 0}),

    // Attack 2 A RMB {SDLK_a, 0x04}
    AttackEvent(2, true, std::chrono::milliseconds(1500), std::chrono::milliseconds(6000),
            {0, 0, static_cast<unsigned char>(0x04), 0, 0, 0, 0, 0}, {2, 0, 0, 0}),

    // Attack 3 D RMB {SDLK_d, 0x07}
    AttackEvent(3, true, std::chrono::milliseconds(1500), std::chrono::milliseconds(6000),
            {0, 0, static_cast<unsigned char>(0x07), 0, 0, 0, 0, 0}, {2, 0, 0, 0}),

    // Attack 4 Shift+F {SDLK_f, 0x09}
    AttackEvent(4, true, std::chrono::milliseconds(1500), std::chrono::milliseconds(7000),
            {2, 0, static_cast<unsigned char>(0x09), 0, 0, 0, 0, 0}, {0, 0, 0, 0}),

    // Attack 5 Space {SDLK_SPACE, 0x2C}
    AttackEvent(5, true, std::chrono::milliseconds(1500), std::chrono::milliseconds(7000),
            {0, 0, static_cast<unsigned char>(0x2C), 0, 0, 0, 0, 0}, {0, 0, 0, 0}),
    // Attack 6 Shift RMB
    AttackEvent(6, true, std::chrono::milliseconds(2200), std::chrono::milliseconds(7000),
            {2, 0, 0, 0, 0, 0, 0, 0}, {2, 0, 0, 0}),

    // Attack 7 LMB
    AttackEvent(7, true, std::chrono::milliseconds(1500), std::chrono::milliseconds(7000),
            {0, 0, 0, 0, 0, 0, 0, 0}, {1, 0, 0, 0}),

    // Attack 8 S+F {SDLK_s, 0x16} {SDLK_f, 0x09}
    AttackEvent(8, true, std::chrono::milliseconds(1450), std::chrono::milliseconds(12000),
            {0, 0, static_cast<unsigned char>(0x16), static_cast<unsigned char>(0x09), 0, 0, 0, 0}, {0, 0, 0, 0}),

    // Attack 9 Shift Q RMB {SDLK_q, 0x14}
    AttackEvent(9, true, std::chrono::milliseconds(2000), std::chrono::milliseconds(9000),
            {2, 0, static_cast<unsigned char>(0x14), 0, 0, 0, 0, 0}, {2, 0, 0, 0}),

    // Attack 10 S LMB {SDLK_s, 0x16}
    AttackEvent(10, true, std::chrono::milliseconds(1500), std::chrono::milliseconds(6000),
            {0, 0, static_cast<unsigned char>(0x16), 0, 0, 0, 0, 0}, {1, 0, 0, 0}),

    // Attack 11 LMB RMB
    AttackEvent(11, true, std::chrono::milliseconds(2000), std::chrono::milliseconds(7000),
            {0, 0, 0, 0, 0, 0, 0, 0}, {3, 0, 0, 0}),
    // Attack 12
    AttackEvent(12, true, std::chrono::milliseconds(0), std::chrono::milliseconds(0),
            {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0}),
};

std::array<AttackEvent, 12> guardianAwakeAttacks = {
    // Attack 1 Omua's Blessing Shift E
    AttackEvent(1, true, std::chrono::milliseconds(1200), std::chrono::milliseconds(180000),
            {2, 0, static_cast<unsigned char>(0x08), 0, 0, 0, 0, 0}, {0, 0, 0, 0}),

    // Attack 2 Fireborn Rupture #2
    AttackEvent(2, true, std::chrono::milliseconds(1200), std::chrono::milliseconds(4000),
            {0, 0, static_cast<unsigned char>(0x1F), 0, 0, 0, 0, 0}, {0, 0, 0, 0}),

    // Attack 3 Glorious Advance S LMB only need 1 hit.
    AttackEvent(3, true, std::chrono::milliseconds(400), std::chrono::milliseconds(5000),
            {0, 0, static_cast<unsigned char>(0x16), 0, 0, 0, 0, 0}, {1, 0, 0, 0}),

    // Attack 4 Cleansing Flame Shift F
    AttackEvent(4, true, std::chrono::milliseconds(1800), std::chrono::milliseconds(6000),
            {2, 0, static_cast<unsigned char>(0x09), 0, 0, 0, 0, 0}, {0, 0, 0, 0}),

    // Attack 5 Dragons Maw Shift LMB
    AttackEvent(5, true, std::chrono::milliseconds(400), std::chrono::milliseconds(10000),
            {2, 0, 0, 0, 0, 0, 0, 0}, {1, 0, 0, 0}),

    // Attack 6 Flow Howling Light LMB
    AttackEvent(6, true, std::chrono::milliseconds(700), std::chrono::milliseconds(1000),
            {0, 0, 0, 0, 0, 0, 0, 0}, {1, 0, 0, 0}),

    // Attack 7 Fireborn Rupture Space ?
    AttackEvent(7, true, std::chrono::milliseconds(800), std::chrono::milliseconds(4000),
            {0, 0, static_cast<unsigned char>(0x2C), 0, 0, 0, 0, 0}, {0, 0, 0, 0}),

    // Attack 8 God Incinerator  Shift Q
    AttackEvent(8, true, std::chrono::milliseconds(1200), std::chrono::milliseconds(8000),
            {2, 0, static_cast<unsigned char>(0x14), 0, 0, 0, 0, 0}, {0, 0, 0, 0}),

    // Attack 9 Searing Fang Shift RMB
    AttackEvent(9, true, std::chrono::milliseconds(1400), std::chrono::milliseconds(7000),
            {2, 0, 0, 0, 0, 0, 0, 0}, {2, 0, 0, 0}),

    // Attack 10 Flow: Suppress RMB
    AttackEvent(10, true, std::chrono::milliseconds(1200), std::chrono::milliseconds(1000),
            {0, 0, 0, 0, 0, 0, 0, 0}, {2, 0, 0, 0}),

    // Attack 11 Scornful Slash S RMB
    AttackEvent(11, true, std::chrono::milliseconds(2000), std::chrono::milliseconds(5000),
            {0, 0, static_cast<unsigned char>(0x16), 0, 0, 0, 0, 0}, {2, 0, 0, 0}),

    // Attack 12
    AttackEvent(12, false, std::chrono::milliseconds(0), std::chrono::milliseconds(0),
            {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0})
};

std::array<AttackEvent, 12> otherAttacks = {
    // Attack 1
    AttackEvent(1, true, std::chrono::milliseconds(1100), std::chrono::milliseconds(7000), {0, 0, 0, 0, 0, 0, 0, 0}, {2, 0, 0, 0}),

    // Attack 2
    AttackEvent(2, true, std::chrono::milliseconds(1000), std::chrono::milliseconds(7000), {0, 0, 0, 0, 0, 0, 0, 0}, {3, 0, 0, 0}),

    // Attack 3
    AttackEvent(3, true, std::chrono::milliseconds(1000), std::chrono::milliseconds(6000), {0, 0, 22, 0, 0, 0, 0, 0}, {2, 0, 0, 0}),

    // Attack 4
    AttackEvent(4, true, std::chrono::milliseconds(1000), std::chrono::milliseconds(10000), {0, 0, 0, 0, 0, 0, 0, 0}, {2, 0, 0, 0}),

    // Attack 5
    AttackEvent(5, true, std::chrono::milliseconds(800), std::chrono::milliseconds(7000), {2, 0, 0, 0, 0, 0, 0, 0}, {2, 0, 0, 0}),

    // Attack 6
    AttackEvent(6, true, std::chrono::milliseconds(600), std::chrono::milliseconds(12000), {0, 0, 0, 0, 0, 0, 0, 0}, {2, 0, 0, 0}),

    // Attack 7
    AttackEvent(7, true, std::chrono::milliseconds(1000), std::chrono::milliseconds(6000), {0, 0, 26, 0, 0, 0, 0, 0}, {2, 0, 0, 0}),

    // Attack 8
    AttackEvent(8, true, std::chrono::milliseconds(1000), std::chrono::milliseconds(4000), {0, 0, 9, 0, 0, 0, 0, 0}, {0, 0, 0, 0}),

    // Attack 9
    AttackEvent(9, true, std::chrono::milliseconds(1000), std::chrono::milliseconds(10000), {2, 0, 0, 0, 0, 0, 0, 0}, {1, 0, 0, 0}),

    // Attack 10
    AttackEvent(10, true, std::chrono::milliseconds(1000), std::chrono::milliseconds(10000), {0, 0, 0, 0, 0, 0, 0, 0}, {1, 0, 0, 0}),

    // Attack 11
    AttackEvent(11, false, std::chrono::milliseconds(0), std::chrono::milliseconds(0), {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0}),

    // Attack 12
    AttackEvent(12, false, std::chrono::milliseconds(0), std::chrono::milliseconds(0), {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0})
};

std::string convertKeysToString(const std::array<unsigned char, 8>& keys) {
    std::stringstream result;
    for (unsigned char key : keys) {
        result << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(key) << " ";
    }
    return result.str();
}

std::string convertMouseButtonsToString(const std::array<unsigned char, 4>& mouseButtons) {
    std::string result;
    for (unsigned char button : mouseButtons) {
        result += std::to_string(button) + " ";
    }
    return result;
}

std::array<unsigned char, 8> keyStringToKey(const std::string& keyStr) {
    std::array<unsigned char, 8> result = {0};
    std::istringstream keyStream(keyStr);
    for (int i = 0; i < 8 && keyStream; ++i) {
        unsigned int temp;
        keyStream >> std::hex >> temp; // Use std::hex to interpret the string as hexadecimal
        result[i] = static_cast<unsigned char>(temp);
    }
    return result;
}

std::array<unsigned char, 4> mouseStringToMouse(const std::string& mouseStr) {
    std::array<unsigned char, 4> result = {0};
    std::istringstream mouseStream(mouseStr);
    for (int i = 0; i < 4 && mouseStream; ++i) {
        int temp;
        mouseStream >> temp;
        result[i] = static_cast<unsigned char>(temp);
    }
    return result;
}

// Function to load variables based on selected character
void loadVariablesFor(const std::string& character) {
    std::array<AttackEvent, 12>* selectedAttacks = nullptr;

    if (character == "Lahn") {
        Champ = "Lahn";
        selectedAttacks = &lahnAttacks;
    } else if (character == "Woosa") {
        Champ = "Woosa";
        selectedAttacks = &woosaAttacks;
    } else if (character == "Scholar") {
        Champ = "Scholar";
        selectedAttacks = &scholarAttacks;
    } else if (character == "Witch") {
        Champ = "Witch";
        selectedAttacks = &witchAttacks;
    } else if (character == "GuardianSucc") {
        Champ = "GuardianSucc";
        selectedAttacks = &guardianSuccAttacks;
    }
    else if (character == "GuardianAwake") {
        Champ = "GuardianAwake";
        selectedAttacks = &guardianAwakeAttacks;
    }

    if (selectedAttacks != nullptr) {
        for (int i = 0; i < 12; ++i) {
            attacks[i] = (*selectedAttacks)[i];

            // Update Text Input Boxes
            std::string prefix = "attack" + std::to_string(i + 1);
            textBoxes[prefix + "Duration"].text = std::to_string(attacks[i].duration.count());
            textBoxes[prefix + "SleepTimer"].text = std::to_string(attacks[i].sleepTimer.count());
            textBoxes[prefix + "Key"].text = convertKeysToString(attacks[i].key);
            textBoxes[prefix + "MouseButton"].text = convertMouseButtonsToString(attacks[i].mouseButton);
        }
    }
}

void setVariablesFromTextBoxes() {
    for (int i = 0; i < 12; ++i) {
        attacks[i] = AttackEvent();
        attacks[i].id = i + 1;
        attacks[i].ready = true; // This ... is wrong?, should not be set to True, but read from the variables table

        std::string prefix = "attack" + std::to_string(i + 1);

        // Duration
        int durationMs = std::stoi(textBoxes[prefix + "Duration"].text);
        attacks[i].duration = std::chrono::milliseconds(durationMs);

        // Sleep Timer
        int sleepTimerMs = std::stoi(textBoxes[prefix + "SleepTimer"].text);
        attacks[i].sleepTimer = std::chrono::milliseconds(sleepTimerMs);

        // Key
        attacks[i].key = keyStringToKey(textBoxes[prefix + "Key"].text);

        // MouseButton
        attacks[i].mouseButton = mouseStringToMouse(textBoxes[prefix + "MouseButton"].text);
    }
}

// Open the file at the start of the program
bool openFile() {
    #ifndef _WIN32
    hidFileKeyboard.open("/dev/hidg0", std::ios::binary | std::ios::out);
    if (!hidFileKeyboard) {
        std::cerr << "Failed to open keyboard file." << std::endl;
        return false;
    }

    hidFileMouse.open("/dev/hidg1", std::ios::binary | std::ios::out);
    if (!hidFileMouse) {
        std::cerr << "Failed to open mouse file." << std::endl;
        return false;
    }
    #else
    std::cerr << "Note: openFile() functionality is not required on Windows." << std::endl;
    // Windows specific code or handling can be added here if necessary.
    #endif

    return true;
}

// Close the file at the end of the program
void closeFile() {
    if (hidFileKeyboard.is_open()) {
        hidFileKeyboard.close();
    }
}

void prepareKeyboardBuffer() {
    finalKeyboardBuffer[0] = keyboardModifiers;

    for (size_t i = 0; i < keyBufferSize; i++) {
        finalKeyboardBuffer[i + 1] = keyboardBuffer[i];
    }
}

void displayBufferContents(SDL_Renderer* renderer, TTF_Font* font) {
    SDL_Color textColor = {255, 255, 255, 255};
    int textWidth, textHeight;

    // Final Mouse Buffer
    std::string mouseBufferText = "Final Mouse Buffer: ";
    for (size_t i = 0; i < mouseBufferSize; i++) {
        std::stringstream ss;
        ss << "0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(finalMouseBuffer[i]);
        mouseBufferText += ss.str() + " ";
    }

    SDL_Surface* textSurface = TTF_RenderText_Solid(font, mouseBufferText.c_str(), textColor);
    if (!textSurface) {
        std::cerr << "TTF_RenderText_Solid Error: " << TTF_GetError() << std::endl;
        return;
    }

    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (!textTexture) {
        std::cerr << "SDL_CreateTextureFromSurface Error: " << SDL_GetError() << std::endl;
        SDL_FreeSurface(textSurface);
        return;
    }

    SDL_FreeSurface(textSurface);

    SDL_QueryTexture(textTexture, nullptr, nullptr, &textWidth, &textHeight);
    SDL_Rect textRect = {TEXT_OFFSET, TEXT_OFFSET, textWidth, textHeight};

    SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);
    SDL_DestroyTexture(textTexture);


    // Final Keyboard Buffer
    std::string finalKeyboardBufferText = "Final Keyboard Buffer: ";
    for (size_t i = 0; i < 8; i++) {
        std::stringstream ss3;
        ss3 << "0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(finalKeyboardBuffer[i]);
        finalKeyboardBufferText += ss3.str() + " ";
    }

    textSurface = TTF_RenderText_Solid(font, finalKeyboardBufferText.c_str(), textColor);
    if (!textSurface) {
        std::cerr << "TTF_RenderText_Solid Error: " << TTF_GetError() << std::endl;
        return;
    }

    textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (!textTexture) {
        std::cerr << "SDL_CreateTextureFromSurface Error: " << SDL_GetError() << std::endl;
        SDL_FreeSurface(textSurface);
        return;
    }

    SDL_FreeSurface(textSurface);

    SDL_QueryTexture(textTexture, nullptr, nullptr, &textWidth, &textHeight);
    textRect = {TEXT_OFFSET, TEXT_OFFSET + TEXT_SPACING, textWidth, textHeight};

    SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);
    SDL_DestroyTexture(textTexture);
}

void addToBuffer(uint8_t code, uint8_t buffer[], uint8_t bufferSize) {
    // Check if the buffer is full
    bool isBufferFull = true;
    for (int i = 1; i < bufferSize; i++) {  // Start from index 1 to exclude the first byte
        if (buffer[i] == 0) {
            isBufferFull = false;
            break;
        }
    }

    // Check if code is already present in the buffer
    for (int i = 0; i < bufferSize; i++) {
        if (buffer[i] == code) {
            return;  // Code is already present, exit the function
        }
    }

    // Add the code to the buffer if it's not full and code is not already present
    if (!isBufferFull) {
        for (int i = 1; i < bufferSize; i++) {  // Start from index 1 to exclude the first byte
            if (buffer[i] == 0) {
                buffer[i] = code;
                break;
            }
        }
    }
}

void removeFromBuffer(unsigned char buffer[], int size, unsigned char element) {
    int foundIndex = -1;

    // Find the index of the element in the buffer
    for (int i = 0; i < size; i++) {
        if (buffer[i] == element) {
            foundIndex = i;
            break;
        }
    }

    if (foundIndex != -1) {
        // Shift the buffer down starting from the found index
        for (int i = foundIndex; i < size - 1; i++) {
            buffer[i] = buffer[i + 1];
        }

        // Add an empty byte (0x00) at the end
        buffer[size - 1] = 0x00;
    }
}

void addFromBufferMouse(unsigned char button) {
    if (button == 0x01) {
        finalMouseBuffer[0] |= 0x01; // Set the button bit
    } else if (button == 0x02) {
        finalMouseBuffer[0] |= 0x02; // Set the button bit
    } else if (button == 0x04) {
        finalMouseBuffer[0] |= 0x04; // Set the button bit
    }
}

void removeFromBufferMouse(unsigned char button) {
    if (button == 0x01) {
        finalMouseBuffer[0] &= ~0x01; // Clear the button bit
    } else if (button == 0x02) {
        finalMouseBuffer[0] &= ~0x02; // Clear the button bit
    } else if (button == 0x04) {
        finalMouseBuffer[0] &= ~0x04; // Clear the button bit
    }
}

// Keyboard/Mouse HID Functions
void write_report_k(const unsigned char* finalKeyboardBuffer) {
    if (!isRunning) return;  // Check if the start button is pressed

    #ifndef _WIN32
    if (hidFileKeyboard.is_open()) {
        hidFileKeyboard.write(reinterpret_cast<const char*>(finalKeyboardBuffer), 8);
        hidFileKeyboard.flush(); // Only perform HID write on Linux
    }
    #endif

    // Debugging output
    std::cout << "Keyboard Bytes written: ";
    for (int i = 0; i < 8; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(finalKeyboardBuffer[i]) << " ";
    }
    std::cout << std::endl;

}

void write_report_m(const unsigned char* finalMouseBuffer) {
    if (!isRunning) return;  // Check if the start button is pressed

    #ifndef _WIN32
    if (hidFileMouse.is_open()) {
        hidFileMouse.write(reinterpret_cast<const char*>(finalMouseBuffer), mouseBufferSize);
        hidFileMouse.flush(); // Only perform HID write on Linux
    }
    #endif

    // Debugging output
    std::cout << "Mouse Bytes written: ";
    for (size_t i = 0; i < mouseBufferSize; ++i) {
        std::cout << static_cast<int>(finalMouseBuffer[i]) << " ";
    }
    std::cout << std::endl;
}

// Global atomic flag to indicate the state of ZBuff
std::atomic<bool> ZBuffActive{false};
double ZBuffPercent = 0.8; // Represents 80%
unsigned char ZBuffHexKey[8] = {0, 0, 0x1d, 0, 0, 0, 0, 0}; // Empty Keyboard

void ZBuffFunction() {
    if (ZBuffActive.load()) {
        std::cout << "ZBuff is already active. Exiting function to avoid resetting timer.\n";
        return;
    }

    std::cout << "Activating ZBuff...\n";
    write_report_k(ZBuffHexKey);
    ZBuffActive = true;

    // Simulate ZBuff duration
    std::this_thread::sleep_for(std::chrono::seconds(60));

    ZBuffActive = false;
    std::cout << "ZBuff has expired.\n";
}

std::thread ZBuffThread; // Global thread object to manage ZBuff timer

// Global variable to track the last attempt time
auto lastAttemptTime = std::chrono::steady_clock::now();

void ActivateZBuff() {
    using namespace std::chrono;

    auto now = steady_clock::now();
    // Cooldown period of 1 seconds
    auto cooldownPeriod = seconds(1);

    // Check if ZBuff is already active or if a thread is running
    if (!ZBuffActive && !ZBuffThread.joinable()) {
        ZBuffThread = std::thread(ZBuffFunction);
        ZBuffThread.detach(); // Detach the new thread to let it run independently
        lastAttemptTime = now; // Update the last attempt time
    } else {
        if (duration_cast<seconds>(now - lastAttemptTime) >= cooldownPeriod) {
            std::cout << "Attempt to activate ZBuff ignored (already active or thread is running).\n";
            lastAttemptTime = now; // Reset the last attempt time to throttle messages
        }
    }
}


void attackFunction(AttackEvent& attack, std::atomic<bool>& isMacroActive) {
    if (attack.ready) {
        attack.ready = false;

        // Simulate pressing of keys
        if (attack.key[0] != 0 || attack.key[2] != 0 || (attack.key[3] != 0 && isMacroActive)) {
            write_report_k(attack.key.data());  // Key down event
        }

        // Simulate pressing of mouse buttons
        if (attack.mouseButton[0] != 0 && isMacroActive) {
            if (attack.key[0] && attack.key[2] == 0 && attack.key[3] == 0 && isMacroActive) {  // No keyboard in combo
                if (attack.mouseButton[0] == 1 && isMacroActive) {  // LMB
                    write_report_m(mouseButton.LMB.data());
                } else if (attack.mouseButton[0] == 2 && isMacroActive) {  // RMB
                    write_report_m(mouseButton.RMB.data());
                } else if (attack.mouseButton[0] == 3 && isMacroActive) {  // Both LMB and RMB
                    write_report_m(mouseButton.LMB.data());
                    if (isMacroActive ) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(simultaneousPressDistr(generator)));  // Tiny 2-5 button pressed delay (simulated human behavior)
                    if (isMacroActive ) {
                        write_report_m(mouseButton.Both.data());
                        }
                    }
                }
            } else {  // Slight pause for keyboard first
                if (isMacroActive ) {
                std::this_thread::sleep_for(std::chrono::milliseconds(simultaneousPressDistr(generator)));  // Tiny 2-5 button pressed delay (simulated human behavior)
                }
                if (attack.mouseButton[0] == 1 && isMacroActive) {   // LMB
                    write_report_m(mouseButton.LMB.data());
                } else if (attack.mouseButton[0] == 2 && isMacroActive) { // RMB
                    write_report_m(mouseButton.RMB.data());
                } else if (attack.mouseButton[0] == 3 && isMacroActive) {  // Both LMB and RMB
                    write_report_m(mouseButton.LMB.data());
                    if (isMacroActive) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(simultaneousPressDistr(generator)));  // Tiny 2-5 button pressed delay (simulated human behavior)
                    }
                    if (isMacroActive) {
                    write_report_m(mouseButton.Both.data());
                    }
                }
            }
        }

        // Resetting the attack for future use
        std::thread([&attack]() {
            std::this_thread::sleep_for(attack.sleepTimer);
            attack.ready = true;
            std::cout << "Attack " << std::to_string(attack.id) << " is ready again!\n"; // Explicitly convert id to string
        }).detach();
    }
}

std::condition_variable macroCondition;
std::mutex cv_m;

void invokeAttack(AttackEvent& attack, std::atomic<bool>& isMacroActive) {
    std::thread t([&] {
        attackFunction(attack, isMacroActive);
    });
    t.join();
}

void manualCooldown(size_t attackNumber) {
    // Set attack as not ready immediately
    attacks[attackNumber].ready = false;
    // Start a detached thread to handle the sleep timer and reset attack status
    std::thread([attackNumber]() {
        std::cout << "Cooldown started for attack " << attackNumber << std::endl;

        // Corrected sleep_for usage with proper syntax to access sleepTimer
        std::this_thread::sleep_for(attacks[attackNumber].sleepTimer);

        attacks[attackNumber].ready = true;
        std::cout << "Attack " << attackNumber << " is ready again!" << std::endl;
    }).detach(); // Detaching the thread allows it to run independently
}

int attackSpeedBuffCounter = 1;
int attackRound = 1;

void macroFunction(std::array<AttackEvent, 12>& attacks) {
    std::unique_lock<std::mutex> lk(cv_m);

    while (isMacroActive) {
        if (Champ == "Woosa") {
            int startIdx = 2; // Default start index for Woosa is 2

            // Determine the starting index based on the readiness of the first two attacks
            if (attacks[0].ready) {
                startIdx = 0;
            } else if (attacks[1].ready) {
                startIdx = 1;
            }

            for (int i = startIdx; i < 12; ++i) {
                if (!isMacroActive) break; // Check to exit loop if macro is no longer active

                // Skip logic for Woosa
                if (startIdx == 0 && i == 1) continue;

                if (attacks[i].ready) {
                    if (attacks[i].duration == std::chrono::milliseconds(0) && attacks[i].sleepTimer == std::chrono::milliseconds(0)) {
                        continue;
                    }
                    invokeAttack(attacks[i], isMacroActive);

                    int totalIterations = attacks[i].duration / std::chrono::milliseconds(50);
                    for (int j = 0; j < totalIterations && isMacroActive; ++j) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(50));
                        if (!isMacroActive) break;
                    }

                    if (isMacroActive) {
                        if (attacks[i].key[0] != 0 || attacks[i].key[2] != 0) {
                            std::this_thread::sleep_for(std::chrono::milliseconds(simultaneousPressDistr(generator)));
                            write_report_k(emptyKey);
                        }
                        if (attacks[i].mouseButton[0] != 0) {
                            std::this_thread::sleep_for(std::chrono::milliseconds(simultaneousPressDistr(generator)));
                            write_report_m(emptyMouse);
                        }
                    }
                }
            }
        }

        if (Champ == "Witch") {
            for (int i = 0; i < 12; ++i) {
                if (!isMacroActive) break;

                if (attacks[i].ready) {
                    if (attacks[i].duration == std::chrono::milliseconds(0) && attacks[i].sleepTimer == std::chrono::milliseconds(0)) {
                        continue; // Skip if no duration and no sleep timer
                    }

                    switch (i) {
                        // Stacked Case Normal Code 1st
                        case 0: // Index 0 (1st attack) is normal
                        case 3: // Index 3 (4th attack) is normal
                        case 10: // Index 10 (11th attack) is normal
                        case 11: { // Index 11 (12th attack) is normal
                            invokeAttack(attacks[i], isMacroActive);

                            int totalIterations = attacks[i].duration / std::chrono::milliseconds(50);
                            for (int j = 0; j < totalIterations && isMacroActive; ++j) {
                                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                                if (!isMacroActive) break;
                            }

                            if (isMacroActive) {
                                if (attacks[i].key[0] != 0 || attacks[i].key[2] != 0) {
                                    std::this_thread::sleep_for(std::chrono::milliseconds(simultaneousPressDistr(generator)));
                                    write_report_k(emptyKey);
                                }
                                if (attacks[i].mouseButton[0] != 0) {
                                    std::this_thread::sleep_for(std::chrono::milliseconds(simultaneousPressDistr(generator)));
                                    write_report_m(emptyMouse);
                                }
                            }
                            break;
                        }
                        case 1: // Index 1 (2nd attack), skipped
                        case 4: // Index 4 (5th attack), skipped
                        case 6: { // Index 6 (7th attack), skipped
                            // Intentionally left empty to skip
                            break;
                        }
                        case 2: {// Index 2 (3rd attack), special handling
                            // Directly check if 3rd attack is ready; no need to check sleepTimer
                            if (attacks[2].ready) {
                                // Execute the 3rd attack if ready
                                invokeAttack(attacks[2], isMacroActive);

                                int totalIterations = attacks[2].duration / std::chrono::milliseconds(50);
                                for (int j = 0; j < totalIterations && isMacroActive; ++j) {
                                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                                    if (!isMacroActive) break;
                                }

                                if (isMacroActive) {
                                    if (attacks[2].key[0] != 0 || attacks[2].key[2] != 0) {
                                        std::this_thread::sleep_for(std::chrono::milliseconds(simultaneousPressDistr(generator)));
                                        write_report_k(emptyKey);
                                    }
                                    if (attacks[2].mouseButton[0] != 0) {
                                        std::this_thread::sleep_for(std::chrono::milliseconds(simultaneousPressDistr(generator)));
                                        write_report_m(emptyMouse);
                                    }
                                }
                            } else {
                                // If 3rd attack is not ready, check if the 2nd attack is ready as an alternative
                                if (attacks[1].ready) {
                                    // If alternative 2nd attack is ready, execute it
                                    invokeAttack(attacks[1], isMacroActive);

                                    int totalIterations = attacks[1].duration / std::chrono::milliseconds(50);
                                    for (int j = 0; j < totalIterations && isMacroActive; ++j) {
                                        std::this_thread::sleep_for(std::chrono::milliseconds(50));
                                        if (!isMacroActive) break;
                                    }

                                    if (isMacroActive) {
                                        if (attacks[1].key[0] != 0 || attacks[1].key[2] != 0) {
                                            std::this_thread::sleep_for(std::chrono::milliseconds(simultaneousPressDistr(generator)));
                                            write_report_k(emptyKey);
                                        }
                                        if (attacks[1].mouseButton[0] != 0) {
                                            std::this_thread::sleep_for(std::chrono::milliseconds(simultaneousPressDistr(generator)));
                                            write_report_m(emptyMouse);
                                        }
                                    }
                                }
                                // If neither 3rd nor 2nd attacks are ready, nothing happens
                            }
                            break;
                            }
                            case 5: { // Special logic for Index 5 (6th attack)
                            // Always execute attack 7 (index 6) first if ready
                            if (attacks[6].ready) {
                                invokeAttack(attacks[6], isMacroActive);

                                int totalIterations7 = attacks[6].duration / std::chrono::milliseconds(50);
                                for (int j = 0; j < totalIterations7 && isMacroActive; ++j) {
                                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                                    if (!isMacroActive) break;
                                }

                                if (isMacroActive) {
                                    if (attacks[6].key[0] != 0 || attacks[6].key[2] != 0) {
                                        std::this_thread::sleep_for(std::chrono::milliseconds(simultaneousPressDistr(generator)));
                                        write_report_k(emptyKey);
                                    }
                                    if (attacks[6].mouseButton[0] != 0) {
                                        std::this_thread::sleep_for(std::chrono::milliseconds(simultaneousPressDistr(generator)));
                                        write_report_m(emptyMouse);
                                    }
                                }
                            }

                            if (!isMacroActive) break;

                            // Decision to execute attack 5 or attack 6 based on their readiness
                            int chosenAttackIndex = -1; // Initialize with an invalid index
                            if (attacks[4].ready) { // Attack 5 is at index 4
                                chosenAttackIndex = 4;
                            } else if (attacks[5].ready) { // Attack 6 is at index 5
                                chosenAttackIndex = 5;
                            }

                            if (chosenAttackIndex != -1) {
                                invokeAttack(attacks[chosenAttackIndex], isMacroActive);

                                int totalIterations = attacks[chosenAttackIndex].duration / std::chrono::milliseconds(50);
                                for (int j = 0; j < totalIterations && isMacroActive; ++j) {
                                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                                    if (!isMacroActive) break;
                                }

                                if (isMacroActive) {
                                    if (attacks[chosenAttackIndex].key[0] != 0 || attacks[chosenAttackIndex].key[2] != 0) {
                                        std::this_thread::sleep_for(std::chrono::milliseconds(simultaneousPressDistr(generator)));
                                        write_report_k(emptyKey);
                                    }
                                    if (attacks[chosenAttackIndex].mouseButton[0] != 0) {
                                        std::this_thread::sleep_for(std::chrono::milliseconds(simultaneousPressDistr(generator)));
                                        write_report_m(emptyMouse);
                                    }
                                }
                            }
                            break;
                        }
                        case 7: // Index 7 (8th attack) does 7 first
                        case 8: // Index 8 (9th attack) does 7 first
                        case 9: {// Index 9 (10th attack) does 7 first
                            // Assuming you meant to invoke logic for attack 7 before the current attack
                        if (attacks[6].ready) {
                            if (attacks[6].duration == std::chrono::milliseconds(0) && attacks[6].sleepTimer == std::chrono::milliseconds(0)) {
                                continue;
                            }
                            // Check if 'i' is equal to 8 or 9 and if attack 2 (index 1) is ready
                            if ((i == 8 || i == 9) && attacks[2].ready) {
                                continue; // Skip execution of attack 7
                            }
                            invokeAttack(attacks[6], isMacroActive);

                            int totalIterations = attacks[6].duration / std::chrono::milliseconds(50);
                            for (int j = 0; j < totalIterations && isMacroActive; ++j) {
                                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                                if (!isMacroActive) break;
                            }

                            if (isMacroActive) {
                                if (attacks[6].key[0] != 0 || attacks[6].key[2] != 0) {
                                    std::this_thread::sleep_for(std::chrono::milliseconds(simultaneousPressDistr(generator)));
                                    write_report_k(emptyKey);
                                }
                                if (attacks[6].mouseButton[0] != 0) {
                                    std::this_thread::sleep_for(std::chrono::milliseconds(simultaneousPressDistr(generator)));
                                    write_report_m(emptyMouse);
                                }
                            }
                        }
                        // check for macro after atk 7 (index6),
                        if (!isMacroActive) break;
                        if (attacks[i].ready) {
                            if (attacks[i].duration == std::chrono::milliseconds(0) && attacks[i].sleepTimer == std::chrono::milliseconds(0)) {
                                continue;
                            }
                            if (attacks[2].ready) {
                                continue; // Skip execution of attack if combo ready to start
                            }
                            invokeAttack(attacks[i], isMacroActive);

                            int totalIterations = attacks[i].duration / std::chrono::milliseconds(50);
                            for (int j = 0; j < totalIterations && isMacroActive; ++j) {
                                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                                if (!isMacroActive) break;
                            }

                            if (isMacroActive) {
                                if (attacks[i].key[0] != 0 || attacks[i].key[2] != 0) {
                                    std::this_thread::sleep_for(std::chrono::milliseconds(simultaneousPressDistr(generator)));
                                    write_report_k(emptyKey);
                                }
                                if (attacks[i].mouseButton[0] != 0) {
                                    std::this_thread::sleep_for(std::chrono::milliseconds(simultaneousPressDistr(generator)));
                                    write_report_m(emptyMouse);
                                }
                            }
                        }
                            break;
                    }
                    }
                    if (!isMacroActive) break; // Check for active status after each attack execution
                }
            }
        }

        if (Champ == "GuardianSucc") { // Updated 2/19
            for (int i = 0; i < 6; ++i) {
                if (!isMacroActive) break;
                switch (i) {
                    // class buff
                    case 0: { // Class buff if not Z
                        if (attacks[8].ready) {
                        attackRound = 1;
                        }
                        if (attacks[i].ready && !ZBuffActive) { // Check if the attack is ready and ZBuff is not active
                            if (attacks[i].duration == std::chrono::milliseconds(0) && attacks[i].sleepTimer == std::chrono::milliseconds(0)) {
                                continue; // Skip if no duration and no sleep timer
                            }
                            invokeAttack(attacks[i], isMacroActive);

                            int totalIterations = attacks[i].duration.count() / 50; // Each iteration waits for 50 milliseconds

                            for (int j = 0; j < totalIterations && isMacroActive; ++j) {
                                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                                if (!isMacroActive) break;
                            }
                            if (isMacroActive) {
                                if (attacks[i].key[0] != 0 || attacks[i].key[2] != 0) {
                                    std::this_thread::sleep_for(std::chrono::milliseconds(simultaneousPressDistr(generator)));
                                    write_report_k(emptyKey);
                                }
                                if (attacks[i].mouseButton[0] != 0) {
                                    std::this_thread::sleep_for(std::chrono::milliseconds(simultaneousPressDistr(generator)));
                                    write_report_m(emptyMouse);
                                }
                            }
                            break;
                        }
                    }
                    case 1: { // A/D RMB Speed Buff if not Z
                        if (attacks[1].ready && !ZBuffActive) {
                            if (attacks[1].duration == std::chrono::milliseconds(0) && attacks[1].sleepTimer == std::chrono::milliseconds(0)) {
                                continue; // Skip if no duration and no sleep timer
                            }
                            if (attackSpeedBuffCounter == 1) {
                                invokeAttack(attacks[1], isMacroActive);
                                manualCooldown(2); // Attack 2 is on CD when attack 1 is on CD
                                attackSpeedBuffCounter = 2;
                            } else if (attackSpeedBuffCounter == 2) {
                                invokeAttack(attacks[2], isMacroActive);
                                manualCooldown(1); // Attack 2 is on CD when attack 1 is on CD
                                attackSpeedBuffCounter = 1;
                            }

                            int totalIterations;
                            totalIterations = attacks[i].duration.count() / 50; // Direct calculation if ZBuff is not active

                            for (int j = 0; j < totalIterations && isMacroActive; ++j) {
                                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                                    if (!isMacroActive) break;
                                }

                            if (isMacroActive) {
                                if (attacks[1].key[0] != 0 || attacks[1].key[2] != 0) {
                                    std::this_thread::sleep_for(std::chrono::milliseconds(simultaneousPressDistr(generator)));
                                    write_report_k(emptyKey);
                                }
                                if (attacks[1].mouseButton[0] != 0) {
                                    std::this_thread::sleep_for(std::chrono::milliseconds(simultaneousPressDistr(generator)));
                                    write_report_m(emptyMouse);
                                }
                            }
                        }
                        break;
                    }
                    case 2: { // Shift+F  and Space
                        if (attacks[3].ready && isMacroActive) {
                            if (!(attacks[3].duration == std::chrono::milliseconds(0) && attacks[3].sleepTimer == std::chrono::milliseconds(0))) {
                                invokeAttack(attacks[3], isMacroActive);

                                int totalIterations = ZBuffActive ?
                                    static_cast<int>(attacks[3].duration.count() * ZBuffPercent) / 50 :
                                    attacks[3].duration.count() / 50;

                                for (int j = 0; j < totalIterations && isMacroActive; ++j) {
                                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                                    if (!isMacroActive) break;
                                }

                                if (isMacroActive) {
                                    if (attacks[3].key[0] != 0 || attacks[3].key[2] != 0) {
                                        std::this_thread::sleep_for(std::chrono::milliseconds(simultaneousPressDistr(generator)));
                                        write_report_k(emptyKey);
                                    }
                                    if (attacks[3].mouseButton[0] != 0) {
                                        std::this_thread::sleep_for(std::chrono::milliseconds(simultaneousPressDistr(generator)));
                                        write_report_m(emptyMouse);
                                    }
                                }
                            }
                        }

                        // LMB Flow
                        if (attacks[4].ready && isMacroActive) {
                            if (!(attacks[4].duration == std::chrono::milliseconds(0) && attacks[4].sleepTimer == std::chrono::milliseconds(0))) {
                                invokeAttack(attacks[4], isMacroActive);

                                int totalIterations = ZBuffActive ?
                                    static_cast<int>(attacks[4].duration.count() * ZBuffPercent) / 50 :
                                    attacks[4].duration.count() / 50;

                                for (int j = 0; j < totalIterations && isMacroActive; ++j) {
                                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                                    if (!isMacroActive) break;
                                }

                                if (isMacroActive) {
                                    if (attacks[4].key[0] != 0 || attacks[4].key[2] != 0) {
                                        std::this_thread::sleep_for(std::chrono::milliseconds(simultaneousPressDistr(generator)));
                                        write_report_k(emptyKey);
                                    }
                                    if (attacks[4].mouseButton[0] != 0) {
                                        std::this_thread::sleep_for(std::chrono::milliseconds(simultaneousPressDistr(generator)));
                                        write_report_m(emptyMouse);
                                    }
                                }
                            }
                        }
                        break;
                    }
                    case 3: { // Shift RMB + Flow LMB
                        if (attacks[5].ready && isMacroActive) {
                            if (!(attacks[5].duration == std::chrono::milliseconds(0) && attacks[5].sleepTimer == std::chrono::milliseconds(0))) {
                                invokeAttack(attacks[5], isMacroActive);

                                int totalIterations = ZBuffActive ?
                                    static_cast<int>(attacks[5].duration.count() * ZBuffPercent) / 50 :
                                    attacks[5].duration.count() / 50;

                                for (int j = 0; j < totalIterations && isMacroActive; ++j) {
                                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                                    if (!isMacroActive) break;
                                }

                                if (isMacroActive) {
                                    if (attacks[5].key[0] != 0 || attacks[5].key[2] != 0) {
                                        std::this_thread::sleep_for(std::chrono::milliseconds(simultaneousPressDistr(generator)));
                                        write_report_k(emptyKey);
                                    }
                                    if (attacks[5].mouseButton[0] != 0) {
                                        std::this_thread::sleep_for(std::chrono::milliseconds(simultaneousPressDistr(generator)));
                                        write_report_m(emptyMouse);
                                    }
                                }
                            }
                        }

                        // LMB Flow
                        if (attacks[6].ready && isMacroActive) {
                            if (!(attacks[6].duration == std::chrono::milliseconds(0) && attacks[6].sleepTimer == std::chrono::milliseconds(0))) {
                                invokeAttack(attacks[6], isMacroActive);

                                int totalIterations = ZBuffActive ?
                                    static_cast<int>(attacks[6].duration.count() * ZBuffPercent) / 50 :
                                    attacks[6].duration.count() / 50;

                                for (int j = 0; j < totalIterations && isMacroActive; ++j) {
                                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                                    if (!isMacroActive) break;
                                }

                                if (isMacroActive) {
                                    if (attacks[6].key[0] != 0 || attacks[6].key[2] != 0) {
                                        std::this_thread::sleep_for(std::chrono::milliseconds(simultaneousPressDistr(generator)));
                                        write_report_k(emptyKey);
                                    }
                                    if (attacks[6].mouseButton[0] != 0) {
                                        std::this_thread::sleep_for(std::chrono::milliseconds(simultaneousPressDistr(generator)));
                                        write_report_m(emptyMouse);
                                    }
                                }
                            }
                        }
                        break;
                    }
                    case 4: {
                        if (attackRound == 1 && isMacroActive) {
                            if (attacks[7].ready) {
                                if (!(attacks[7].duration == std::chrono::milliseconds(0) && attacks[7].sleepTimer == std::chrono::milliseconds(0))) {
                                    invokeAttack(attacks[7], isMacroActive);

                                    int totalIterations = ZBuffActive ?
                                        static_cast<int>(attacks[7].duration.count() * ZBuffPercent) / 50 :
                                        attacks[7].duration.count() / 50;

                                    for (int j = 0; j < totalIterations && isMacroActive; ++j) {
                                        std::this_thread::sleep_for(std::chrono::milliseconds(50));
                                        if (!isMacroActive) break;
                                    }
                                    if (isMacroActive) {
                                        if (attacks[7].key[0] != 0 || attacks[7].key[2] != 0) {
                                            std::this_thread::sleep_for(std::chrono::milliseconds(simultaneousPressDistr(generator)));
                                            write_report_k(emptyKey);
                                        }
                                        if (attacks[7].mouseButton[0] != 0) {
                                            std::this_thread::sleep_for(std::chrono::milliseconds(simultaneousPressDistr(generator)));
                                            write_report_m(emptyMouse);
                                        }
                                    }
                                }
                            }
                        } else if (attackRound == 2 && isMacroActive) {
                            if (attacks[9].ready) {
                                if (!(attacks[9].duration == std::chrono::milliseconds(0) && attacks[9].sleepTimer == std::chrono::milliseconds(0))) {
                                    invokeAttack(attacks[9], isMacroActive);

                                    int totalIterations = ZBuffActive ?
                                        static_cast<int>(attacks[9].duration.count() * ZBuffPercent) / 50 :
                                        attacks[9].duration.count() / 50;

                                    for (int j = 0; j < totalIterations && isMacroActive; ++j) {
                                        std::this_thread::sleep_for(std::chrono::milliseconds(50));
                                        if (!isMacroActive) break;
                                    }
                                    if (isMacroActive) {
                                        if (attacks[9].key[0] != 0 || attacks[9].key[2] != 0) {
                                            std::this_thread::sleep_for(std::chrono::milliseconds(simultaneousPressDistr(generator)));
                                            write_report_k(emptyKey);
                                        }
                                        if (attacks[9].mouseButton[0] != 0) {
                                            std::this_thread::sleep_for(std::chrono::milliseconds(simultaneousPressDistr(generator)));
                                            write_report_m(emptyMouse);
                                        }
                                    }
                                }
                            }
                        }
                        break;
                    }
                    case 5: { // Execute attacks based on the round logic
                        if (attackRound == 1 && isMacroActive) {
                            if (attacks[8].ready) {
                                if (!(attacks[8].duration == std::chrono::milliseconds(0) && attacks[8].sleepTimer == std::chrono::milliseconds(0))) {
                                    invokeAttack(attacks[8], isMacroActive);
                                    attackRound = 2;

                                    int totalIterations = ZBuffActive ?
                                        static_cast<int>(attacks[8].duration.count() * ZBuffPercent) / 50 :
                                        attacks[8].duration.count() / 50;

                                    for (int j = 0; j < totalIterations && isMacroActive; ++j) {
                                        std::this_thread::sleep_for(std::chrono::milliseconds(50));
                                        if (!isMacroActive) break;
                                    }
                                    if (isMacroActive) {
                                        if (attacks[8].key[0] != 0 || attacks[8].key[2] != 0) {
                                            std::this_thread::sleep_for(std::chrono::milliseconds(simultaneousPressDistr(generator)));
                                            write_report_k(emptyKey);
                                        }
                                        if (attacks[8].mouseButton[0] != 0) {
                                            std::this_thread::sleep_for(std::chrono::milliseconds(simultaneousPressDistr(generator)));
                                            write_report_m(emptyMouse);
                                        }
                                    }
                                }
                            }
                        } else if (attackRound == 2 && isMacroActive) {
                            // Round 2 logic for attacks[11], assuming next in the sequence
                            if (attacks[10].ready) {
                                if (!(attacks[10].duration == std::chrono::milliseconds(0) && attacks[10].sleepTimer == std::chrono::milliseconds(0))) {
                                    invokeAttack(attacks[10], isMacroActive);
                                    attackRound = 1;

                                    int totalIterations = ZBuffActive ?
                                        static_cast<int>(attacks[10].duration.count() * ZBuffPercent) / 50 :
                                        attacks[10].duration.count() / 50;

                                    for (int j = 0; j < totalIterations && isMacroActive; ++j) {
                                        std::this_thread::sleep_for(std::chrono::milliseconds(50));
                                        if (!isMacroActive) break;
                                    }
                                    if (isMacroActive) {
                                        if (attacks[10].key[0] != 0 || attacks[10].key[2] != 0) {
                                            std::this_thread::sleep_for(std::chrono::milliseconds(simultaneousPressDistr(generator)));
                                            write_report_k(emptyKey);
                                        }
                                        if (attacks[10].mouseButton[0] != 0) {
                                            std::this_thread::sleep_for(std::chrono::milliseconds(simultaneousPressDistr(generator)));
                                            write_report_m(emptyMouse);
                                        }
                                    }
                                }
                            }
                        }
                        break;
                    }
                    if (!isMacroActive) break; // Check for active status after each attack execution
                }
            }
        }

        if (Champ == "GuardianAwake") {
            for (int i = 0; i < 12; ++i) {
                if (!isMacroActive) break;

                if (attacks[i].ready) {
                    if (attacks[i].duration == std::chrono::milliseconds(0) && attacks[i].sleepTimer == std::chrono::milliseconds(0)) {
                        continue; // Skip if no duration and no sleep timer
                    }

                    switch (i) {
                        case 0:
                        case 2:
                        case 3:
                        case 7:
                        case 10:
                        case 11: { // Normal attack pattern
                            invokeAttack(attacks[i], isMacroActive);

                            int totalIterations = attacks[i].duration / std::chrono::milliseconds(50);
                            for (int j = 0; j < totalIterations && isMacroActive; ++j) {
                                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                                if (!isMacroActive) break;
                            }

                            if (isMacroActive) {
                                if (attacks[i].key[0] != 0 || attacks[i].key[2] != 0) {
                                    std::this_thread::sleep_for(std::chrono::milliseconds(simultaneousPressDistr(generator)));
                                    write_report_k(emptyKey);
                                }
                                if (attacks[i].mouseButton[0] != 0) {
                                    std::this_thread::sleep_for(std::chrono::milliseconds(simultaneousPressDistr(generator)));
                                    write_report_m(emptyMouse);
                                }
                            }
                            break;
                        }

                        case 4:
                        case 8: {
                            // Intentionally left empty to skip
                            break;
                        }

                        case 1: {
                            if (attacks[1].ready && attacks[6].ready) {
                                invokeAttack(attacks[1], isMacroActive);

                                int totalIterations = attacks[1].duration / std::chrono::milliseconds(50);
                                for (int j = 0; j < totalIterations && isMacroActive; ++j) {
                                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                                    if (!isMacroActive) break;
                                }

                                if (isMacroActive) {
                                    if (attacks[1].key[0] != 0 || attacks[1].key[4] != 0) {
                                        std::this_thread::sleep_for(std::chrono::milliseconds(simultaneousPressDistr(generator)));
                                        write_report_k(emptyKey);
                                    }
                                    if (attacks[1].mouseButton[0] != 0) {
                                        std::this_thread::sleep_for(std::chrono::milliseconds(simultaneousPressDistr(generator)));
                                        write_report_m(emptyMouse);
                                    }
                                }
                            }
                            break;
                        }

                        case 6: {
                            if (attacks[1].ready && attacks[6].ready) {
                                invokeAttack(attacks[6], isMacroActive);

                                int totalIterations = attacks[1].duration / std::chrono::milliseconds(50);
                                for (int j = 0; j < totalIterations && isMacroActive; ++j) {
                                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                                    if (!isMacroActive) break;
                                }

                                if (isMacroActive) {
                                    if (attacks[6].key[0] != 0 || attacks[6].key[4] != 0) {
                                        std::this_thread::sleep_for(std::chrono::milliseconds(simultaneousPressDistr(generator)));
                                        write_report_k(emptyKey);
                                    }
                                    if (attacks[6].mouseButton[0] != 0) {
                                        std::this_thread::sleep_for(std::chrono::milliseconds(simultaneousPressDistr(generator)));
                                        write_report_m(emptyMouse);
                                    }
                                }
                            }
                            break;
                        }

                        case 5:
                        case 9: {// Dragon's Maw + Flow
                            if (attacks[i-1].ready) {
                                invokeAttack(attacks[i-1], isMacroActive);

                                int totalIterations = attacks[i-1].duration / std::chrono::milliseconds(50);
                                for (int j = 0; j < totalIterations && isMacroActive; ++j) {
                                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                                    if (!isMacroActive) break;
                                }

                                if (isMacroActive) {
                                    if (attacks[i-1].key[0] != 0 || attacks[i-1].key[4] != 0) {
                                        std::this_thread::sleep_for(std::chrono::milliseconds(simultaneousPressDistr(generator)));
                                        write_report_k(emptyKey);
                                    }
                                    if (attacks[i-1].mouseButton[0] != 0) {
                                        std::this_thread::sleep_for(std::chrono::milliseconds(simultaneousPressDistr(generator)));
                                        write_report_m(emptyMouse);
                                    }
                                }
                            }
                            if (isMacroActive) {
                                invokeAttack(attacks[i], isMacroActive);

                                int totalIterations = attacks[i].duration / std::chrono::milliseconds(50);
                                for (int j = 0; j < totalIterations && isMacroActive; ++j) {
                                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                                    if (!isMacroActive) break;
                                }

                                if (isMacroActive) {
                                    if (attacks[i].key[0] != 0 || attacks[i].key[4] != 0) {
                                        std::this_thread::sleep_for(std::chrono::milliseconds(simultaneousPressDistr(generator)));
                                        write_report_k(emptyKey);
                                    }
                                    if (attacks[i].mouseButton[0] != 0) {
                                        std::this_thread::sleep_for(std::chrono::milliseconds(simultaneousPressDistr(generator)));
                                        write_report_m(emptyMouse);
                                    }
                                }
                            }


                            break;
                            }
                    }
                    if (!isMacroActive) break; // Check for active status after each attack execution
                }
            }
        }

        else {
            // General case for other champs
            for (int i = 0; i < 12; ++i) {
                if (!isMacroActive) break;

                if (attacks[i].ready) {
                    if (attacks[i].duration == std::chrono::milliseconds(0) && attacks[i].sleepTimer == std::chrono::milliseconds(0)) {
                        continue;
                    }
                    invokeAttack(attacks[i], isMacroActive);

                    int totalIterations = attacks[i].duration / std::chrono::milliseconds(50);
                    for (int j = 0; j < totalIterations && isMacroActive; ++j) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(50));
                        if (!isMacroActive) break;
                    }

                    if (isMacroActive) {
                        if (attacks[i].key[0] != 0 || attacks[i].key[2] != 0) {
                            std::this_thread::sleep_for(std::chrono::milliseconds(simultaneousPressDistr(generator)));
                            write_report_k(emptyKey);
                        }
                        if (attacks[i].mouseButton[0] != 0) {
                            std::this_thread::sleep_for(std::chrono::milliseconds(simultaneousPressDistr(generator)));
                            write_report_m(emptyMouse);
                        }
                    }
                }
            }
        }

        if (!isMacroActive) {
                break; // Break out of the loop if isMacroActive is false
            }
    }
}

void renderCurrentChampLabel(SDL_Renderer* renderer, TTF_Font* font, const Label& label) {
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, label.text.c_str(), {255, 255, 255, 255}); // White color text
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

    // Create a local copy of SDL_Rect to modify
    SDL_Rect renderQuad = label.rect;
    renderQuad.w = textSurface->w;
    renderQuad.h = textSurface->h;

    // Render the text to the screen
    SDL_RenderCopy(renderer, textTexture, NULL, &renderQuad);

    // Clean up the surface and texture
    SDL_DestroyTexture(textTexture);
    SDL_FreeSurface(textSurface);
}

int main() {
    std::map<std::string, std::string> variablesMap = initializeVariablesMap();
    initializeAttacks();
        // Convert attack event data to strings and place them in the map
    for (int i = 0; i < 12; ++i) {
        variablesMap["attack" + std::to_string(i + 1) + "Duration"] = std::to_string(attacks[i].duration.count());
        variablesMap["attack" + std::to_string(i + 1) + "SleepTimer"] = std::to_string(attacks[i].sleepTimer.count());
        variablesMap["attack" + std::to_string(i + 1) + "Key"] = convertKeysToString(attacks[i].key);
        variablesMap["attack" + std::to_string(i + 1) + "MouseButton"] = convertMouseButtonsToString(attacks[i].mouseButton);
    }

    currentChampLabel.rect = {350, 10, 0, 0}; // Set the x, y position and initialize w, h to 0
    currentChampLabel.text = "Current Champ : None";

    // Open the file at the start of the program
    if (!openFile()) {
        return 1;
    }

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    if (TTF_Init() != 0) {
        std::cerr << "TTF_Init Error: " << TTF_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Input Buffer Display", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    if (window == nullptr) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr) {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    TTF_Font* font = TTF_OpenFont("Montserrat-Bold.ttf", 14);
    if (font == nullptr) {
        std::cerr << "TTF_OpenFont Error: " << TTF_GetError() << std::endl;
        font = TTF_OpenFont("arial.ttf", 14); // Load Arial as a fallback
        if (font == nullptr) {
            std::cerr << "TTF_OpenFont Error (Arial): " << TTF_GetError() << std::endl;
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            TTF_Quit();
            SDL_Quit();
            return 1;
        }
    }

    initializeTextBoxes(variablesMap);

    SDL_Color startStopColor = GREEN_COLOR;

    bool quit = false;

    SDL_Event event;
    std::thread macroThread; // ???

    while (!quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN) {
                unsigned char button = 0;
                if (event.button.button == SDL_BUTTON_LEFT) {
                    button |= 0x01;
                } else if (event.button.button == SDL_BUTTON_RIGHT) {
                    button |= 0x02;
                } else if (event.button.button == SDL_BUTTON_MIDDLE) {
                    button |= 0x04;
                }
                addFromBufferMouse(button);
                write_report_m(finalMouseBuffer);
            }
    // Text box focus handling
    else if (event.type == SDL_MOUSEBUTTONUP) {
        SDL_Point mouse_pos = {event.button.x, event.button.y};
        for (auto& pair : textBoxes) {
            if (is_inside(mouse_pos, pair.second.rect)) {
                pair.second.hasFocus = true;
            } else {
                pair.second.hasFocus = false;
            }
        }

        unsigned char button = 0;
        if (event.button.button == SDL_BUTTON_LEFT) {
            button |= 0x01;
            if (is_inside(mouse_pos, START_STOP_RECT)) {
                if (startStopColor.r == RED_COLOR.r && startStopColor.g == RED_COLOR.g) {
                    startStopColor = GREEN_COLOR;
                    isRunning = false;
                } else {
                    startStopColor = RED_COLOR;
                    isRunning = true;
                }
            }
            else if (is_inside(mouse_pos, COUNTER_RECT)) {
                setVariablesFromTextBoxes();
            }
            else if (is_inside(mouse_pos, RESET_RECT)) {
                write_report_k(emptyKey);
                write_report_m(emptyMouse);
            }
            else if (is_inside(mouse_pos, CLOSE_APP_RECT)) {
                quit = true;
            }

            else if (is_inside(mouse_pos, LAHN_BUTTON_RECT)) {
                currentChampLabel.text = "Current Champ : Lahn";
                loadVariablesFor("Lahn");
            } else if (is_inside(mouse_pos, WOOSA_BUTTON_RECT)) {
                currentChampLabel.text = "Current Champ : Woosa";
                loadVariablesFor("Woosa");
            } else if (is_inside(mouse_pos, SCHOLAR_BUTTON_RECT)) {
                currentChampLabel.text = "Current Champ : Scholar";
                loadVariablesFor("Scholar");
            } else if (is_inside(mouse_pos, WITCH_BUTTON_RECT)) {
                currentChampLabel.text = "Current Champ : Witch";
                loadVariablesFor("Witch");
            } else if (is_inside(mouse_pos, GUARDIAN_SUCC_BUTTON_RECT)) {
                currentChampLabel.text = "Current Champ : Guardian Succ";
                loadVariablesFor("GuardianSucc");
            } else if (is_inside(mouse_pos, GUARDIAN_AWAKE_BUTTON_RECT)) {
                currentChampLabel.text = "Current Champ : Guardian Awakening";
                loadVariablesFor("GuardianAwake");
            }

        } else if (event.button.button == SDL_BUTTON_RIGHT) {
            button |= 0x02;
        } else if (event.button.button == SDL_BUTTON_MIDDLE) {
            button |= 0x04;
        }
        removeFromBufferMouse(button);
        write_report_m(finalMouseBuffer);
    }
    // Text box input handling
    else if (event.type == SDL_TEXTINPUT) {
        for (auto& pair : textBoxes) {
            if (pair.second.hasFocus) {
                pair.second.text += event.text.text;
            }
        }
    }
    // Text box backspace handling
    else if (event.type == SDL_KEYDOWN) {
        for (auto& pair : textBoxes) {
            if (pair.second.hasFocus && event.key.keysym.sym == SDLK_BACKSPACE && !pair.second.text.empty()) {
                pair.second.text.pop_back();
            }
        }

        unsigned char key = sdlToUsbHidScanCodes[event.key.keysym.sym];
        if (isMacroActive) {
            continue;
        }

        if (event.key.keysym.sym == ZBuffKey && isRunning) {
            ActivateZBuff();
            // insert code to write hid keyboard
            continue;
        }

        if (event.key.keysym.sym == macroKey && isRunning) {
            if (!isMacroActive) {
                isMacroActive = true;
                macroThread = std::thread(macroFunction, std::ref(attacks));
            }
            continue;
        }

        bool isDuplicate = false;
        for (size_t i = 0; i < keyboardBufferIndex; i++) {
            if (keyboardBuffer[i] == key) {
                isDuplicate = true;
                break;
            }
        }
        if (event.key.keysym.scancode == SDL_SCANCODE_GRAVE && !isDuplicate) {
            key = 0x35;
        }
        if (key == 0 || isDuplicate) {
            continue; // Skip unknown keys
        }

        if (key == 0xE0 || key == 0xE4 || key == 0xE1 || key == 0xE5 || key == 0xE2 || key == 0xE6) {
            // Modifier keys
            if (event.key.keysym.sym == SDLK_LSHIFT) {
                keyboardModifiers |= 0x02; // Set LEFT SHIFT bit
            } else if (event.key.keysym.sym == SDLK_RSHIFT) {
                keyboardModifiers |= 0x20; // Set RIGHT SHIFT bit
            } else if (event.key.keysym.sym == SDLK_LCTRL) {
                keyboardModifiers |= 0x01; // Set LEFT CTRL bit
            } else if (event.key.keysym.sym == SDLK_RCTRL) {
                keyboardModifiers |= 0x10; // Set RIGHT CTRL bit
            } else if (event.key.keysym.sym == SDLK_LALT) {
                keyboardModifiers |= 0x04; // Set LEFT ALT bit
            } else if (event.key.keysym.sym == SDLK_RALT) {
                keyboardModifiers |= 0x40; // Set RIGHT ALT bit
            }
            prepareKeyboardBuffer();
            write_report_k(finalKeyboardBuffer);
        } else {
            addToBuffer(key, keyboardBuffer, keyBufferSize);
            prepareKeyboardBuffer();
            write_report_k(finalKeyboardBuffer);
        }
    }
    else if (event.type == SDL_KEYUP) {
        unsigned char key = sdlToUsbHidScanCodes[event.key.keysym.sym];
        if (event.key.keysym.sym == macroKey && isMacroActive && isRunning) {
            isMacroActive = false;
            macroCondition.notify_all();
            if (macroThread.joinable()) {
                macroThread.join();
            // after thread is closed, clear HID
            write_report_k(emptyKey);
            write_report_m(emptyMouse);
            }
            continue;
        }
        if (event.key.keysym.scancode == SDL_SCANCODE_GRAVE) {
            key = 0x35;
        }
        if (key == 0) {
            continue; // Skip unknown keys
        }
        if (key == 0xE0 || key == 0xE4 || key == 0xE1 || key == 0xE5 || key == 0xE2 || key == 0xE6) {
            // Modifier keys
            if (event.key.keysym.sym == SDLK_LSHIFT) {
                keyboardModifiers &= ~0x02; // Clear LEFT SHIFT bit
            } else if (event.key.keysym.sym == SDLK_RSHIFT) {
                keyboardModifiers &= ~0x20; // Clear RIGHT SHIFT bit
            } else if (event.key.keysym.sym == SDLK_LCTRL) {
                keyboardModifiers &= ~0x01; // Clear LEFT CTRL bit
            } else if (event.key.keysym.sym == SDLK_RCTRL) {
                keyboardModifiers &= ~0x10; // Clear RIGHT CTRL bit
            } else if (event.key.keysym.sym == SDLK_LALT) {
                keyboardModifiers &= ~0x04; // Clear LEFT ALT bit
            } else if (event.key.keysym.sym == SDLK_RALT) {
                keyboardModifiers &= ~0x40; // Clear RIGHT ALT bit
            }
            prepareKeyboardBuffer();
            write_report_k(finalKeyboardBuffer);
        } else {
            removeFromBuffer(keyboardBuffer, keyBufferSize, key);
            prepareKeyboardBuffer();
            write_report_k(finalKeyboardBuffer);
        }
    }
    else if (event.type == SDL_MOUSEMOTION) {
        mouseDeltaX = event.motion.xrel;
        mouseDeltaY = event.motion.yrel;

        finalMouseBuffer[1] = static_cast<unsigned char>(mouseDeltaX);
        finalMouseBuffer[2] = static_cast<unsigned char>(mouseDeltaY);

        write_report_m(finalMouseBuffer);

        finalMouseBuffer[1] = 0x00;
        finalMouseBuffer[2] = 0x00;
    }
    else if (event.type == SDL_MOUSEWHEEL) {
        if (event.wheel.y > 0) {
            finalMouseBuffer[3] = 0x01; // Scroll up
        } else if (event.wheel.y < 0) {
            finalMouseBuffer[3] = 0xFF; // Scroll down
        }
        write_report_m(finalMouseBuffer);
        finalMouseBuffer[3] = 0x00;
    }
}

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        for (const auto& pair : textBoxes) {
            renderTextBox(renderer, font, pair.second);
            }

        // Inside your main loop, after clearing the renderer
        renderCurrentChampLabel(renderer, font, currentChampLabel);

        // Start/Stop Button
        SDL_SetRenderDrawColor(renderer, startStopColor.r, startStopColor.g, startStopColor.b, 255);
        SDL_RenderFillRect(renderer, &START_STOP_RECT);
        SDL_Surface* startStopSurface = TTF_RenderText_Solid(font, (startStopColor.r == RED_COLOR.r && startStopColor.g == RED_COLOR.g) ? "Stop" : "Start", {0, 0, 0});
        SDL_Texture* startStopTexture = SDL_CreateTextureFromSurface(renderer, startStopSurface);
        int text_width = startStopSurface->w;
        int text_height = startStopSurface->h;
        SDL_FreeSurface(startStopSurface);
        SDL_Rect textRect = {START_STOP_RECT.x + (START_STOP_RECT.w - text_width) / 2, START_STOP_RECT.y + (START_STOP_RECT.h - text_height) / 2, text_width, text_height};
        SDL_RenderCopy(renderer, startStopTexture, NULL, &textRect);
        SDL_DestroyTexture(startStopTexture);

        // Hold Button // TODO Change Counter to HOLD
        SDL_SetRenderDrawColor(renderer, BLUE_COLOR.r, BLUE_COLOR.g, BLUE_COLOR.b, 255);
        SDL_RenderFillRect(renderer, &COUNTER_RECT);
        SDL_Surface* counterSurface = TTF_RenderText_Solid(font, "Set Var", {0, 0, 0});
        SDL_Texture* counterTexture = SDL_CreateTextureFromSurface(renderer, counterSurface);
        text_width = counterSurface->w;
        text_height = counterSurface->h;
        SDL_FreeSurface(counterSurface);
        SDL_Rect counterRect = {COUNTER_RECT.x + (COUNTER_RECT.w - text_width) / 2, COUNTER_RECT.y + (COUNTER_RECT.h - text_height) / 2, text_width, text_height};
        SDL_RenderCopy(renderer, counterTexture, NULL, &counterRect);
        SDL_DestroyTexture(counterTexture);

        // Reset Button
        SDL_SetRenderDrawColor(renderer, YELLOW_COLOR.r, YELLOW_COLOR.g, YELLOW_COLOR.b, 255);
        SDL_RenderFillRect(renderer, &RESET_RECT);
        SDL_Surface* resetSurface = TTF_RenderText_Solid(font, "Reset", {0, 0, 0});
        SDL_Texture* resetTexture = SDL_CreateTextureFromSurface(renderer, resetSurface);
        text_width = resetSurface->w;
        text_height = resetSurface->h;
        SDL_FreeSurface(resetSurface);
        SDL_Rect resetTextRect = {RESET_RECT.x + (RESET_RECT.w - text_width) / 2, RESET_RECT.y + (RESET_RECT.h - text_height) / 2, text_width, text_height};
        SDL_RenderCopy(renderer, resetTexture, NULL, &resetTextRect);
        SDL_DestroyTexture(resetTexture);

        // Render the Close button
        SDL_SetRenderDrawColor(renderer, RED_COLOR.r, RED_COLOR.g, RED_COLOR.b, 255);
        SDL_RenderFillRect(renderer, &CLOSE_APP_RECT);
        SDL_Surface* closeAppSurface = TTF_RenderText_Solid(font, "Close", {255, 255, 255}); // white text
        SDL_Texture* closeAppTexture = SDL_CreateTextureFromSurface(renderer, closeAppSurface);
        text_width = closeAppSurface->w;
        text_height = closeAppSurface->h;
        SDL_FreeSurface(closeAppSurface);
        SDL_Rect closeAppTextRect = {CLOSE_APP_RECT.x + (CLOSE_APP_RECT.w - text_width) / 2, CLOSE_APP_RECT.y + (CLOSE_APP_RECT.h - text_height) / 2, text_width, text_height};
        SDL_RenderCopy(renderer, closeAppTexture, NULL, &closeAppTextRect);
        SDL_DestroyTexture(closeAppTexture);

        // Render Champ Buttons
        renderButton(renderer, font, LAHN_BUTTON_RECT, LAHN_COLOR, "Lahn");
        renderButton(renderer, font, WOOSA_BUTTON_RECT, WOOSA_COLOR, "Woosa");
        renderButton(renderer, font, SCHOLAR_BUTTON_RECT, SCHOLAR_COLOR, "Scholar");
        renderButton(renderer, font, WITCH_BUTTON_RECT, WITCH_COLOR, "Witch");
        renderButton(renderer, font, GUARDIAN_SUCC_BUTTON_RECT, GUARDIAN_SUCC_COLOR, "GuardianSucc");
        renderButton(renderer, font, GUARDIAN_AWAKE_BUTTON_RECT, GUARDIAN_AWAKE_COLOR, "GuardianAwake");

        displayBufferContents(renderer, font);

        // Render Text Boxes
        for (auto& pair : textBoxes) {
            renderTextBox(renderer, font, pair.second);
            }

        // Render Text Label
        for (const auto& pair : textBoxLabels) {
            renderLabel(renderer, font, pair.second);
            }

        // Finalize
        SDL_RenderPresent(renderer);
    }

    write_report_k(emptyKey);
    write_report_m(emptyMouse);

    hidFileKeyboard.close();
    hidFileMouse.close();

    if (macroThread.joinable()) {
        macroThread.join();
    }

    if (ZBuffThread.joinable()) {
        ZBuffThread.join();
    }


    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}
