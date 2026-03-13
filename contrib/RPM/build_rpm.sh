#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
SPEC_FILE="$SCRIPT_DIR/freeDiameter.spec"

# Read version from spec file
VERSION=$(grep '^Version:' "$SPEC_FILE" | awk '{print $2}')
PKG_NAME=$(grep '^Name:' "$SPEC_FILE" | awk '{print $2}')
TARBALL="${PKG_NAME}-${VERSION}.tar.gz"

echo "Building RPM for $PKG_NAME $VERSION"
echo "Spec: $SPEC_FILE"

# Ensure rpmbuild directories exist
for dir in BUILD RPMS SOURCES SPECS SRPMS; do
    mkdir -p ~/rpmbuild/$dir
done

# Warn on uncommitted changes
if ! git -C "$REPO_ROOT" diff-index --quiet HEAD --; then
    echo "WARNING: Uncommitted changes present. The tarball will reflect HEAD only."
fi

# Create source tarball from git HEAD
echo "Creating tarball: $TARBALL"
git -C "$REPO_ROOT" archive --format=tar.gz --prefix="${PKG_NAME}-${VERSION}/" HEAD \
    > ~/rpmbuild/SOURCES/"$TARBALL"
echo "Tarball written to ~/rpmbuild/SOURCES/$TARBALL"

# Copy spec file
cp "$SPEC_FILE" ~/rpmbuild/SPECS/

# Build the RPM
echo "Running rpmbuild..."
rpmbuild -ba ~/rpmbuild/SPECS/freeDiameter.spec

echo ""
echo "Build complete. RPMs:"
find ~/rpmbuild/RPMS -name "${PKG_NAME}*.rpm" | sort
echo ""
echo "Source RPM:"
find ~/rpmbuild/SRPMS -name "${PKG_NAME}*.rpm" | sort
