#pragma once

/**
 * A simple struct to store font configuration
 */
struct FontConf {
    /** Font identifier */
    const char* id;
    /** Font size in px */
    float size;
    /** Font path relative to application folder */
    const char* path;
    /** If true, icons will be merged to the font */
    bool enableIcons;
};