#!/bin/bash

# Log to file for debugging
echo "$(date): Stop hook called" >> /tmp/claude-stop-hook.log
echo "Tool: $CLAUDE_TOOL_NAME" >> /tmp/claude-stop-hook.log
echo "Files: $CLAUDE_FILE_PATHS" >> /tmp/claude-stop-hook.log
echo "---" >> /tmp/claude-stop-hook.log

# Alert message that should be visible - output to stderr
echo "🚨 STOP HOOK EXECUTED! 🚨" >&2

# Exit with code 2 to make output visible to Claude
# exit 2