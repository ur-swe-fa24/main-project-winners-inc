#!/bin/bash

# Exit on any error
set -e

echo "🔨 Building project..."
cmake --build build --parallel

echo -e "\n🧪 Running tests..."
cd build && ctest --output-on-failure
