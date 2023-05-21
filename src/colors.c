#include "colors.h"
#include "math.h"

typedef struct {
    int r;
    int g;
    int b;
} RGBColor;

typedef struct {
    int h;
    int s;
    int v;
} HSVColor;

void initTextColor(int id, int color) {
    setColorPair(id, color, COLOR_BLACK);
}

void initBGColorWhite(int id, int color) {
    setColorPair(id, COLOR_WHITE, color);
}

void initBGColorBlack(int id, int color) {
    setColorPair(id, COLOR_BLACK, color);
}

int getTextColor(short id) {
    short useless, textColor;
    pair_content(id, &textColor, &useless);
    return textColor;
}

void initColors() {
    // See https://www.ditig.com/256-colors-cheat-sheet
    setColor(COLOR_WHITE, 255, 255, 255);
    setColor(COLOR_BLACK, 0, 0, 0);
    setColorPair(PASTEQUE_COLOR_WHITE, COLOR_WHITE, COLOR_BLACK);
    setColorPair(PASTEQUE_COLOR_BLACK, COLOR_BLACK, COLOR_WHITE);

    // Replace yellow with a more vivid color for terminals that support it.
    setColor(226, 252, 255, 30);
    initTextColor(PASTEQUE_COLOR_YELLOW, 226);

    setColorPair(PASTEQUE_COLOR_BLANK, COLOR_BLACK, COLOR_BLACK);

    initTextColor(PASTEQUE_COLOR_BLUE, 33);

    initTextColor(PASTEQUE_COLOR_RED, 160);

    initTextColor(PASTEQUE_COLOR_ORANGE, 208);

    initTextColor(PASTEQUE_COLOR_DARK_ORANGE, 166);

    initTextColor(PASTEQUE_COLOR_GREEN, 155);

    initTextColor(PASTEQUE_COLOR_DARK_GREEN, 28);

    initTextColor(PASTEQUE_COLOR_TOMATO, 202);

    initTextColor(PASTEQUE_COLOR_FUSCHIA, 165);

    initTextColor(PASTEQUE_COLOR_TURQUOISE, 45);

    setColorPair(PASTEQUE_COLOR_YELLOW_HIGHLIGHT_BG, COLOR_BLACK, COLOR_YELLOW);

    initBGColorBlack(PASTEQUE_COLOR_YELLOW_FOCUS_BG, 202);

    initBGColorWhite(PASTEQUE_COLOR_RED_BLINK1_BG, 204);

    initBGColorBlack(PASTEQUE_COLOR_RED_BLINK2_BG, 196);

    setColorPair(PASTEQUE_COLOR_GREY_25_BG, COLOR_WHITE, 237);

    initBGColorBlack(PASTEQUE_COLOR_LIGHT_BLUE_BG, 39);

    initBGColorBlack(PASTEQUE_COLOR_TURQUOISE_BG, 45);

    setColorPair(PASTEQUE_COLOR_BLUE_ON_WHITE, getTextColor(PASTEQUE_COLOR_BLUE), COLOR_WHITE);
    setColorPair(PASTEQUE_COLOR_WHITE_ON_WHITE, COLOR_WHITE, COLOR_WHITE);
    setColorPair(PASTEQUE_COLOR_WHITE_ON_RED, COLOR_WHITE, getTextColor(PASTEQUE_COLOR_RED));
    setColorPair(PASTEQUE_COLOR_WHITE_ON_DARK_GREEN, COLOR_WHITE, getTextColor(PASTEQUE_COLOR_DARK_GREEN));
    setColorPair(PASTEQUE_COLOR_WHITE_ON_DARK_ORANGE, COLOR_WHITE, getTextColor(PASTEQUE_COLOR_DARK_ORANGE));

    setColorPair(PASTEQUE_COLOR_WATERMELON_BG_DYN, COLOR_BLACK, 210);
    setColorPair(PASTEQUE_COLOR_WATERMELON_BG_LIGHT_DYN, COLOR_BLACK, 211);
    setColorPair(PASTEQUE_COLOR_WATERMELON_DYN, 210, COLOR_BLACK);
    setColorPair(PASTEQUE_COLOR_WHITE_ON_WATERMELON_DYN, COLOR_WHITE, 210);
}

// RGB-HSV Color conversion code taken from
// https://stackoverflow.com/questions/3018313/algorithm-to-convert-rgb-to-hsv-and-hsv-to-rgb-in-range-0-255-for-both
// (I'm too lazy to write down those boring conversion methods)

RGBColor toRGB(HSVColor hsv) {
    RGBColor rgb;
    unsigned char region, remainder, p, q, t;

    if (hsv.s == 0) {
        rgb.r = hsv.v;
        rgb.g = hsv.v;
        rgb.b = hsv.v;
        return rgb;
    }

    region = hsv.h / 43;
    remainder = (hsv.h - (region * 43)) * 6;

    p = (hsv.v * (255 - hsv.s)) >> 8;
    q = (hsv.v * (255 - ((hsv.s * remainder) >> 8))) >> 8;
    t = (hsv.v * (255 - ((hsv.s * (255 - remainder)) >> 8))) >> 8;

    switch (region) {
        case 0:
            rgb.r = hsv.v;
            rgb.g = t;
            rgb.b = p;
            break;
        case 1:
            rgb.r = q;
            rgb.g = hsv.v;
            rgb.b = p;
            break;
        case 2:
            rgb.r = p;
            rgb.g = hsv.v;
            rgb.b = t;
            break;
        case 3:
            rgb.r = p;
            rgb.g = q;
            rgb.b = hsv.v;
            break;
        case 4:
            rgb.r = t;
            rgb.g = p;
            rgb.b = hsv.v;
            break;
        default:
            rgb.r = hsv.v;
            rgb.g = p;
            rgb.b = q;
            break;
    }

    return rgb;
}

HSVColor toHSV(RGBColor rgb) {
    HSVColor hsv;
    unsigned char rgbMin, rgbMax;

    rgbMin = rgb.r < rgb.g ? (rgb.r < rgb.b ? rgb.r : rgb.b) : (rgb.g < rgb.b ? rgb.g : rgb.b);
    rgbMax = rgb.r > rgb.g ? (rgb.r > rgb.b ? rgb.r : rgb.b) : (rgb.g > rgb.b ? rgb.g : rgb.b);

    hsv.v = rgbMax;
    if (hsv.v == 0) {
        hsv.h = 0;
        hsv.s = 0;
        return hsv;
    }

    hsv.s = 255 * (long) (rgbMax - rgbMin) / hsv.v;
    if (hsv.s == 0) {
        hsv.h = 0;
        return hsv;
    }

    if (rgbMax == rgb.r)
        hsv.h = 0 + 43 * (rgb.g - rgb.b) / (rgbMax - rgbMin);
    else if (rgbMax == rgb.g)
        hsv.h = 85 + 43 * (rgb.b - rgb.r) / (rgbMax - rgbMin);
    else
        hsv.h = 171 + 43 * (rgb.r - rgb.g) / (rgbMax - rgbMin);

    return hsv;
}

HSVColor lerpHSV(HSVColor a, HSVColor b, float alpha) {
    if (alpha < 0.0f) { alpha = 0; }
    else if (alpha > 1.0f) { alpha = 1.0f; }

    HSVColor result = {
            (int) ((float) a.h * (1 - alpha) + (float) b.h * (alpha)),
            (int) ((float) a.s * (1 - alpha) + (float) b.s * (alpha)),
            (int) ((float) a.v * (1 - alpha) + (float) b.v * (alpha))};
    return result;
}

void colorsUpdateDynamic(unsigned long gameTime) {
    static RGBColor watermelonGradient[] = {
            {235, 134, 134},
            {162, 235, 131}
    };
    int watermelonNumColors = 2;
    float watermelonStillTime = 0.575f; // Seconds for both colors
    float watermelonTransitionTime = 1.0f;
    float watermelonPassTime = watermelonStillTime * 2 + watermelonTransitionTime;
    float watermelonTotalTime = watermelonPassTime * 2;

    // Update the watermelon color
    float gtSec = gameTime / (1000.0f * 1000.0f);
    float gradientTime = fmodf(gtSec, watermelonTotalTime);
    float loopTime = gradientTime < watermelonPassTime ? gradientTime : (watermelonTotalTime - gradientTime);
    RGBColor finalColor;

    if (loopTime < watermelonStillTime) {
        finalColor = watermelonGradient[0];
    } else if (loopTime <= watermelonStillTime + watermelonTransitionTime) {
        float alpha = (loopTime - watermelonStillTime) / watermelonTransitionTime;
        finalColor = toRGB(lerpHSV(toHSV(watermelonGradient[0]), toHSV(watermelonGradient[1]), alpha));
    } else {
        finalColor = watermelonGradient[1];
    }

    setColor(210, finalColor.r, finalColor.g, finalColor.b);

    // Update the light watermelon color
    HSVColor hsvColor = toHSV(finalColor);
    hsvColor.s -= 40;
    hsvColor.v += 40;
    if (hsvColor.s < 0) { hsvColor.s = 0; }
    if (hsvColor.v > 255) { hsvColor.v = 255; }
    RGBColor lightColor = toRGB(hsvColor);

    setColor(211, lightColor.r, lightColor.g, lightColor.b);
}
