// version, change on release
#ifndef REMOD_VERSION
#define REMOD_VERSION "SVN build $Rev$"
#endif

// system
#if defined(_WIN32) || defined(_WIN64) || defined(__MINGW32__)
    #define REMOD_SYSTEM "Windows"
#elif defined(__linux__)
    #define REMOD_SYSTEM "Linux"
#elif defined(__FreeBSD__)
    #define REMOD_SYSTEM "FreeBSD"
#elif defined(BSD)
    #define REMOD_SYSTEM "BSD"
#elif defined(__sun)
    #define REMOD_SYSTEM "Solaris"
#elif defined(__APPLE__)
    #define REMOD_SYSTEM "Mac"
#else
    #define REMOD_SYSTEM "Unknown"
#endif

// cpu arch
#if defined(__i386) || defined(__i386__) || defined(i386)
    #define REMOD_ARCH "x32"
#elif defined(__x86_64) || defined(__x86_64__) || defined(__amd64) || defined(__amd64__)
    #define REMOD_ARCH "x64"
#else
    #define REMOD_ARCH "x??"
#endif
