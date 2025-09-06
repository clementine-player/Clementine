#!/bin/bash
# Clementine Extended AppImage Builder
# GPL v3 Licensed - See COPYING for details

set -e

VERSION="0.0.4-experimental"
APP_NAME="ClementineExtended"
BUILD_DIR="build" 
APPDIR="${APP_NAME}.AppDir"

echo "🎛️ Building ${APP_NAME} v${VERSION} AppImage"

# Clean previous build
rm -rf ${APPDIR}
mkdir -p ${APPDIR}

# Create AppDir structure
mkdir -p ${APPDIR}/usr/bin
mkdir -p ${APPDIR}/usr/share/${APP_NAME}
mkdir -p ${APPDIR}/usr/share/doc

# Copy executables
echo "📦 Copying application files..."
cp ${BUILD_DIR}/clementine ${APPDIR}/usr/bin/
cp ${BUILD_DIR}/clementine-tagreader ${APPDIR}/usr/bin/

# Copy Python scripts
echo "🐍 Copying AI scripts..."
cp clementine_stem_cli.py ${APPDIR}/usr/share/${APP_NAME}/
cp clementine_stem_player.py ${APPDIR}/usr/share/${APP_NAME}/
cp clementine_simple_stem.py ${APPDIR}/usr/share/${APP_NAME}/
cp clementine_stem_wrapper.sh ${APPDIR}/usr/share/${APP_NAME}/
chmod +x ${APPDIR}/usr/share/${APP_NAME}/*.sh

# Copy documentation
cp README.md ${APPDIR}/usr/share/doc/
cp COPYING ${APPDIR}/usr/share/doc/
cp LICENSE_COMPLIANCE.md ${APPDIR}/usr/share/doc/

# Create AppRun script
cat > ${APPDIR}/AppRun << 'EOF'
#!/bin/bash
HERE="$(dirname "$(readlink -f "${0}")")"
export PATH="${HERE}/usr/bin:${PATH}"
export LD_LIBRARY_PATH="${HERE}/usr/lib:${LD_LIBRARY_PATH}"

# Set up Python environment paths
export CLEMENTINE_STEM_SCRIPTS="${HERE}/usr/share/ClementineExtended"

exec "${HERE}/usr/bin/clementine" "$@"
EOF

chmod +x ${APPDIR}/AppRun

# Create desktop file
cat > ${APPDIR}/${APP_NAME}.desktop << EOF
[Desktop Entry]
Version=1.0
Type=Application
Name=Clementine Extended
Comment=AI-Powered Music Player with Stem Separation
Exec=clementine
Icon=clementine-extended
Categories=AudioVideo;Audio;Player;
EOF

# Create icon (placeholder - use actual icon if available)
mkdir -p ${APPDIR}/usr/share/icons/hicolor/64x64/apps
if [ -f "data/icons/64x64/clementine.png" ]; then
    cp data/icons/64x64/clementine.png ${APPDIR}/clementine-extended.png
else
    # Create simple placeholder icon
    echo "Creating placeholder icon..."
    touch ${APPDIR}/clementine-extended.png
fi

# Check if linuxdeploy is available
if [ -f "./linuxdeploy-x86_64.AppImage" ]; then
    echo "🔧 Using linuxdeploy to create AppImage..."
    ./linuxdeploy-x86_64.AppImage --appdir ${APPDIR} --output appimage
    
    # Rename the output
    if [ -f "${APP_NAME}-x86_64.AppImage" ]; then
        mv "${APP_NAME}-x86_64.AppImage" "${APP_NAME}-${VERSION}-x86_64.AppImage"
        echo "✅ AppImage created: ${APP_NAME}-${VERSION}-x86_64.AppImage"
    fi
else
    echo "⚠️  linuxdeploy not found. Creating manual AppImage structure..."
    echo "📁 AppDir structure created in: ${APPDIR}"
    echo "💡 To complete AppImage build, download linuxdeploy from:"
    echo "   https://github.com/linuxdeploy/linuxdeploy/releases"
fi

echo ""
echo "📦 Build Summary:"
echo "  Version: ${VERSION}"
echo "  License: GPL v3"
echo "  AppDir: ${APPDIR}"
echo ""
echo "🚀 Usage:"
echo "  chmod +x ${APP_NAME}-${VERSION}-x86_64.AppImage"
echo "  ./${APP_NAME}-${VERSION}-x86_64.AppImage"
