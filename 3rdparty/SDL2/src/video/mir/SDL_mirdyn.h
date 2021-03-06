/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2016 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#ifndef _SDL_mirdyn_h
#define _SDL_mirdyn_h

#include "../../SDL_internal.h"

#include <EGL/egl.h>
#include <mir_toolkit/mir_client_library.h>
#include <xkbcommon/xkbcommon.h>

#ifdef __cplusplus
extern "C" {
#endif

int SDL_MIR_LoadSymbols(void);
void SDL_MIR_UnloadSymbols(void);

/* Declare all the function pointers and wrappers... */
#define SDL_MIR_SYM(rc,fn,params) \
    typedef rc (*SDL_DYNMIRFN_##fn) params; \
    extern SDL_DYNMIRFN_##fn MIR_##fn;
#define SDL_MIR_SYM_CONST(type, name) \
    typedef type SDL_DYMMIRCONST_##name; \
    extern SDL_DYMMIRCONST_##name MIR_##name;
#include "SDL_mirsym.h"

#ifdef __cplusplus
}
#endif

#endif /* !defined _SDL_mirdyn_h */

/* vi: set ts=4 sw=4 expandtab: */
