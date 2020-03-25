/*
 * Copyright (c) 2016-2020 WangBin <wbsecg1 at gmail.com>
 */
#pragma once
// pragma once is not standard, the same file(content) in different paths will all be included

#include <cfloat>
#include <cinttypes> //_UINT8_MAX. c++11 <cstdint> defines _STDC_LIMIT_MACROS
#include <climits> // INT_MAX
#include <functional>
#include <memory>
#include <string>
#include <variant>

#ifndef MDK_NS
#define MDK_NS mdk
#endif
#define MDK_ABI abi // add abi namespace, indicating the library for developers dependes on c++ abi
#ifdef MDK_ABI
// use inline namespace to ensure the same user code supports both abi and non-abi interfaces, e.g. using namespace MDK_NS, MDK_NS::SomeClass
# define MDK_NS_BEGIN namespace MDK_NS { inline namespace MDK_ABI {
# define MDK_NS_END }}
# define MDK_NS_PREPEND(X) ::MDK_NS::MDK_ABI::X
#else
# define MDK_NS_BEGIN namespace MDK_NS {
# define MDK_NS_END }
# define MDK_NS_PREPEND(X) ::MDK_NS::X
#endif

#define MDK_VERSION_INT(major, minor, patch) \
    (((major&0xff)<<16) | ((minor&0xff)<<8) | (patch&0xff))
#define MDK_MAJOR 0
#define MDK_MINOR 7
#define MDK_MICRO 0
#define MDK_VERSION MDK_VERSION_INT(MDK_MAJOR, MDK_MINOR, MDK_MICRO)
#define MDK_VERSION_STR MDK_STRINGIFY(MDK_MAJOR) "." MDK_STRINGIFY(MDK_MINOR) "." MDK_STRINGIFY(MDK_MICRO)
#define MDK_VERSION_CHECK(a, b, c) (MDK_VERSION >= MDK_VERSION_INT(a, b, c))

#if defined(_WIN32)
#define MDK_EXPORT __declspec(dllexport)
#define MDK_IMPORT __declspec(dllimport)
#define MDK_LOCAL
#else
#define MDK_EXPORT __attribute__((visibility("default")))
#define MDK_IMPORT __attribute__((visibility("default")))
#define MDK_LOCAL  __attribute__((visibility("hidden"))) // mingw gcc set hidden symbol in a visible class has no effect and will make it visible
#endif

#ifdef BUILD_MDK_STATIC
# define MDK_API
#else
# if defined(BUILD_MDK_LIB)
#  define MDK_API MDK_EXPORT
# else
#  define MDK_API MDK_IMPORT // default is import if nothing is defined
# endif
#endif //BUILD_MDK_STATIC
#define MDK_PRIVATE_API MDK_API

MDK_NS_BEGIN
constexpr double TimeScaleForInt = 1000.0; // ms

constexpr float IgnoreAspectRatio = 0; // stretch, ROI etc.
// aspect ratio > 0: keep the given aspect ratio and scale as large as possible inside target rectangle
constexpr float KeepAspectRatio = FLT_EPSILON; // expand using original aspect ratio
// aspect ratio < 0: keep the given aspect ratio and scale as small as possible outside renderer viewport
constexpr float KeepAspectRatioCrop = -FLT_EPSILON; // expand and crop using original aspect ratio

/*!
  \brief CallbackToken
  A callback can be registered by (member)function onXXX(callback, CallbackToken* token = nullptr). With the returned token we can remove the callback by onXXX(nullptr, token).
  Non-null callback: register a callback and return a token(if not null).
  Null callback + non-null token: can remove the callback of given token.
  Null callback + null token: clear all callbacks.
 */
using CallbackToken = uint64_t;

#if defined(_MSC_VER) && _MSC_VER < 1900
# ifdef constexpr
#  undef constexpr
# endif
# define constexpr //inline // constexpr implies inline. but we can not declare a var as inline like constexpr
#endif
/*!
 * \brief is_flag
 * if enum E is of enum type, to enable flag(bit) operators, define
 * \code template<> struct is_flag<E> : std::true_type {}; \endcode
 */
template<typename T> struct is_flag; //
template<typename T>
using if_flag = std::enable_if<std::is_enum<T>::value && is_flag<T>::value>;
template<typename E, typename = if_flag<E>>
constexpr E operator~(E e1) { return E(~typename std::underlying_type<E>::type(e1));}
template<typename E, typename = if_flag<E>>
constexpr E operator|(E e1, E e2) { return E(typename std::underlying_type<E>::type(e1) | typename std::underlying_type<E>::type(e2));}
template<typename E, typename = if_flag<E>>
constexpr E operator^(E e1, E e2) { return E(typename std::underlying_type<E>::type(e1) ^ typename std::underlying_type<E>::type(e2));}
template<typename E, typename = if_flag<E>>
constexpr E operator&(E e1, E e2) { return E(typename std::underlying_type<E>::type(e1) & typename std::underlying_type<E>::type(e2));}
// assign in constexpr requires c++14 for clang/gcc, but not msvc(2013+), so the following functions are not constexpr for now. check c++ version?
template<typename E, typename = if_flag<E>>
constexpr E& operator|=(E& e1, E e2) { return e1 = e1 | e2;}
template<typename E, typename = if_flag<E>>
constexpr E& operator^=(E& e1, E e2) { return e1 = e1 ^ e2;}
template<typename E, typename = if_flag<E>>
constexpr E& operator&=(E& e1, E e2) { return e1 = e1 & e2;}
// convenience functions to test whether a flag exists. REQUIRED by scoped enum
template<typename E>
constexpr bool test_flag(E e) { return typename std::underlying_type<E>::type(e);}
template<typename E1, typename E2>
constexpr bool test_flag(E1 e1, E2 e2) { return test_flag(e1 & e2);}
template<typename E>
constexpr bool flags_added(E oldFlags, E newFlags, E testFlags) { return test_flag(newFlags, testFlags) && !test_flag(oldFlags, testFlags);}
template<typename E>
constexpr bool flags_removed(E oldFlags, E newFlags, E testFlags) { return !test_flag(newFlags, testFlags) && test_flag(oldFlags, testFlags);}

enum class MediaType : int8_t {
    Unknown = -1,
    Video,
    Audio,
    Data, // e.g. timed metadata tracks in ffmpeg 3.2
    Subtitle,
    Attachment,
    Count
};

/*!
  \brief The MediaStatus enum
  Defines the io status of a media stream,
  Use flags_added/removed() to check the change, for example buffering after seek is Loaded|Prepared|Buffering, and changes to Loaded|Prepared|Buffered when seek completed
 */
enum MediaStatus
{
    NoMedia = 0, // initial status, not invalid. // what if set an empty url and closed?
    Unloaded = 1, // unloaded // (TODO: or when a source(url) is set?)
    Loading = 1<<1, // opening and parsing the media
    Loaded = 1<<2, // media is loaded and parsed. player is stopped state. mediaInfo() is available now
    Prepared = 1<<8, // all tracks are buffered and ready to decode frames. tracks failed to open decoder are ignored
    Stalled = 1<<3, // insufficient buffering or other interruptions (timeout, user interrupt)
    Buffering = 1<<4, // when buffering starts
    Buffered = 1<<5, // when buffering ends
    End = 1<<6, // reached the end of the current media, no more data to read
    Seeking = 1<<7,
    Invalid = 1<<31, //failed to load media because of unsupport format or invalid media source
};
template<> struct is_flag<MediaStatus> : std::true_type {};
// MediaStatusCallback

/*!
  \brief The State enum
  Current playback state. Set/Get by user
 */
enum class State : int8_t {
    NotRunning,
    Stopped = NotRunning,
    Running,
    Playing = Running, /// start/resume to play
    Paused,
};
typedef State PlaybackState;

enum class SeekFlag {
    /// choose one of FromX
    From0       = 1,    /// relative to time 0
    FromStart   = 1<<1, /// relative to media start position
    FromNow     = 1<<2, /// relative to current position, the seek position can be negative
    Byte        = 1<<5, // internal only?
    /// combine the above values with one of the following
    KeyFrame    = 1<<8, // fast key-frame seek, forward if Backward is not set. If not set, it's accurate seek but slow, implies backward seek internally
    Fast        = KeyFrame,
    AnyFrame    = 1<<9, // fast, broken image if video format has key frames. TODO: remove? broken

    // Useful if seek backward repeatly, .i.e. target < playback(not buffered) position. result positions may be the same repeatly if seek forward w/ this flag, or seek backward w/o this flag
    Backward    = 1<<16, // for KeyFrame seek only. NOTE: FrameReader/PacketIO only. It has no effect to (un)set this flag in MediaControl/MediaPlayer and higher level apis
    /// default values
    Default     = KeyFrame|FromStart
};
template<> struct is_flag<SeekFlag> : std::true_type {};

/*!
  \brief javaVM
  Set/Get current java vm
  \param vm null to get current vm
  \return vm before set
 */
// TODO: mdk/ugl/ugs set/getGlobal(): XDisplay, JavaVM. use std::any or void*?
MDK_API void* javaVM(void* vm = nullptr);

using OptionVal = std::variant<std::monostate, int, int64_t, float, double, void*, std::string>;
MDK_API OptionVal SetGlobalOption(const char* key, OptionVal value);
MDK_API OptionVal GetGlobalOption(const char* key);

/*!
  \brief OptionListener
  \return true if is processed and should stop dispatching.
 */
using OptionListener = std::function<bool(const char*, OptionVal)>;
MDK_API void OnGlobalOptionChanged(OptionListener cb, CallbackToken* token = nullptr);

enum LogLevel {
    Off,
    Error,
    Warning,
    Info,
    Debug,
    All
};
MDK_API void setLogLevel(LogLevel value);
MDK_API LogLevel logLevel();
/* \brief setLogHandler
  if log handler is not set, i.e. setLogHandler() was not called, log is disabled.
  if set to non-null handler, log will be passed to the handler.
  if previous handler is set by user and not null, then call setLogHandler(nullptr) will print to stderr, and call setLogHandler(nullptr) again to silence the log
*/
MDK_API void setLogHandler(std::function<void(LogLevel, const char*)>);

/*
  events:
  {timestamp(ms), "render.video", "1st_frame"}: when the first frame is rendererd
  {error, "decoder.audio/video/subtitle", "open", stream}: decoder of a stream is open, or failed to open if error != 0. TODO: do not use "open"?
  {progress 0~100, "reader.buffering"}: error is buffering progress
  {0/1, "thread.audio/video/subtitle", stream}: decoder thread is started (error = 1) and about to exit(error = 0)
  {error, "snapshot", saved_file if no error and error string if error < 0}
*/
class MediaEvent {
public:
    int64_t error = 0; // result <0: error code(fourcc?). >=0: special value depending on event
    std::string category;
    std::string detail; // if error, detail can be error string

    union {
        struct {
            int stream;
        } decoder;
    };
};
/*!
  \brief MediaEventListener
  \return true if event is processed and should stop dispatching.
 */
using MediaEventListener = std::function<bool(const MediaEvent&)>;

static const int64_t kTimeout = 10000;
/*!
  \brief TimeoutCallback
  \param ms elapsed milliseconds
  \return true to abort current operation on timeout.
  A null callback can abort current operation.
  Negative timeout infinit.
  Default timeout is 10s
 */
using TimeoutCallback = std::function<bool(int64_t ms)>;

/*!
  \brief PrepareCallback
  \param position in callback is the actual position, or <0 (TODO: error code as position) if prepare() failed.
  \param boost in callback can be set by user to boost the first frame rendering
  \return false to unload media immediately when media is loaded and MediaInfo is ready, true to continue.
    example: always return false can be used as media information reader
 */
using PrepareCallback = std::function<bool(int64_t position, bool* boost)>;

MDK_NS_END

constexpr uint32_t operator"" _fourcc(const char* name, size_t len) {
    return uint32_t(name[0]) | (uint32_t(name[1]) << 8) | (uint32_t(name[2]) << 16) | (uint32_t(name[3]) << 24);
}

#if (_MSC_VER + 0) >= 1500
# define VC_NO_WARN(number)       __pragma(warning(disable: number))
#else
# define VC_NO_WARN(number)
#endif

VC_NO_WARN(4251) /* class 'type' needs to have dll-interface to be used by clients of class 'type2' */

#if defined(_MSC_VER)
#define MDK_FUNCINFO __FUNCSIG__
#define MDK_NOVTBL __declspec(novtable)
#else
#define MDK_FUNCINFO __PRETTY_FUNCTION__
#define MDK_NOVTBL
#endif
#define MDK_STRINGIFY(X) _MDK_STRINGIFY(X)
#define _MDK_STRINGIFY(X) #X

#ifdef WINAPI_FAMILY
# include <winapifamily.h>
# if !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#   define MDK_WINRT 1
# endif
#endif
