/*
 Arcan Shared Memory Interface, Interoperability definitions

 Copyright (c) 2014-2016, Bjorn Stahl
 All rights reserved.

 Redistribution and use in source and binary forms,
 with or without modification, are permitted provided that the
 following conditions are met:

 1. Redistributions of source code must retain the above copyright notice,
 this list of conditions and the following disclaimer.

 2. Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.

 3. Neither the name of the copyright holder nor the names of its contributors
 may be used to endorse or promote products derived from this software without
 specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef HAVE_ARCAN_SHMIF_INTEROP
#define HAVE_ARCAN_SHMIF_INTEROP

/*
 * Version number works as tag and guard- bytes in the shared memory page, it
 * is set by arcan upon creation and verified along with the offset- cookie
 * during _integrity_check
 */
#define ASHMIF_VERSION_MAJOR 0
#define ASHMIF_VERSION_MINOR 9

#ifndef LOG
#define LOG(...) (fprintf(stderr, __VA_ARGS__))
#endif

#define SHMIF_PT_SIZE(ppcm, sz_mm) ((size_t)(\
	(((double)(sz_mm)) / 0.352778) * \
	(((double)(ppcm)) / 28.346566) \
))

/*
 * For porting the shmpage interface, these functions need to be implemented
 * and pulled in, shouldn't be more complicated than mapping to the
 * corresponding platform/ functions.
 */
#ifndef PLATFORM_HEADER

#define BADFD -1
#include <sys/types.h>
#include <sys/stat.h>
#include <semaphore.h>
typedef int file_handle;
typedef pid_t process_handle;
typedef sem_t* sem_handle;

long long int arcan_timemillis(void);
int arcan_sem_post(sem_handle sem);
file_handle arcan_fetchhandle(int insock, bool block);
bool arcan_pushhandle(int fd, int channel);
int arcan_sem_wait(sem_handle sem);
int arcan_sem_trywait(sem_handle sem);
#endif

struct arcan_shmif_cont;
struct arcan_event;

/*
 * Note the different semantics in return- values for _poll versus _wait
 */

/*
 * _poll will return as soon as possible with one of the following values:
 *  > 0 when there are incoming events available,
 *  = 0 when there are no incoming events available,
 *  < 0 when the shmif_cont is unable to process events (terminal state)
 */
int arcan_shmif_poll(struct arcan_shmif_cont*, struct arcan_event* dst);

/*
 * _wait will block an unspecified time and return:
 * !0 when an event was successfully dequeued and placed in *dst
 *  0 when the shmif_cont is unable to process events (terminal state)
 */
int arcan_shmif_wait(struct arcan_shmif_cont*, struct arcan_event* dst);

/*
 * When integrating with libraries assuming that a window can be created
 * synchronously, there is a problem with what to do with events that are
 * incoming while waiting for the accept- or reject to our request.
 *
 * The easiest approach is to simply skip forwarding events until we receive
 * the proper reply since windows allocations typically come during some init/
 * setup phase or as low-frequent event response. Thep problem with this is
 * that any events in between will be dropped.
 *
 * The other option is to buffer events, and then flush them out,
 * essentially creating an additional event-queue. This works EXCEPT for the
 * cases where there are events that require file descriptor transfers.
 *
 * This function implements this buffering indefinitely (or until OOM),
 * dup:ing/saving descriptors while waiting and forcing the caller to cleanup.
 *
 * The correct use of this function is as follows:
 * (send SEGREQ event)
 *
 * struct arcan_event acq_event;
 * struct arcan_event* evpool = NULL;
 *
 * if (arcan_shmif_acquireloop(cont, &acq_event, &evpool, &evpool_sz){
 * 	we have a valid segment
 *   acq_event is valid, arcan_shmif_acquire(...);
 * }
 * else {
 * 	if (!evpool){
 *    OOM
 * 	}
 * 	if (evpool_sz < 0){
 *  	shmif-state broken, only option is to terminate the connection
 *  	arcan_shmif_drop(cont);
 *  	return;
 * 	}
 *	the segment request failed
 *}
 *
 * cleanup
 * for (size_t i = 0; i < evpool_sz; i++){
 *  forward_event(&evpool[i]);
 *  if (arcan_shmif_descrevent(&evpool[i]) &&
 *  	evpool[i].tgt.ioev[0].iv != -1)
 *  		close(evpool[i].tgt.ioev[0].iv);
 * }
 *
 * free(evpool);
 *
 * Be sure to check the cookie of the acq_event in the case of a
 * TARGET_COMMAND_NEWSEGMENT as the server might have tried to preemptively
 * psuh a new subsegment (clipboard management, output, ...)
 */
bool arcan_shmif_acquireloop(struct arcan_shmif_cont*,
	struct arcan_event*, struct arcan_event**, ssize_t*);

/*
 * returns true if the provided event carries a file descriptor
 */
bool arcan_shmif_descrevent(struct arcan_event*);

/*
 * Try and enqueue the element to the queue. If the context is set to lossless,
 * enqueue may block, sleep (or spinlock).
 *
 * returns the number of FREE slots left on success or a negative value on
 * failure. The purpose of the try- approach is to let the user distinguish
 * between necessary and merely "helpful" events (e.g. frame numbers, net
 * ping-pongs etc.)
 *
 * These methods are thread-safe if and only if ARCAN_SHMIF_THREADSAFE_QUEUE
 * has been defined at build-time and not during a pending resize operation.
 */
int arcan_shmif_enqueue(struct arcan_shmif_cont*,
	const struct arcan_event* const);

int arcan_shmif_tryenqueue(struct arcan_shmif_cont*,
	const struct arcan_event* const);

/*
 * Provide a text representation useful for logging, tracing and debugging
 * purposes. If dbuf is NULL, a static buffer will be used (so for
 * threadsafety, provide your own).
 */
const char* arcan_shmif_eventstr(
	struct arcan_event* aev, char* dbuf, size_t dsz);

/*
 * Resolve implementation- defined connection connection path based on a
 * suggested key. Returns -num if the resolved path couldn't fit in dsz (with
 * abs(num) indicating the number of truncated bytes) and number of characters
 * (excluding NULL) written to dst.
 */
int arcan_shmif_resolve_connpath(
	const char* key, char* dst, size_t dsz);

/*
 * calculates a hash of the layout of the shmpage in order to detect subtle
 * compiler mismatches etc.
 */
uint64_t arcan_shmif_cookie(void);

/*
 * The following functions are simple lookup/unpack support functions for
 * argument strings usually passed on the command-line to a newly spawned
 * frameserver in a simple (utf-8) key=value\tkey=value type format.
 */
struct arg_arr {
	char* key;
	char* value;
};

/* take the input string and unpack it into an array of key-value pairs */
struct arg_arr* arg_unpack(const char*);

/*
 * return the value matching a certain key, if ind is larger than 0, it's the
 * n-th result that will be stored in dst
 */
bool arg_lookup(struct arg_arr* arr, const char* val,
	unsigned short ind, const char** found);

void arg_cleanup(struct arg_arr*);

/*
 * Duplicates a descriptor and set safe flags (e.g. CLOEXEC)
 * if [dstnum] is <= 0, it will ATTEMPT to duplicate to the specific number,
 * (though NOT GUARANTEED).
 *
 * Returns a valid descriptor or -1 on error (with errno set according
 * to the dup() call family.
 */
int arcan_shmif_dupfd(int fd, int dstnum, bool blocking);

/*
 * Part of auxiliary library, pulls in more dependencies and boiler-plate
 * for setting up accelerated graphics
 */
#ifdef WANT_ARCAN_SHMIF_HELPER

/*
 * Maintain both context and display setup. This is for cases where you don't
 * want to set up EGL or similar support yourself. For cases where you want to
 * do the EGL setup except for the NativeDisplay part, use _egl.
 *
 * [Warning] stick to either _setup OR (_egl, vk), don't mix
 *
 */
enum shmifext_setup_status {
	SHHIFEXT_UNKNOWN = 0,
	SHMIFEXT_NO_API,
	SHMIFEXT_NO_DISPLAY,
	SHMIFEXT_NO_EGL,
	SHMIFEXT_NO_CONFIG,
	SHMIFEXT_NO_CONTEXT,
	SHMIFEXT_OK
};

enum shmifext_api {
	API_OPENGL = 0,
	API_GLES,
	API_VHK
};

struct arcan_shmifext_setup {
	uint8_t red, green, blue, alpha, depth;
	uint8_t api, major, minor;
	uint64_t flags;
	uint64_t mask;
	uint8_t builtin_fbo;
	uint8_t supersample;
	uint8_t stencil;
/* workaround for versioning snafu with _setup not taking sizeof(...) */
	uint8_t uintfl_reserve[5];
	uint64_t reserved[7];
};

struct arcan_shmifext_setup arcan_shmifext_defaults(
	struct arcan_shmif_cont* con);

enum shmifext_setup_status arcan_shmifext_setup(
	struct arcan_shmif_cont* con,
	struct arcan_shmifext_setup arg);

/*
 * for use with the shmifext_setup approach, try and find the
 * requested symbol within the context of the accelerated graphics backend
 */
void* arcan_shmifext_lookup(
	struct arcan_shmif_cont* con, const char*);

/*
 * Uses lookupfun to get the function pointers needed, writes back matching
 * EGLNativeDisplayType into *display and tags *con as accelerated.
 * Can be called multiple times as response to DEVICE_NODE calls or to
 * retrieve the display associated with con
 */
bool arcan_shmifext_egl(struct arcan_shmif_cont* con,
	void** display, void*(*lookupfun)(void*, const char*), void* tag);

/*
 * For the corner cases where you need access to the display/surface/context
 * but don't want to detract from the _setup
 */
bool arcan_shmifext_egl_meta(struct arcan_shmif_cont* con,
	uintptr_t* display, uintptr_t* surface, uintptr_t* context);

/*
 * Similar to extracting the display, surface, context and manually
 * making it the current eglContext. If the setup has been called with
 * builtin- FBO, it will also manage allocating and resizing FBO.
 */
bool arcan_shmifext_make_current(struct arcan_shmif_cont* con);

/*
 * Free and destroy an associated context and internal buffers,
 * in order to stop using the connection for accelerated drawing,
 * or to rebuild- a little later without destroying the connection
 */
bool arcan_shmifext_drop(struct arcan_shmif_cont* con);

/*
 * If headless setup uses a built-in FBO configuration, this function can be
 * used to extract the opaque handles from it. These are only valid when the
 * context is active (_make_current).
 */
bool arcan_shmifext_gl_handles(struct arcan_shmif_cont* con,
	uintptr_t* frame, uintptr_t* color, uintptr_t* depth);

/*
 * Placeholder awaiting VK support
 */
bool arcan_shmifext_vk(struct arcan_shmif_cont* con,
	void** display, void*(*lookupfun)(void*, const char*), void* tag);

/*
 * Similar behavior to signalhandle, but any conversion from the texture id
 * in [tex_id] is handled internally in accordance with the last _egl
 * call on [con]. The context where tex_id is valid should already be
 * active.
 *
 * Display corresponds to the EGLDisplay where tex_id is valid, or
 * 0 if the shmif_cont is managing the context.
 *
 * If (tex_id is SHMIFEXT_BUILTIN and context was setup with FBO management,
 * the color attachment for the active FBO will be transferred).
 *
 * Returns -1 on handle- generation/passing failure, otherwise the number
 * of miliseconds (clamped to INT_MAX) that elapsed from signal to ack.
 */
#define SHMIFEXT_BUILTIN (~(uintptr_t)0)
int arcan_shmifext_signal(struct arcan_shmif_cont*,
	uintptr_t display, int mask, uintptr_t tex_id, ...);
#endif

#endif
