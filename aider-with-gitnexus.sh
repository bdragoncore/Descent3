#!/usr/bin/env bash
gitnexus mcp > /dev/null 2>&1 &
sleep 1
aider --model claude-3-5-sonnet

