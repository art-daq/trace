TRACE_NAME — How TRACE/TLOG names are formed
=================================================

Summary
-------
- **TRACE/TLOG name**: the identifier associated with a TRACE entry (the "name" used to index/lookup per-name trace levels).
- The name used by `TLOG` / `TRACEN` is derived from (in precedence order):
  1. An explicit non-empty name passed to `TLOG`/`TRACEN`.
  2. The `TRACE_NAME` specification (macro defined in code) if present.
  3. The `TRACE_NAME` environment variable if set and non-empty.
  4. The compile-time default `TRACE_DFLT_NAME` (`%f %H`) when none of the above are provided.

Precedence and behavior
------------------------
- If a `TLOG`/`TRACEN` call provides a name argument which is non-NULL and non-empty, that string is taken verbatim and immediately passed to `trace_name2TID()` (see code: [include/trace.h](include/trace.h#L1459)).
- If no explicit name is provided (or the provided name is the empty string `""`), the static helper `trace_tlog_name_()` builds the name using the `TRACE_NAME` macro or `TRACE_NAME` environment variable and then calls `trace_name2TID()` with the constructed name.
- For calls made from the program's main/base file (where `base_file == FILEp`), `trace_tlog_name_()` will reuse the already-determined `traceTID` when available (fast path).
- For calls made from included/header files, `trace_tlog_name_()` decides which part of the `TRACE_NAME` specification to use (see below). See implementation at [include/trace.h](include/trace.h#L1459-L1501).

The `TRACE_NAME` specification (format string)
---------------------------------------------
- `TRACE_NAME` is not just a literal name in many cases; it is a small format/specification language interpreted by `trace_name_path()` (see [include/trace.h](include/trace.h#L1265)).
- If `TRACE_NAME` contains a space, it is treated as two parts: the first part is used for the "base/main" file name and the second part is used for included/header files. The default `TRACE_DFLT_NAME` is `%f %H`.
- Recognized percent sequences (summary):
  - `%%` — a literal `%`.
  - `%f` / `%h` — filename (file/hdrf) without extension.
  - `%F` / `%H` — filename (file/hdrf) with extension.
  - `%[-][0-9]f` — include additional path components (a digit 0–9 after `%` will include that many extra path components; a leading `-` can affect extension handling).
  - `~` / `^` followed by a needle — search for the needle in the path and use the remainder (search delimiter-aware).
  - `%p` — program name (`__progname`) when compiled with `TRACE_DO__PROGNAME` enabled.

See the `trace_name_path()` implementation for the full parsing rules and corner cases: [include/trace.h](include/trace.h#L1258-L1510).

Examples
--------
- Default (`TRACE_DFLT_NAME = "%f %H"`):
  - In the main/base file the produced name will typically be the base filename without extension (`%f`).
  - In included/header files the produced name will typically be the header filename with extension (`%H`).
- `export TRACE_NAME="%p-%-f"` will attempt to prefix the name with the program name and then the base filename without extension (if `%p` is supported at compile time).
- If you explicitly call `TRACEN("myname", TLVL_INFO, "...")`, the name `"myname"` is used verbatim.

Short examples
--------------

1) Define-before-include (recommended — avoids fallback symbol):

```c
/* Define module default TRACE_NAME, then include headers */
#define TRACE_NAME "%p-%2f %~src/%H"
#include "trace.h"

int main(void) {
    TLOG_INFO() << "uses programname-2pathcomponents"; /* expands using TRACE_NAME */
}
```

Explanation: the spec is two-part by space; the first part (`%p-%2f`) is used for the program/base file (prefix with program name and include two path components), the second part (`%~src/%H`) searches for `src/` in the header path and prints the remainder plus the header filename with extension.

2) Temporary define/undef around a block of uses (valid when done carefully):

```c
#include "trace.h"

/* Temporarily override TRACE_NAME for the next block of TLOG/TRACEN uses */
#define TRACE_NAME "%p-%-f %H"
TLOG_INFO() << "temporary name used here";
TRACEN("explicit", TLVL_INFO, "explicit used verbatim");
#undef TRACE_NAME

/* Back to default / env-controlled behavior */
TLOG_INFO() << "default name used here";
```

3) Explicit name passed to `TRACEN` (always verbatim):

```c
#include "trace.h"

TRACEN("my_module,subpart", TLVL_INFO, "message: %d", 42);
```

Notes about these examples
- Two-part `TRACE_NAME` specs (space-separated) allow different naming for base files vs headers; include interesting percent sequences like `%p`, `%2f`, `%-f`, and `~` to tailor names.
- The temporary `#define`/`#undef` pattern works because macros are expanded at the point of use; still prefer define-before-include for clarity and to avoid the fallback `static const char *TRACE_NAME = NULL;` symbol.

Sanitization and insertion
--------------------------
- `trace_name2TID()` looks up a name in the trace-name table and inserts it if absent. Non-printable or non-graph characters are replaced with `_` before insertion; long names are truncated from the left so the trailing/right-hand part (often the most significant part of a path) is preserved. See implementation: [include/trace.h](include/trace.h#L2526-L2632).
- If the trace-name table is full, the function will return the reserved slot (last slot) rather than inserting.

Notes
-----
- An explicit empty string (`""`) passed as a name is treated as "not provided"; the header/path-based logic will be used instead.
- You can define `TRACE_NAME` in source before including the headers to customize per-module default name behavior. If `TRACE_NAME` is not defined by the build or source, a static `const char *TRACE_NAME = NULL;` is provided and the environment/default is used instead (see [include/trace.h](include/trace.h#L618-L626)).
- Because `trace_tlog_name_()` may call `getenv("TRACE_NAME")` for header-file calls, there is a small runtime cost for header-local TLOG/TRACEN instances when `TRACE_NAME` isn't statically provided.

Ordering and recommendation
-------------------------
- Macro expansion for `TLOG`/`TRACEN` happens at the site where those macros are used (not when `trace.h` is parsed). That means a `#define TRACE_NAME ...` placed after `#include "trace.h"` will still affect subsequent uses of the `TLOG`/`TRACEN` macros in the same translation unit.
- However, if `TRACE_NAME` was not defined at include-time the header emits a fallback `static const char *TRACE_NAME = NULL;`. Defining a preprocessor macro with the same name later is permitted but can be confusing; to avoid ambiguity and the fallback symbol, prefer defining `TRACE_NAME` before including `trace.h` in a translation unit (or use the `TRACE_NAME` environment variable).

See also
--------
- `trace_tlog_name_()` implementation: [include/trace.h](include/trace.h#L1459)
- `trace_name_path()` implementation: [include/trace.h](include/trace.h#L1258)
- `trace_name2TID()` implementation: [include/trace.h](include/trace.h#L2526)

