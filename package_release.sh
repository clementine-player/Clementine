#!/bin/bash
# Clementine Extended v0.0.4-experimental Package Builder
# GPL v3 Licensed - See COPYING for details

set -e

VERSION="0.0.4-experimental"
PACKAGE_NAME="clementine-extended"
BUILD_DIR="build"
PACKAGE_DIR="package"
DEB_DIR="${PACKAGE_DIR}/${PACKAGE_NAME}_${VERSION}_amd64"

echo "🎛️ Building Clementine Extended v${VERSION} Package"

# Clean previous packages
rm -rf ${PACKAGE_DIR}
mkdir -p ${DEB_DIR}

# Create directory structure
mkdir -p ${DEB_DIR}/usr/bin
mkdir -p ${DEB_DIR}/usr/share/applications
mkdir -p ${DEB_DIR}/usr/share/icons/hicolor/64x64/apps
mkdir -p ${DEB_DIR}/usr/share/doc/${PACKAGE_NAME}
mkdir -p ${DEB_DIR}/DEBIAN

# Copy main executable
echo "📦 Copying executables..."
cp ${BUILD_DIR}/clementine ${DEB_DIR}/usr/bin/clementine-extended
cp ${BUILD_DIR}/clementine-tagreader ${DEB_DIR}/usr/bin/

# Copy Python scripts
echo "🐍 Copying Python AI scripts..."
mkdir -p ${DEB_DIR}/usr/share/${PACKAGE_NAME}
cp clementine_stem_cli.py ${DEB_DIR}/usr/share/${PACKAGE_NAME}/
cp clementine_stem_player.py ${DEB_DIR}/usr/share/${PACKAGE_NAME}/
cp clementine_simple_stem.py ${DEB_DIR}/usr/share/${PACKAGE_NAME}/
cp clementine_stem_wrapper.sh ${DEB_DIR}/usr/share/${PACKAGE_NAME}/
chmod +x ${DEB_DIR}/usr/share/${PACKAGE_NAME}/*.sh

# Copy documentation
echo "📚 Copying documentation..."
cp README.md ${DEB_DIR}/usr/share/doc/${PACKAGE_NAME}/
cp COPYING ${DEB_DIR}/usr/share/doc/${PACKAGE_NAME}/
cp LICENSE_COMPLIANCE.md ${DEB_DIR}/usr/share/doc/${PACKAGE_NAME}/
cp RELEASE_NOTES_v0.0.4-experimental.md ${DEB_DIR}/usr/share/doc/${PACKAGE_NAME}/

# Create desktop file
echo "🖥️ Creating desktop entry..."
cat > ${DEB_DIR}/usr/share/applications/${PACKAGE_NAME}.desktop << EOF
[Desktop Entry]
Version=1.0
Type=Application
Name=Clementine Extended
Comment=AI-Powered Music Player with Stem Separation
Exec=clementine-extended %F
Icon=clementine-extended
Categories=AudioVideo;Audio;Player;
MimeType=audio/mpeg;audio/flac;audio/x-wav;audio/ogg;
EOF

# Create control file
echo "🔧 Creating package metadata..."
cat > ${DEB_DIR}/DEBIAN/control << EOF
Package: ${PACKAGE_NAME}
Version: ${VERSION}
Section: sound
Priority: optional  
Architecture: amd64
Depends: libc6, libqt5core5a, libqt5gui5a, libqt5widgets5a, python3, python3-pip, python3-venv, gstreamer1.0-plugins-base, gstreamer1.0-plugins-good
Maintainer: AI Stem Separation Implementation <noreply@example.com>
Description: Clementine Extended - AI-Powered Music Player
 Clementine Extended is a modern music player with AI-powered stem separation
 capabilities. Split songs into individual stems (vocals, drums, bass, other)
 and mix them in real-time with professional controls.
 .
 Features:
  * Offline AI stem separation using Demucs models
  * Real-time multitrack mixing interface  
  * Quick presets for karaoke and instrumental modes
  * Zero-latency GStreamer playback engine
  * GPL v3 licensed with compatible dependencies
Homepage: https://github.com/your-repo/AIMP-Clementine
EOF

# Create postinst script for Python setup
cat > ${DEB_DIR}/DEBIAN/postinst << EOF
#!/bin/bash
set -e

# Set up Python environment for AI features
echo "🐍 Setting up Python environment for AI features..."
echo "Run the following commands to complete setup:"
echo "  python3 -m venv ~/.clementine-stem-env"
echo "  source ~/.clementine-stem-env/bin/activate"  
echo "  pip install torch demucs"
echo ""
echo "📚 See /usr/share/doc/${PACKAGE_NAME}/README.md for complete instructions"

exit 0
EOF

chmod 755 ${DEB_DIR}/DEBIAN/postinst

# Build DEB package
echo "🔨 Building DEB package..."
dpkg-deb --build ${DEB_DIR}

# Create final package
mv ${DEB_DIR}.deb ./${PACKAGE_NAME}-${VERSION}-Linux.deb

echo "✅ Package created: ${PACKAGE_NAME}-${VERSION}-Linux.deb"
echo ""
echo "📦 Package Information:"
echo "  Name: ${PACKAGE_NAME}"
echo "  Version: ${VERSION}"
echo "  License: GPL v3"
echo "  Size: $(du -h ${PACKAGE_NAME}-${VERSION}-Linux.deb | cut -f1)"
echo ""
echo "🚀 Installation:"
echo "  sudo dpkg -i ${PACKAGE_NAME}-${VERSION}-Linux.deb"
echo "  sudo apt-get install -f  # Fix dependencies if needed"
echo ""
echo "🎵 Ready for release!"
