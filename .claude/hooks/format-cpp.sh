#!/bin/sh
# Runs format_cpp.py with whichever Python works. On Windows, python3 is often
# only a stub that offers to install Python from the Microsoft Store, and on
# macOS there is often no python.
if python3 -c '' 2>/dev/null; then
  exec python3 "$CLAUDE_PROJECT_DIR/.claude/hooks/format_cpp.py"
fi
exec python "$CLAUDE_PROJECT_DIR/.claude/hooks/format_cpp.py"
