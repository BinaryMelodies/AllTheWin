# All the Windows versions

This is an attempt to compile a simple test application for (almost) all versions of Windows using (almost) exclusively open source tools.

Currently the following versions are supported:
* Windows 1.x (win1x.exe)
* Windows 2.x (win2x.exe)
* Windows 3.x (win3x.exe)
* Win32s (win4x.exe)
* Windows 95 and later (win4x.exe)
* Windows NT and later (win4x.exe, win4xw.exe)
* Windows XP (win5x.exe)
* 64-bit Windows (win6x.exe)
* Windows Phone (armce.exe, x86ce.exe)

The project originally started because OpenWatcom provides an excellent toolkit to compile 16-bit and 32-bit applications, however it does not support Windows versions below 3.0.
To fill in the missing details, a few files had to be replaced or developed.
Currently the only open source tool that is missing is to compile the Windows resources for Windows versions 1.0 and 2.0.

The project also explores the idea of what software developed for these Windows versions might look like.
There are several "versions" of the application that can be compiled from the same source code.
For example, version 1.0 of the software only provides features available for Windows 1.x, etc.

The About window displays "Copyleft" as a license.
This is not true, as the project is licensed under a more permissive license, the [MIT license](LICENSE).
The line was included at the time as a joke, as a reversal of the early Microsoft examples.

## 16-bit versions

* win1x.exe: "Version 1.0", targetting Windows 1.x, compiled using OpenWatcom.
* win2x.exe: "Version 2.0", targetting Windows 2.x, with support for earlier Windows versions, compiled using OpenWatcom.
* win3x.exe: "Version 3.0", targetting Windows 3.x, compiled using OpenWatcom.

## 32-bit versions

* win4x.exe: "Version 4.0", targetting Windows 95, also runs as a Win32s application on Windows 3.x (provided that the Win32s tool is installed), compiled using OpenWatcom.
* win4xw.eex: Recompiled for Unicode support, targets Windows NT 3.x/4.x, compiled using OpenWatcom.
* win5x.exe: "Version 5.0", targetting Windows XP, compiled using OpenWatcom.
* gnu5x.exe: Same code, compiled using MinGW tools.
* win6x32.exe: "Version 6.0", compiled using MinGW tools. (Not significantly different from gnu5x.exe)

## 64-bit versions

* win6x.exe: "Version 6.0", compiled using MinGW tools. (Marked as "targetting Windows 7", but no Windows 7 specific code was included)

## Windows Phone versions

* armce.exe, x86ce.exe: (Not entirely certain these work properly)

# Required tools

To compile everything, the following tools are required:

* OpenWatcom C compiler (if 16-bit support is desired)
* MinGW C compiler (if 64-bit support is desired)
* [CeGCC compiler](https://cegcc.sourceforge.net) for Windows CE
* Python interpreter
* Microsoft Resource Compiler targetting Windows 1 and 2
* Make and a UNIX terminal

Before compiling, the paths to these compilers must be set up in Makefile.

# Disclaimer

This code is provided as is with no warranty to correctness or code quality.
It is provided as an educational tool, as an example of what is possible among the various versions of Windows.

# References

To develop this project, the examples in early Microsoft Windows APIs have been consulted, as well as initialization files (crts) for open source compilers.
Some URLs that proved helpful:

* https://learn.microsoft.com/en-us/windows/win32/gdi/drawing-with-the-mouse
* https://learn.microsoft.com/en-us/windows/win32/learnwin32/working-with-strings
* https://learn.microsoft.com/en-us/windows/win32/sysinfo/targeting-your-application-at-windows-8-1
* https://learn.microsoft.com/en-us/windows/win32/menurc/versioninfo-resource
* https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-iswindowunicode
* https://github.com/mingw-w64/mingw-w64/blob/master/mingw-w64-crt/crt/crtexewin.c
* https://www.transmissionzero.co.uk/computing/win32-apps-with-mingw
* https://zetcode.com/gui/winapi/controls

