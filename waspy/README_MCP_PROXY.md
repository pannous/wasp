# MCP Proxy for Claude Desktop

This directory contains MCP (Model Context Protocol) proxy implementations to connect Claude Desktop to remote MCP servers.

## Files

- `mcp_proxy.py` - Advanced async proxy with dynamic tool discovery
- `simple_mcp_proxy.py` - Simple proxy with predefined tools
- `requirements.txt` - Python dependencies

## Usage

### Option 1: Simple Proxy (Recommended)

The simple proxy has predefined tools that match the remote server:

```bash
python simple_mcp_proxy.py
```

### Option 2: Advanced Proxy

The advanced proxy dynamically discovers tools from the remote server:

```bash
python mcp_proxy.py
```

## Setup with Claude Desktop

1. Install dependencies:
```bash
pip install -r requirements.txt
```

2. Run the proxy:
```bash
python simple_mcp_proxy.py
```

3. Configure Claude Desktop to use the local proxy (add to your Claude Desktop MCP settings):
```json
{
  "mcpServers": {
    "wasp-proxy": {
      "command": "python",
      "args": ["/path/to/waspy/simple_mcp_proxy.py"]
    }
  }
}
```

## Available Tools

The proxy forwards these tools from `https://mcp.pannous.com/mcp/`:

- `ping` - Responds with 'pong'
- `echo` - Echoes input back
- `hello` - Sends greetings
- `sine` - Sine function
- `sine_sin` - Sine sin function

## Remote Server

The proxy connects to: `https://mcp.pannous.com/mcp/`

This server hosts WebAssembly functions as MCP tools, allowing you to run WASM lambdas through Claude Desktop.