# C Templates

A C based template engine, with C as the template language.
I.e., create templates with C code embedded in it, that can
be expanded at runtime.

See `tests/` directory. At a basic level, `ctpl` can be used
as an extended `sprintf` where the host program's variables
can be interpolated in the template [string] - anything between
the `@{` and `@}` tags. But any valid C code can go into the
template - comments, declarations ...

The `_()` function does the actual interpolation. There are
convenience macros like `NL` and `TAB` to insert newline and
tabs respectively.

There are 3 parts to using `ctpl`:

### Define a template

Template is any C string, with parts to be expanded under
`@{` and `@}` - there can be many such blocks.

### Initialize

Call `ctpl_init()` with a buffer that can hold expanded template.

### Expand

Call `ctpl_expand()`, passing as arg, template and _all the variables/symbols_ used in the
template (from the host program). This is where `ctpl` differs from other template engines
which use dynamic/reflective languages!
If the variable is not a basic C data-type, then `ctpl_custom_type()` call is needed. Also,
the user defined type should be in a separate _header file_ that should be given to `ctpl`.

Once expanded, the user provided buffer will have expanded string.
`ctpl_status()` can be used to check if we succeeded or not.

## Dependency

Packages to be installed on the host: `libtcc-dev`
