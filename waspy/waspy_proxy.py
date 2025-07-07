# stupid anthropic does not support a remote MCP service yet so we need to create a local proxy
#!/usr/bin/env python3
# CLI: mcp dev demo.py
from mcp.server.fastmcp import FastMCP

mcp = FastMCP("Proxy")

@mcp.tool()
def hallo(name: str) -> str:
    """ Returns a greeting message with the provided name. """
    return f"HalliHallo {name}!"

if __name__ == "__main__":
    mcp.run()