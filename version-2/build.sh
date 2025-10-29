#!/bin/bash
set -e

# Go to the directory where this script is located
cd "$(dirname "$0")"

# Remove old build directory if it exists
if [ -d "build" ]; then
    echo "Removing old build directory..."
    rm -rf build
fi

# Recreate it
echo "Creating new build directory..."
mkdir build
cd build

# Run cmake
echo "Running CMake configuration..."
cmake ..

# Optionally build immediately (uncomment if you want it to compile too)
echo "Building project..."
cmake --build .

echo "Rebuild complete!"