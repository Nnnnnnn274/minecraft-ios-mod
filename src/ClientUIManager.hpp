/*
 *  ClientUIManager.hpp — Minecraft Client Loader
 *  ===============================================
 *  Component 4: Custom UI Texture Pipeline & Procedural Minecraft Styling
 *
 *  Provides two texture creation paths:
 *    1. File-based — loads PNG assets from the filesystem using stb_image
 *    2. Procedural — generates Minecraft-styled pixel textures at runtime
 *
 *  All textures use GL_NEAREST filtering to preserve the authentic
 *  pixelated look on high-DPI mobile displays.
 */

#pragma once
#include <string>
#include <vector>
#include <cstdint>

#ifdef __APPLE__
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#endif

class ClientUIManager {
public:
    ClientUIManager();
    ~ClientUIManager();

    /*
     *  loadTextureFromFile()
     *  --------------------
     *  Loads a PNG texture from the filesystem via stb_image, uploads it
     *  to OpenGL ES with NEAREST filtering, and returns the GL handle.
     *
     *  Parameters:
     *    path        - Filesystem path to the PNG asset
     *    out_texture - Output: generated GL texture name
     *    out_width   - Output: texture width in pixels
     *    out_height  - Output: texture height in pixels
     *
     *  Returns: true on success, false on file error.
     */
    bool loadTextureFromFile(const std::string& path,
                             GLuint* out_texture,
                             int* out_width,
                             int* out_height);

    /*
     *  generateMinecraftUIElement()
     *  ----------------------------
     *  Procedurally generates a Minecraft-styled UI texture by writing
     *  raw RGBA pixel data into an OpenGL ES texture object.
     *
     *  styleType values:
     *    0 — Vanilla Button (gray stone button with dark shadow edges)
     *    1 — Hover Button (lime-green highlighted variant)
     *    2 — Dirt/Stone Background Panel (tiled pixel grid)
     */
    void generateMinecraftUIElement(GLuint* out_texture,
                                    int width,
                                    int height,
                                    int styleType);

    /*
     *  getFallbackTexture()
     *  --------------------
     *  Convenience wrapper that generates a procedural texture and returns
     *  the GL handle directly. Useful when a file load fails.
     */
    GLuint getFallbackTexture(int width, int height, int styleType);

    /*
     *  getOrCreateTexture()
     *  --------------------
     *  High-level API: attempts to load from file first; if that fails,
     *  falls back to procedural generation with the given style parameters.
     */
    GLuint getOrCreateTexture(const std::string& filePath,
                              int width,
                              int height,
                              int styleType);

private:
    /*
     *  Internal texture slot tracker for cleanup in the destructor.
     */
    struct TextureSlot {
        GLuint id;       // OpenGL texture name
        int width;       // Texture width
        int height;      // Texture height
    };

    std::vector<TextureSlot> proceduralTextures_;

    /*
     *  uploadTexture()
     *  ---------------
     *  Uploads raw RGBA pixel data to OpenGL ES as a 2D texture with
     *  GL_NEAREST min/mag filtering to preserve pixel art quality.
     */
    GLuint uploadTexture(const uint8_t* pixels, int width, int height);

    /*
     *  Procedural generators for each style type.
     */
    void generateButtonStyle(uint8_t* pixels, int width, int height, bool hover);
    void generatePanelStyle(uint8_t* pixels, int width, int height);
};
