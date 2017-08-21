#include <arcan_shmif.h>
#include "lua.h"

/*
 * preconditions:
 *  1. modifications to TUI
 *     - custom draw region support
 *
 *  2. exposing TUI api as a lua metatable
 *
 *  grab luvit/luvit/blob/master/deps/ustring.lua and readline.lua
 *  patch readline to work with the TUI functions.
 */

static void prepare_env(lua_State* l)
{
/* if not envtbl, forward existing environment */
/* if append arg set, overlay envtbl on existing environment and exec */
}

static void exec_handover(lua_State* l)
{
/* requires us to fake-inject the preroll stuff we may want in the child eventqueue */
	prepare_env(l);
}

static void exec_new(lua_State* l)
{
	prepare_env(l);
}

static void exec_proxy(lua_State* l)
{
/* requires a libifcation of the main arcan client management code,
 * and then proxy map to subsegment requests on a new connection */
	prepare_env(l);
}

static void exec_legacy(lua_State* l)
{
/* we can probably have fork/detach here */
	prepare_env(l);
}

static void on_line(lua_State* l)
{
}

static void loop(lua_State* l)
{
	while (true){
/* use active_tui->readline */
	}
}

int lash_setup(struct arcan_shmif_cont* con)
{
/* 1. spawn empty Lua context */
/* 2. expand functions */
/* 3. install signal handlers and all that crap */
/* 4. run builtin- script */
/* 5. functions needed:
 *    - exec [legacy] : spawn afsrv_terminal with a shell through TUI. For this to work, we need to add a way to switch TUI screen
 *    - exec [handover] + [forward] : check if target binary actually links to shmif
 *    - exec [new connection] : not much to it, forward connpath/connkey
 *
 * 6. steal some of the normal interactive lua shell code
 * 7. build the normal script
 * 8. really a shmif- extension, but FUSE-shmif mount somewhere
 */
}

/*
 * useful keybindings:
 *  1. paninc *in exec legacy
 *  - be able to set default exec command
 *  - ls, cp, ...
 */
