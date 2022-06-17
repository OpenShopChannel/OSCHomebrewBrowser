# OSCHomebrewBrowser
The [Open Shop Channel](https://oscwii.org) version of the Homebrew Browser.

#### Planned / TODO:
- [x] More meaningful errors
- [x] New background graphics
- [ ] Nicer dpad scrolling
- [x] Truncate long descriptions to fit / Implement scrolling if plausible (Currently, descriptions longer than the box size result in visual bugs.)
- [ ] Look into common DSI exceptions.
- [ ] More new graphics, possibly

## Building
1. Install [devkitPro](https://devkitpro.org/wiki/Getting_Started),  a toolchain for building Wii Homebrew applications. 
2. Install the `wii-dev` package via `pacman`.
3. Clone and install the latest [GRRLIB via its instructions](https://github.com/grrlib/grrlib#installing-grrlib).
4. Install the latest versions of `libwiisocket`, `mbedtls`, and `libcurl` from [4TU's wii-packages](https://gitlab.com/4TU/wii-packages/-/tags).
    1. Use the latest available tag, if possible. Packages are attached as release files.
    2. `pacman -U <filename>` to manually install.
5. Run `make`, and enjoy!