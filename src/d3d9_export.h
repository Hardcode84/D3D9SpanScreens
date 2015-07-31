
#ifndef D3D9_EXPORT_H
#define D3D9_EXPORT_H

#ifdef D3D9_STATIC_DEFINE
#  define D3D9_EXPORT
#  define D3D9_NO_EXPORT
#else
#  ifndef D3D9_EXPORT
#    ifdef d3d9_EXPORTS
        /* We are building this library */
#      define D3D9_EXPORT __declspec(dllexport)
#    else
        /* We are using this library */
#      define D3D9_EXPORT __declspec(dllimport)
#    endif
#  endif

#  ifndef D3D9_NO_EXPORT
#    define D3D9_NO_EXPORT 
#  endif
#endif

#ifndef D3D9_DEPRECATED
#  define D3D9_DEPRECATED __attribute__ ((__deprecated__))
#  define D3D9_DEPRECATED_EXPORT D3D9_EXPORT __attribute__ ((__deprecated__))
#  define D3D9_DEPRECATED_NO_EXPORT D3D9_NO_EXPORT __attribute__ ((__deprecated__))
#endif

#define DEFINE_NO_DEPRECATED 0
#if DEFINE_NO_DEPRECATED
# define D3D9_NO_DEPRECATED
#endif

#endif
