Samizdat Layer 0: Core Library
==============================

I/O
---

<br><br>
### Primitive Definitions

#### `io0Die stringlet? <> ~. (exits)`

Prints the given stringlet to the system console (as if with `io0Note`)
if supplied, and terminates the runtime with a failure status code (`1`).

#### `io0Note stringlet <> ~.`

Writes out a newline-terminated note to the system console or equivalent.
This is intended for debugging, and as such this will generally end up
emitting to the standard-error stream.

#### `io0PathFromStringlet stringlet <> pathListlet`

Converts the given path stringlet to an absolute form, in the "form factor"
that is used internally. The input `stringlet` is expected to be a
"Posix-style" path:

* Path components are separated by slashes (`"/"`).

* A path-initial slash indicates an absolute path.

* If there is no path-initial slash, then the result has the system's
  "current working directory" prepended.

* Empty path components (that is, if there are two slashes in a row)
  are ignored.

* Path components of value `@"."` (canonically representing "this directory")
  are ignored.

* Path components of value `@".."` cause the previous non-`..` path component
  to be discarded. It is invalid (terminating the runtime) for such a
  component to "back up" beyond the filesystem root.

The result is a listlet of path components, representing the absolute path.
None of the components will be the empty stringlet (`@""`), except possibly
the last. If the last component is empty, that is an indication that the
original path ended with a trailing slash.

#### `io0ReadFileUtf8 pathListlet <> stringlet`

Reads the named file, using the underlying OS's functionality,
interpreting the contents as UTF-8 encoded text. Returns a stringlet
of the read and decoded text.

`pathListlet` must be a listlet of the form described by `io0PathFromStringlet`
(see which). It is invalid (terminating the runtime) for a component to
be any of `@""` `@"."` `@".."` or to contain a slash (`/`).

#### `io0ReadLink pathListlet <> pathListlet | ~.`

Checks the filesystem to see if the given path refers to a symbolic
link. If it does, then this returns the path which represents the
direct resolution of that link. It does not try to re-resolve
the result iteratively, so the result may not actually refer to a
real file (for example).

If the path does not refer to a symbolic link, then this function returns
void.

`pathListlet` must be a listlet of the form described by `io0PathFromStringlet`
(see which). See `io0ReadFileUtf8` for further discussion.

#### `io0WriteFileUtf8 pathListlet text <> ~.`

Writes out the given text to the named file, using the underlying OS's
functionality, and encoding the text (a stringlet) as a stream of UTF-8 bytes.

`pathListlet` must be a listlet of the form described by `io0PathFromStringlet`
(see which). See `io0ReadFileUtf8` for further discussion.



<br><br>
### In-Language Definitions


#### `io0SandboxedReader directory <> function`

Returns a file reader function which is limited to *only* reading
files from underneath the named directory (a path-listlet as
described in `io0PathFromStringlet`). The return value from this call
behaves like `ioReadFileUtf8`, as if the given directory is both the
root of the filesystem and is the current working directory. Symbolic
links are respected, but only if the link target is under the named
directory.

This function is meant to help enable a "supervisor" to build a sandbox
from which untrusted code can read its own files.