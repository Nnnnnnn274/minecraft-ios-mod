#define GLES_SILENCE_DEPRECATION
/*
 *  ClientUIManager.cpp — Minecraft Client Loader
 *  ===============================================
 *  Component 4: Custom UI Texture Pipeline & Procedural Minecraft Styling
 *
 *  This manager provides two texture creation paths:
 *    - File loading via stb_image (supports PNG, JPG, BMP, etc.)
 *    - Procedural pixel generation mimicking Minecraft's classic UI
 *
 *  Texture filtering uses GL_NEAREST exclusively to maintain the
 *  authentic retro-blocky appearance of Minecraft's interface on
 *  modern high-resolution iOS displays.
 *
 *  If external texture files are missing, the system automatically
 *  falls back to procedural generation so the UI always renders.
 */

#include "ClientUIManager.hpp"
#include <cstdio>
#include <cstring>
#include <cstdlib>

/*
 *  stb_image: public-domain image loader.
 *  The implementation is inlined here via STB_IMAGE_IMPLEMENTATION.
 *  Replace with the full stb_image.h from https://github.com/nothings/stb
 */
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

/*
 *  8x8 tiled pixel pattern for the dirt/stone background panel (style 2).
 *  Simulates Minecraft's classic options screen background with a
 *  repeating dark gray checkerboard-like noise pattern.
 */
static const uint8_t PANEL_TILE_PATTERN[8][8] = {
    {0x8B, 0x8B, 0x7B, 0x7B, 0x8B, 0x8B, 0x7B, 0x7B},
    {0x8B, 0x7B, 0x8B, 0x8B, 0x8B, 0x7B, 0x8B, 0x8B},
    {0x7B, 0x8B, 0x8B, 0x7B, 0x7B, 0x8B, 0x8B, 0x7B},
    {0x7B, 0x8B, 0x7B, 0x8B, 0x7B, 0x8B, 0x7B, 0x8B},
    {0x8B, 0x8B, 0x7B, 0x7B, 0x8B, 0x8B, 0x7B, 0x7B},
    {0x8B, 0x7B, 0x8B, 0x8B, 0x8B, 0x7B, 0x8B, 0x8B},
    {0x7B, 0x8B, 0x8B, 0x7B, 0x7B, 0x8B, 0x8B, 0x7B},
    {0x7B, 0x8B, 0x7B, 0x8B, 0x7B, 0x8B, 0x7B, 0x8B},
};

/*
 *  setPixel()
 *  ----------
 *  Helper to write an RGBA pixel into a raw byte buffer at (x, y).
 *  Bounds-checked to prevent out-of-range writes.
 */
static void setPixel(uint8_t* pixels, int x, int y, int width, int height,
                     uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    if (x < 0 || x >= width || y < 0 || y >= height) return;
    int idx = (y * width + x) * 4;
    pixels[idx + 0] = r;
    pixels[idx + 1] = g;
    pixels[idx + 2] = b;
    pixels[idx + 3] = a;
}

ClientUIManager::ClientUIManager() {}
ClientUIManager::~ClientUIManager() {
    /* Clean up all procedurally generated OpenGL textures */
    for (auto& tex : proceduralTextures_) {
        glDeleteTextures(1, &tex.id);
    }
    proceduralTextures_.clear();
}

/*
 *  uploadTexture()
 *  ---------------
 *  Uploads raw RGBA pixel data to OpenGL ES as a 2D texture.
 *  Uses GL_NEAREST for both minification and magnification filters to
 *  preserve crisp pixel-art rendering without anti-aliasing blur.
 */
GLuint ClientUIManager::uploadTexture(const uint8_t* pixels, int width, int height) {
    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    /*
     *  GL_NEAREST: point filtering — no linear interpolation.
     *  This preserves the blocky pixel look essential for Minecraft.
     */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    /*
     *  Clamp to edge to avoid texture wrapping artifacts on UI elements.
     */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    /*
     *  Upload the raw RGBA byte data as a 2D texture image.
     *  The source is always 4 channels (RGBA) regardless of input format.
     */
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    glBindTexture(GL_TEXTURE_2D, 0);
    return texture;
}

/*
 *  loadTextureFromFile()
 *  --------------------
 *  Loads an image from disk using stb_image, converts to RGBA, and
 *  uploads it to OpenGL ES. Returns the GL texture handle plus the
 *  image dimensions via out pointers.
 *
 *  Falls back gracefully: returns false if the file is missing or
 *  corrupted, allowing the caller to use procedural generation.
 */
bool ClientUIManager::loadTextureFromFile(const std::string& path,
                                          GLuint* out_texture,
                                          int* out_width,
                                          int* out_height) {
    if (path.empty() || out_texture == nullptr) return false;

    int width = 0, height = 0, channels = 0;

    /*
     *  stbi_load loads any common image format (PNG, JPG, BMP, GIF, etc.)
     *  We force 4 channels (RGBA) with the last parameter so the data
     *  layout matches what OpenGL expects for GL_RGBA.
     */
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 4);
    if (data == nullptr) {
        printf("[MCCL] UI: Failed to load texture from '%s'\n", path.c_str());
        return false;
    }

    /* Upload to OpenGL and populate output parameters */
    *out_texture = uploadTexture(data, width, height);
    if (out_width) *out_width = width;
    if (out_height) *out_height = height;

    printf("[MCCL] UI: Loaded texture '%s' (%dx%d)\n", path.c_str(), width, height);

    stbi_image_free(data);
    return true;
}

/*
 *  generateButtonStyle()
 *  ---------------------
 *  Draws a Minecraft-style button shape into the pixel buffer.
 *
 *  Layout (2px border on all sides):
 *    Top/Left border   — light highlight (white or lime-green for hover)
 *    Bottom/Right edge — dark shadow (dark gray or dark green for hover)
 *    Center body       — medium stone gray
 *
 *  This matches the classic Minecraft button appearance from the
 *  original Java edition style guide.
 */
void ClientUIManager::generateButtonStyle(uint8_t* pixels, int width, int height, bool hover) {
    int border = 2;

    /* Initialize all pixels to transparent black */
    memset(pixels, 0, (size_t)(width * height * 4));

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            bool onTopEdge    = (y < border);
            bool onBottomEdge = (y >= height - border);
            bool onLeftEdge   = (x < border);
            bool onRightEdge  = (x >= width - border);

            if (hover) {
                /*
                 *  Hover style: lime-green highlights.
                 *  Top/left: bright green (#55FF55)
                 *  Bottom/right: dark green (#2A8A2A)
                 *  Body: stone gray (#666666)
                 */
                if (onTopEdge || onLeftEdge) {
                    setPixel(pixels, x, y, width, height, 0x55, 0xFF, 0x55, 0xFF);
                } else if (onBottomEdge || onRightEdge) {
                    setPixel(pixels, x, y, width, height, 0x2A, 0x8A, 0x2A, 0xFF);
                } else {
                    setPixel(pixels, x, y, width, height, 0x66, 0x66, 0x66, 0xFF);
                }
            } else {
                /*
                 *  Normal style: classic stone button.
                 *  Top/left: light gray highlight (#DDDDDD)
                 *  Bottom/right: dark gray shadow (#333333)
                 *  Body: medium gray (#888888)
                 */
                if (onTopEdge || onLeftEdge) {
                    setPixel(pixels, x, y, width, height, 0xDD, 0xDD, 0xDD, 0xFF);
                } else if (onBottomEdge || onRightEdge) {
                    setPixel(pixels, x, y, width, height, 0x33, 0x33, 0x33, 0xFF);
                } else {
                    setPixel(pixels, x, y, width, height, 0x88, 0x88, 0x88, 0xFF);
                }
            }
        }
    }
}

/*
 *  generatePanelStyle()
 *  --------------------
 *  Fills the pixel buffer with a repeating 8x8 noise pattern that
 *  mimics Minecraft's classic dirt/stone background screen from the
 *  options menu. Uses the predefined PANEL_TILE_PATTERN matrix.
 */
void ClientUIManager::generatePanelStyle(uint8_t* pixels, int width, int height) {
    memset(pixels, 0, (size_t)(width * height * 4));
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            uint8_t val = PANEL_TILE_PATTERN[y % 8][x % 8];
            /* Grayscale — R=G=B for a monochrome stone/dirt look */
            setPixel(pixels, x, y, width, height, val, val, val, 0xFF);
        }
    }
}

/*
 *  generateMinecraftUIElement()
 *  ----------------------------
 *  Public API: generates a procedural Minecraft-styled UI texture.
 *
 *  styleType mapping:
 *    0 → Vanilla button (normal, gray/slate)
 *    1 → Hover/active button (lime-green accent)
 *    2 → Dirt/stone background panel (tiled noise)
 *    default → Falls back to vanilla button style
 */
void ClientUIManager::generateMinecraftUIElement(GLuint* out_texture,
                                                 int width,
                                                 int height,
                                                 int styleType) {
    if (out_texture == nullptr || width <= 0 || height <= 0) return;

    /* Allocate RGBA pixel buffer (width * height * 4 bytes) */
    auto* pixels = (uint8_t*)malloc((size_t)(width * height * 4));
    if (pixels == nullptr) return;

    /* Dispatch to the appropriate style generator */
    switch (styleType) {
        case 0:
            generateButtonStyle(pixels, width, height, false);
            break;
        case 1:
            generateButtonStyle(pixels, width, height, true);
            break;
        case 2:
            generatePanelStyle(pixels, width, height);
            break;
        default:
            generateButtonStyle(pixels, width, height, false);
            break;
    }

    /* Upload to OpenGL ES */
    *out_texture = uploadTexture(pixels, width, height);

    /* Track for later cleanup in destructor */
    TextureSlot slot;
    slot.id = *out_texture;
    slot.width = width;
    slot.height = height;
    proceduralTextures_.push_back(slot);

    free(pixels);
}

/*
 *  getFallbackTexture()
 *  --------------------
 *  Convenience wrapper: generates a procedural texture and returns the
 *  GL handle immediately. Useful for fallback paths and quick testing.
 */
GLuint ClientUIManager::getFallbackTexture(int width, int height, int styleType) {
    GLuint texture = 0;
    generateMinecraftUIElement(&texture, width, height, styleType);
    return texture;
}

/*
 *  getOrCreateTexture()
 *  --------------------
 *  High-level unified API: tries file-first, falls back to procedural.
 *  If filePath is empty or the file can't be loaded, the requested
 *  procedural style is generated instead. This ensures the UI system
 *  never shows a blank texture.
 */
GLuint ClientUIManager::getOrCreateTexture(const std::string& filePath,
                                           int width,
                                           int height,
                                           int styleType) {
    GLuint texture = 0;
    int w = 0, h = 0;

    /* Attempt file-based load first */
    if (!filePath.empty() && loadTextureFromFile(filePath, &texture, &w, &h)) {
        return texture;
    }

    /* Fall back to procedural generation */
    return getFallbackTexture(width, height, styleType);
}
