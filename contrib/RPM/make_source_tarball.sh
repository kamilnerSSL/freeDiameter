#!/bin/bash

# Set version - this should match the Version tag in the .spec file
VERSION=1.6.0
PKG_NAME=freeDiameter

# Output tarball name
TARBALL=${PKG_NAME}-${VERSION}.tar.gz

# Check for a clean working directory
if ! git diff-index --quiet HEAD --; then
    echo "WARNING: Uncommitted changes present. Commit or stash them first."
fi

# Ensure rpmbuild directories exist
for dir in BUILD RPMS SOURCES SPECS SRPMS; do
    mkdir -p ~/rpmbuild/$dir
done

# Create archive using git archive
echo "Creating tarball: $TARBALL"
git archive --format=tar.gz --prefix=${PKG_NAME}-${VERSION}/ HEAD > "$TARBALL"

# Move to SOURCES if rpmbuild directory exists
if [ -d ~/rpmbuild/SOURCES ]; then
    echo "Copying tarball to ~/rpmbuild/SOURCES/"
    mv "$TARBALL" ~/rpmbuild/SOURCES/
fi

echo "Done."
