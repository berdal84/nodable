#pragma once

/**
 * A simple struct to store font configuration
 */
struct FontConf
{
    const char*  id;//Font identifier
    const char*  path; // Font path relative to application folder
    float        size; // Font size in px
    bool         icons_enable; // If true, icons will be merged to the font
    float        icons_size;
};