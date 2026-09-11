Building projectM from source
=============================

Since version 4.0, projectM uses CMake to configure and build the library on all platforms.

This guide contains both the quick-start build instructions and the in-depth CMake reference. For WebAssembly
builds, see :doc:`emscripten`.

Quick Start (Debian / Ubuntu)
-----------------------------

For other operating systems (Windows/macOS), see the OS-specific sections below.

Install the build tools and dependencies
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Mandatory packages:

.. code:: bash

   sudo apt install build-essential cmake libgl1-mesa-dev mesa-common-dev

**Important:** projectM requires CMake 3.21 or newer. Current stable distributions already ship a new enough
CMake (e.g. Debian 12 "bookworm" with CMake 3.25, Ubuntu 22.04 and newer with CMake 3.22+). Only end-of-life releases
like Debian 11 "bullseye" (CMake 3.18) need a manual install, in which case get the latest CMake
release `from Kitware's download page <https://cmake.org/download/>`__.

Optional packages:

.. code:: bash

   sudo apt install ninja-build # To build projectM with Ninja instead of make
   sudo apt install libsdl2-dev # To build the development test UI

Download the projectM sources
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

For production use, it is highly recommended to use the latest stable version of projectM. Download the latest release
from the `Releases page on GitHub <https://github.com/projectM-visualizer/projectm/releases>`__ and unpack it.

Using a development version
^^^^^^^^^^^^^^^^^^^^^^^^^^^

If you prefer a bleeding-edge version or want to modify the code, clone the Git repository and initialize any
submodules:

.. code:: bash

   sudo apt install git # Probably already installed
   git clone https://github.com/projectM-visualizer/projectm.git /path/to/local/repo
   cd /path/to/local/repo
   git fetch --all --tags
   git submodule init
   git submodule update

Build and install projectM
~~~~~~~~~~~~~~~~~~~~~~~~~~

Replace ``/usr/local`` with your preferred installation prefix.

Configure the project using CMake
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code:: bash

   sudo apt install cmake
   mkdir build
   cd build
   cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local ..

To generate Ninja scripts instead of Makefiles, add ``-GNinja`` to the above command.

Build and install
^^^^^^^^^^^^^^^^^

These commands will build projectM and install it to /usr/local or the configured installation prefix set in the step
before:

.. code:: bash

   cmake --build . -- -j && sudo cmake --build . --target install

**Note**: You won't need to use ``sudo`` if the install prefix is writeable by your non-privileged user.

Test projectM
^^^^^^^^^^^^^

The SDL-based developer test UI is off by default. To build it, add ``-DENABLE_SDL_UI=ON`` to the configure command
above (requires SDL2, e.g. ``sudo apt install libsdl2-dev``). It is not installed; after building, run it from the build
directory, e.g. ``./src/sdl-test-ui/projectM-Test-UI``.

Dependencies
------------

Depending on the OS/distribution and packaging system, libraries might be split into separate packages with binaries and
development files. To build projectM, both binaries and development files need to be installed.

General build dependencies for all platforms:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

* A working build toolchain (compiler, linker).
* CMake 3.21 or higher.
* **OpenGL**: 3D graphics library. Used to render the visualizations.
* **GLES3**: OpenGL libraries for embedded systems, version 3. Required to build projectM on mobile devices, Raspberry
  Pi and Emscripten.
* `glm <https://github.com/g-truc/glm>`__:  OpenGL Mathematics library. It is optional, projectM will use a bundled
  version if not installed.
* `SDL2 <https://github.com/libsdl-org/SDL>`__: Simple Directmedia Layer. Version 2.0.5 or higher is required to build
  the development test application in the build directory.
* `vcpkg <https://github.com/microsoft/vcpkg>`__: C++ Dependency Manager. Optional, but recommended to install
  library dependencies and/or using CMake to configure the build. Mainly used on Windows, but also works for Linux and
  macOS.

Building on Linux and macOS
---------------------------

Installing dependencies
~~~~~~~~~~~~~~~~~~~~~~~

- Linux distributions will have packages available for most (if not all) required libraries. The package names and
  commands to install them vary widely between distributions (and even versions of the same distribution). Please refer
  to the documentation of your build OS on how to find and install the required libraries.
- On *BSD*, install the appropriate Ports with ``pkg install``.
- On macOS, a working Xcode installation is required. CMake (and libSDL2 for the test UI) can be installed
  using `Homebrew <https://brew.sh/>`__.

Building
~~~~~~~~

The steps documented below are a bare minimum quickstart guide on how to build and install the project. If you want to
configure the build to your needs, require more in-depth information about the build process and available tweaks, or on
how to use libprojectM in your own CMake-based projects, see the `Building with CMake`_ reference section below.

Using CMake is the only supported way of building projectM since version 4.0. CMake is a platform-independent tool that
is able to generate files for multiple build systems and toolsets while using only a single set of build instructions.

Building the project with CMake requires two steps:

- Configure the build and generate project files.
- Build and install the project using the selected build tools.

**Note:** The build directory should always be different from the source directory. Generating the build files directly
inside the source tree is possible, but strongly discouraged. Using a subdirectory, e.g. ``cmake-build`` inside the source
directory is fine though.

This documentation only covers project-specific information. CMake is way too versatile and feature-rich to cover any
possible platform- and toolset-specific configuration details here. If you are not experienced in using CMake, please
first read the `official CMake documentation <https://cmake.org/cmake/help/latest/>`__ (at least
the `User Interaction Guide <https://cmake.org/cmake/help/latest/guide/user-interaction/index.html>`__) for basic usage
instructions.

Configure the build
^^^^^^^^^^^^^^^^^^^

Configuring a non-debug build with default options and install prefix (``/usr/local``) can be done with these commands,
building in a subdirectory inside the source directory:

.. code:: shell

   cd /path/to/source
   mkdir cmake-build
   cd cmake-build
   cmake -DCMAKE_BUILD_TYPE=Release ..

CMake will check all required dependencies and display any errors. If configuration was successful, a summary of the
build configuration is printed and CMake should display a ``Generating done`` line. The project is now ready to build.

Compile and install the project
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Depending on your generator choice, you can use your selected toolset as usual to build and install projectM:

- With ``Unix Makefiles``, run ``make && sudo make install``.
- With ``Ninja``, run ``ninja && sudo ninja install``.
- With ``Xcode``, select the appropriate target and configuration in Xcode and build it, or ``INSTALL`` to install the
  project.

You can also use CMake's build mode to run the selected toolset and build any specified target. CMake knows which
command to call and which parameters to pass, so the syntax works on all platforms with all generators. If you've
already set the top-level build directory as working directory, simply pass ``.`` as ``/path/to/build/dir``:

.. code:: shell

   cmake --build /path/to/build/dir --config Release
   sudo cmake --build /path/to/build/dir --config Release --target install

If you don't need root permissions to install to the given path, running the second command without ``sudo`` is
sufficient.

To control how many files are compiled in parallel, pass the ``--parallel N`` argument to the build command, with ``N``
being the number of concurrent compile commands. If not given explicitly, the build processor (make, ninja, MSBuild,
xcodebuild etc.) will decide the limit, which may range from one file at a time to all files at once.

If you want to provide arguments directly to the toolset command, add ``--`` at the end of the CMake command line followed
by any additional arguments. CMake will pass these *unchanged and unchecked* to the subcommand:

.. code:: shell

   cmake --build /path/to/build/dir --config Release -- <additional build tool args>

Building on Windows
-------------------

To build the projectM library and the SDL-based standalone application, CMake must be used to create the project files
first. Using vcpkg to pull in the build dependencies is highly recommended, as CMake can't use NuGet (NuGet pulls in
dependencies using the project files, while CMake requires the libraries before creating the project files).

Installing the dependencies with vcpkg
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

As stated above, using vcpkg is the easiest way to get the required dependencies. First,
install `vcpkg from GitHub <https://github.com/microsoft/vcpkg>`__ by following the official guide.

We've included a vcpkg manifest file in the repository root, designed to automatically install dependencies when you
create your solution.

Creating the Visual Studio solution
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

CMake provides separate generators for different Visual Studio versions. Newer CMake versions will support recent Visual
Studio releases, but may remove generators for older ones. To get a list of available generators from the command line,
use the ``-G`` switch without an argument. The CMake GUI will present you a dropdown list you can easily select from.

To set the build architecture in Visual Studio builds, use the ``-A`` switch and specify either ``Win32`` or ``X64`` as the
argument. If you want to build for both architectures, create separate build directories and configure them accordingly.

To make CMake aware of the installed vcpkg packages, simply use the provided toolchain file when configuring the
projectM build by
pointing `CMAKE_TOOLCHAIN_FILE <https://cmake.org/cmake/help/latest/variable/CMAKE_TOOLCHAIN_FILE.html>`__ to it.

Here is a full command line example to create a Visual Studio 2022 solution for X64:

.. code:: console

   cmake -G "Visual Studio 17 2022" -A "X64" -DCMAKE_TOOLCHAIN_FILE="<path to vcpkg>/scripts/buildsystems/vcpkg.cmake" -S "<path to source dir>" -B "<path to build dir>"

If you use the CMake GUI, check the "Specify toolchain file for cross-compiling" option in the first page of the
configuration assistant, then select the above ``vcpkg.cmake`` file on the second page.

Another option is to open the project folder in a recent Visual Studio version as a CMake project and configure CMake
using Visual Studio's JSON-based settings file.

Building the solution
~~~~~~~~~~~~~~~~~~~~~

To build the project, open the generated solution in Visual Studio and build it like any other solution. Each time the
CMake files are changed, Visual Studio will automatically regenerate the CMake build files and reload the solution
before continuing the build. Be aware that in old Visual Studio versions (2015 and earlier) the reload-and-continue
might not work properly.

You can also build the solution with msbuild via the command line, or use CMake's build wrapper to do that for you:

.. code:: console

   cmake --build "<path to build dir>" --config Release

Using Ninja to build
~~~~~~~~~~~~~~~~~~~~

The Ninja build system is shipped with Visual Studio since version 2019 and used by default if loading a CMake project
directly from within the IDE. Ninja can also be `installed separately <https://github.com/ninja-build/ninja/releases>`__.

To configure the build directory for Ninja, pass ``Ninja`` or ``Ninja Multi-Config`` as the argument for the ``-G`` switch.
The difference between both generators is that the former uses ``CMAKE_BUILD_TYPE`` to specify the configuration (
e.g. ``Debug`` or ``Release``) while the latter supports all configurations in a single build directory, specified during
build time.

The architecture is determined from the toolset, so make sure to run the commands in the correct Visual Studio command
prompt, e.g. "Native Tools for X64".

Configure and build for a single-configuration Release build with vcpkg:

.. code:: console

   cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE="<path to vcpkg>/scripts/buildsystems/vcpkg.cmake" -S "<path to source dir>" -B "<path to build dir>"
   cmake --build "<path to build dir>"

Same, but using the multi-configuration generator:

.. code:: console

   cmake -G "Ninja Multi-Config" -DCMAKE_TOOLCHAIN_FILE="<path to vcpkg>/scripts/buildsystems/vcpkg.cmake" -S "<path to source dir>" -B "<path to build dir>"
   cmake --build "<path to build dir>" --config Release

Notes on other platforms and features
-------------------------------------

Supported platforms
~~~~~~~~~~~~~~~~~~~

libprojectM is designed to be compatible with as many platforms and architectures as possible. Thus, the code base
doesn't use any non-portable code like OS-specific APIs or assembly instructions.

The code compiles for the following target architectures:

- i686 (x32 / x86)
- x86_64 (x64)
- armv7 (arm)
- aarch64 (arm64 / armv8)
- WASM

Other architectures such as PPC, RISC and MIPS should be supported as well. Big-endian support is untested and thus
might be buggy.

libprojectM builds at least on the following platforms:

- Windows
- Linux
- macOS (iOS/tvOS as well, but be aware that Apple's TOS may prevent its use in Store apps!)
- BSD derivatives
- Android
- WebGL/WASM (using emscripten)

As long as a platform provides OpenGL Core 3.3 or OpenGL ES 3.2 libraries and supports at least C++ 14, libprojectM
should build fine.

Raspberry Pi (and other embedded systems)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Make sure to enable GLES support by passing ``-DENABLE_GLES=TRUE`` to CMake's configuration command and install the
appropriate OS libraries. Otherwise, follow the Linux build instructions above.

For applications using projectM, it's highly recommended to keep certain performance-related settings low, such as:

- The per-point mesh resolution should be kept low, e.g. 48x32
- The rendering resolution should not exceed 720p on the Pi, though most presets will also run at ~60 FPS in 1080p

Build using NDK for Android
~~~~~~~~~~~~~~~~~~~~~~~~~~~

To build projectM using the Android SDK, please refer to the official NDK docs:

https://developer.android.com/ndk/guides/cmake

It is highly recommended using the latest NDK and CMake >= 3.21 for building.

Using libprojectM with pkgconfig
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Some UNIX build systems cannot use CMake config packages, like GNU autotools. To use libprojectM with such build
systems, projectM's build system also creates basic ``.pc`` files during the installation process.

Note that the resulting pkgconfig files will not necessarily work in all circumstances, because they are much less
flexible than CMake (or Meson). When using pkgconfig, some required libraries will probably not be linked
automatically (e.g. OpenGL libraries) and have to be added manually depending on the application needs.

Frontends and audio capturing
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Previous projectM versions (before 4.0) shipped several UIs and audio capture implementations in this repository.
To reduce the size of the code base and separate the release cycles of the core library and the frontends, these were
moved into their own repositories under the `projectM-visualizer organization <https://github.com/projectM-visualizer>`__
(e.g. the SDL-based desktop app and the Qt-based desktop app with PipeWire/PulseAudio/JACK backends).

libprojectM on its own does not have any means of capturing audio, it simply takes PCM data via the API. Applications
using projectM can supply this data from their own audio sources, e.g. music playing in an audio player or capturing
audio data from OS or other external sources.

.. _Building with CMake:

Building with CMake
-------------------

This section contains in-depth information for building with the CMake build system.

To use the library in other projects, it is required to install it. Use ``CMAKE_INSTALL_PREFIX`` to specify the
installation directory. If you use other build systems, you have to specify the include and library paths manually.

Selecting a specific project file generator
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Building libprojectM does not require any specific CMake generator. It should work with any available generator, single-
and multi-config.

To specify a CMake generator, use the ``-G`` switch, followed by the generator name. Some newer generators take an
additional architecture using the ``-A`` switch. To list all available generators available on your current platform,
leave out the generator name:

.. code:: shell

   cmake -G

Additional information on the supported generators can be
found `in the CMake documentation <https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html>`__.

Popular generators
^^^^^^^^^^^^^^^^^^

By default, CMake will use the `Unix Makefiles <https://cmake.org/cmake/help/latest/generator/Unix%20Makefiles.html>`__
generator on Linux and macOS, which is a good choice and should work. Yet in some circumstances, you might want to
generate project files for a specific build tool or IDE:

.. code:: shell

   cmake -G "Unix Makefiles" -S /path/to/source/dir -B /path/to/build/dir

A common alternative is the `Ninja <https://cmake.org/cmake/help/latest/generator/Ninja.html>`__ generator, which
requires ``ninja`` to be installed. It is mostly a ``make``
replacement with less overhead and should work equally well. It is supported on all major platforms, including Windows:

.. code:: shell

   cmake -G Ninja -S /path/to/source/dir -B /path/to/build/dir

On macOS, CMake also supports the `Xcode <https://cmake.org/cmake/help/latest/generator/Xcode.html>`__ generator. It will
create an ``.xcodeproj`` bundle which you can open in Xcode. It also adds support for automatic code signing, which might
be required if your application using projectM needs to be notarized for store deployment.

.. code:: shell

   cmake -G Xcode -S /path/to/source/dir -B /path/to/build/dir

If you develop on Windows, you will possibly use Visual Studio. While recent visual Studio versions have CMake support
built-in, you can still pre-generate the solution and project files and open the ``.sln`` file from the build directory.
CMake provides a separate generator for each Visual Studio release. For Visual Studio 2022 you would use
the `Visual Studio 17 2022 <https://cmake.org/cmake/help/latest/generator/Visual%20Studio%2017%202022.html>`__ generator
and provide an additional architecture parameter:

.. code:: shell

   cmake -G "Visual Studio 17 2022" -A "X64" -S /path/to/source/dir -B /path/to/build/dir

It is not possible to generate multi-arch solutions with CMake though. You need to create separate build directories and
use the respective ``-A`` switch for each.

Project-specific configuration options
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

CMake has no built-in way of printing all available configuration options. You can either refer to the
top-level ``CMakeLists.txt`` which contains a block of ``option`` and ``cmake_dependent_option`` commands, or use one of the
available CMake UIs which will display the options after configuring the project once.

Important build switches
^^^^^^^^^^^^^^^^^^^^^^^^

The following table also gives you an overview of important build options and their defaults. All options accept a
boolean
value (``YES``/``NO``, ``TRUE``/``FALSE``, ``ON``/``OFF`` or ``1``/``0``) and can be provided on the configuration-phase command line
using the ``-D`` switch.

.. list-table::
   :header-rows: 1

   * - CMake option
     - Default
     - Required dependencies
     - Description
   * - ``BUILD_TESTING``
     - ``OFF``
     -
     - Builds the unit tests.
   * - ``BUILD_DOCS``
     - ``OFF``
     - ``Doxygen``, ``Sphinx``
     - Builds the API/developer documentation in ``docs/``.
   * - ``BUILD_SHARED_LIBS``
     - ``ON``
     -
     - Build projectM as shared libraries. If ``OFF``, build static libraries.
   * - ``ENABLE_PLAYLIST``
     - ``ON``
     -
     - Builds and installs the playlist library.
   * - ``ENABLE_EMSCRIPTEN``
     - (auto)
     - ``Emscripten``
     - Forced ``ON`` when building with the Emscripten toolchain, otherwise ``OFF``. Not user-settable.
   * - ``ENABLE_GLES``
     - ``OFF``
     - ``GLES``
     - Use OpenGL ES 3 profile for rendering instead of the Core profile.

Note that ``ENABLE_GLES`` will be forcibly set to ``ON`` for Emscripten and Android builds, making it mandatory.

Experimental and application-dependent build switches
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The following table contains a list of build options which are only useful in special circumstances, e.g. when
developing libprojectM, trying experimental features or building the library for a special use-case/environment.

.. list-table::
   :header-rows: 1

   * - CMake option
     - Default
     - Required dependencies
     - Description
   * - ``ENABLE_SDL_UI``
     - ``OFF``
     - ``SDL2``
     - Builds the SDL-based test application. Only used for development testing, will not be installed.
   * - ``ENABLE_INSTALL``
     - ``ON`` (top-level) / ``OFF`` (subproject)
     -
     - Enable projectM install targets. Automatically ``ON`` when built standalone, ``OFF`` when added via ``add_subdirectory()``.
   * - ``ENABLE_DEBUG_POSTFIX``
     - ``ON``
     -
     - Adds ``d`` (by default) to the name of any binary file in debug builds.
   * - ``ENABLE_SYSTEM_GLM``
     - ``OFF``
     -
     - Builds against a system-installed GLM library.
   * - ``ENABLE_SYSTEM_PROJECTM_EVAL``
     - ``ON``
     -
     - Uses a system-installed projectM-eval library if found, otherwise the bundled sources in ``vendor/``.
   * - ``ENABLE_BOOST_FILESYSTEM``
     - ``OFF``
     - ``Boost``
     - Forces the use of ``boost::filesystem`` instead of ``std::filesystem``. Only needed on toolchains without C++17 filesystem support.
   * - ``ENABLE_MACOS_FRAMEWORK``
     - ``OFF``
     - macOS
     - Build as macOS Framework bundles instead of plain shared libraries.
   * - ``ENABLE_CXX_INTERFACE``
     - ``OFF``
     -
     - Exports symbols for the ``ProjectM`` and ``PCM`` C++ classes and installs the additional the headers. Using the C++ interface is not recommended and unsupported.
   * - ``ENABLE_VERBOSE_LOGGING``
     - ``OFF``
     -
     - Enables code for ``TRACE`` and ``DEBUG`` log levels in release builds. By default, these will only be compiled for ``Debug`` builds. Enabling this will negatively affect performance, even if the actual log level is set to ``INFORMATION`` or higher.

Path options
^^^^^^^^^^^^

There are also a few textual parameters that can be used to fine-tune the installation directories. Relative paths in
the following options are appended to the value
of `CMAKE_INSTALL_PREFIX <https://cmake.org/cmake/help/latest/variable/CMAKE_INSTALL_PREFIX.html>`__ (which, on most UNIX
platforms, defaults to ``/usr/local``):

.. list-table::
   :header-rows: 1

   * - CMake option
     - Default
     - Description
   * - ``CMAKE_INSTALL_PREFIX``
     - (OS dependent)
     - Base directory where the projectM libraries, includes and support files will be installed.
   * - ``PROJECTM_BIN_DIR``
     - ``bin``
     - Directory where executables (e.g. the SDL standalone application) are installed.
   * - ``PROJECTM_LIB_DIR``
     - ``lib[64]``
     - Directory where libprojectM is installed.
   * - ``PROJECTM_INCLUDE_DIR``
     - ``include``
     - Directory where the libprojectM include files will be installed under.

Other options
^^^^^^^^^^^^^

Various other options for specific needs.

.. list-table::
   :header-rows: 1

   * - CMake option
     - Default
     - Description
   * - ``CMAKE_DEBUG_POSTFIX``
     - ``d`` (if ``ENABLE_DEBUG_POSTFIX`` is set)
     - Postfix appended to debug libraries.
   * - ``CMAKE_INSTALL_BINDIR``
     - ``bin``
     - Another way to specify the binary installation directory. Used as default value for ``PROJECTM_BIN_DIR``.
   * - ``CMAKE_INSTALL_LIBDIR``
     - ``lib`` or ``lib64``
     - Another way to specify the library installation directory. Used as default value for ``PROJECTM_LIB_DIR``.
   * - ``CMAKE_INSTALL_INCLUDEDIR``
     - ``include``
     - Another way to specify the include file installation directory. Used as default value for ``PROJECTM_INCLUDE_DIR``.

Always perform out-of-tree builds!
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Most classic IDEs and build systems directly make use of the source tree and create project files, temporary build
artifacts (e.g. object files) and the final binaries in the same directory structure as the source files. An advantage
of this approach is that you can find all compiled binaries side-by-side with their sources and generated headers are
already in the same directories as the source files including them. This approach has some drawbacks though:

- Only a single build configuration is supported as files are overwritten in-place.
- A lot of noise is created in the source directory, making it hard to distinguish between generated and original source
  files.
- A very large ``.gitignore`` file is required to cover all unwanted files.
- Mistakes in the build scripts can overwrite source files, causing errors and destroy uncommitted work.

Some of these can be mitigated by providing additional targets (``make clean`` and ``make distclean``) or creating
subdirectories for Debug/Release build configurations.

While CMake also supports in-tree builds, it is "discouraged" in the official documentation, for the above reasons.
Building out-of-tree allows it to create multiple build directories with different configurations which do not influence
each other in any way. If a build directory contains unwanted artifacts, and you want to start fresh, simply delete and
recreate the whole directory - no work is lost.

This project follow this principle by treating the original source tree as read-only and avoiding potential conflicts:

- Everything under `CMAKE_SOURCE_DIR <https://cmake.org/cmake/help/latest/variable/CMAKE_SOURCE_DIR.html>`__ must only be
  read, never changed or written to.
- Everything under `CMAKE_BINARY_DIR <https://cmake.org/cmake/help/latest/variable/CMAKE_BINARY_DIR.html>`__ is temporary
  and related to the current build configuration.
- When generating configuration-dependent files,
  use `CMAKE_CONFIGURATION_TYPES <https://cmake.org/cmake/help/latest/variable/CMAKE_CONFIGURATION_TYPES.html>`__
  and `CMAKE_BUILD_TYPE <https://cmake.org/cmake/help/latest/variable/CMAKE_BUILD_TYPE.html>`__ to create non-conflicting
  files in the build tree.

While this project will not force you to build out-of-tree, there is no mechanism to clean up the generated files after
running cmake in-tree.

CMake build directory layout
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If you are new to CMake, the way of how CMake creates the build directory and where it creates the build targets might
be confusing. Here is a summary of what's in the build directory and how it is structured in general.

Using files from the build tree
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

It is generally not good practice to directly take binaries and other files from the build tree for packaging, for
several reasons:

1. The directory structure is generated by CMake and depends on the generator used. The layout might change between
   CMake versions, even for the same generator.
2. On platforms with RPATH support, CMake will store absolute paths in executables and shared libraries which point to
   the absolute paths of any linked dependencies, either from the build tree or external libraries as well. These
   binaries are not relocatable and will most certainly not work if run on any other computer (or even on the same after
   deleting the build directory).
3. For some configurations, even Release build artifacts may contain debug symbols until they are installed.

It is fine to build and run executables from the build directory for development and debugging. For packaging or using
libprojectM in other projects, always use the ``install`` target and copy files from there.

Generated files
^^^^^^^^^^^^^^^

In the top-level build directory, CMake creates a few files that are present on any platform:

- ``CMakeCache.txt``: This file contains all variables and build settings CMake needs to remember from the first
  configuration run. This file can be edited on demand either manually or using a CMake UI to change any values. On the
  next build, CMake will regenerate the project files if this file has been modified.
- ``cmake_install.cmake``: Contains generated install-related settings.
- ``install_manifest.txt``: After installing the project, this file contains a list with absolute filenames of all
  installed files. It can be used for packaging or deleting installed files as CMake doesn't define an ``uninstall``
  target.
- The top-level project file for use with the selected build toolset, e.g. ``Makefile``, ``build.ninja``, ``projectm.sln``
  or ``projectm.xcodeproj``, plus additional toolset-specific files.

The projectM build files generate additional files used in the build and install phases. These are scattered over the
build tree, but installed into the proper directories. Do not try and gather these files yourself from the build tree.

Subdirectory structure
^^^^^^^^^^^^^^^^^^^^^^

The rest of the directory structure generally resembles the source tree. Source directories containing
a ``CMakeLists.txt`` file will also be created in the build tree with the same relative path. Each of these subdirectories
contains a ``CMakeFiles`` directory with CMake-internal data, generated project files for the select toolset, e.g.
makefiles and any temporary compile artifacts.

The directory structure is created by CMake and may change depending on the generator and CMake version used.

Executable and library locations
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Build targets - shared/static libraries and executables - are created in the same subdirectory in the build tree as
the ``CMakeLists.txt`` file that defines the target in the source tree (which, in most cases, resides in the same
directory as the source files). Depending on the generator used, the binaries are created directly in the directory for
single-configuration generators (like ``Unix Makefiles`` or ``Ninja``) and in a subdirectory with the configuration name,
e.g. ``Debug`` or ``Release``, for multi-configuration generators like ``Xcode`` or ``Visual Studio 17 2022``.

You may also find additional files and symbolic links in the same location depending on the platform, e.g. ``.pdb`` files
on Windows.

Using libprojectM in other CMake projects
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The projectM library can be used as a static library or shared library in other CMake-based projects to provide embedded
audio visualization. It is highly recommended to build projectM as shared libraries for maximum compatibility and LGPL
compliance.

The build directory is not structured in a way that other projects can make use of it. Use the ``install`` target to copy
all required files to the configured installation prefix. You can customize the subdirectories for libraries, includes
and binaries using the ``PROJECTM_<X>>_DIR`` variables when configuring the CMake project.

Importing libprojectM targets from the build tree
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

This approach is not recommended, but can be useful for projects that either require more in-depth access to the
projectM library files, especially to headers that are not installed as part of the public API. This might cause issues
if the internal headers change, but gives a broader set of features and more control to the developer.

Please refer to the `ExternalProject <https://cmake.org/cmake/help/latest/module/ExternalProject.html>`__ CMake module
documentation on how to set up the libprojectM build system for use in another project.

Importing libprojectM targets from an installed version
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

This is the recommended and supported way of importing libprojectM in your project. This project installs a set of CMake
files in ``<PREFIX>/<LIBDIR>/cmake/projectM4``, containing target definitions, version and dependency checks as well as
any additional libraries required for linking. Other projects then use CMake's ``find_package`` command to search for
these files in `different locations <https://cmake.org/cmake/help/latest/command/find_package.html#search-procedure>`__.

In the case projectM libraries and headers are not installed in any system search path, you need to add either the
install prefix path (the top-level install dir) or the directory containing the libraries (the ``lib`` dir by default) to
the `CMAKE_PREFIX_PATH <https://cmake.org/cmake/help/latest/variable/CMAKE_PREFIX_PATH.html>`__ list.

If the package was found, you can then link against libprojectM by using the ``libprojectM::projectM`` target.

Depending on how the package was built, targets might be available for multiple configurations or only ``Release``. CMake
will automatically select the most appropriate one to link.

Include dirs, additional link dependencies and possible compiler options will be propagated to any target the library is
linked to.

Using the optional playlist library
"""""""""""""""""""""""""""""""""""

If you want to use the optional playlist library, you need to specifically request it as a component:

.. code:: cmake

   find_package(projectM4 COMPONENTS Playlist)

You can either use ``REQUIRED`` to force a fatal error if the component cannot be found or check if the target exists
using:

.. code:: cmake

   if(TARGET libprojectM::playlist)
       # ...
   endif()

If you link the playlist library, the main ``libprojectM::projectM`` target will be linked automatically as a dependency.

Example
"""""""

Searches for projectM and the playlist library and links both to the application:

.. code:: cmake

   find_package(projectM4 REQUIRED COMPONENTS Playlist)

   add_executable(MyApp main.cpp)

   target_link_libraries(MyApp PRIVATE
           libprojectM::playlist
           )
