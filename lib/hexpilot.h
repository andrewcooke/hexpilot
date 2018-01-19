
#ifndef HP_HEXPILOT_H
#define HP_HEXPILOT_H

/**
 * @file
 *
 * @brief Assorted definitions.
 */
/// @{

/// The program name.
#define HEXPILOT "hexpilot"
/// The environment variable showing where to find data files.
#define HEXPILOT_DATA "HEXPILOT_DATA"

/// A hack to convert a preprocessor variable into a string - see http://gcc.gnu.org/onlinedocs/gcc-4.1.2/cpp/Stringification.html
#define hp_xstr(a) hp_str(a)
#define hp_str(a) #a

/// @}
#endif
