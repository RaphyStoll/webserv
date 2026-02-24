#!/bin/bash
# A simple bash CGI script to output environment variables

echo -ne "Content-Type: text/plain\r\n\r\n"

echo "=================================="
echo "    BASH CGI: HTTP ENVIRONMENT    "
echo "=================================="
echo ""
echo "This script demonstrates a raw Bash CGI execution."
echo "Here are the environment variables passed by Webserv:"
echo ""

# Print environment variables formatted
env | sort

echo ""
echo "=================================="
echo "      END OF CGI EXECUTION        "
echo "=================================="
