#include "lsp_server.h"
#include "Angle.h"
#include "Node.h"
#include "ErrorHandler.h"
#include <iostream>
#include <sstream>
#include <string>
#include <map>

// Simple JSON escape for LSP responses
std::string escapeJSON(const std::string& str) {
    std::stringstream ss;
    for (char c : str) {
        switch (c) {
            case '"': ss << "\\\""; break;
            case '\\': ss << "\\\\"; break;
            case '\n': ss << "\\n"; break;
            case '\r': ss << "\\r"; break;
            case '\t': ss << "\\t"; break;
            default: ss << c; break;
        }
    }
    return ss.str();
}

class LSPServer {
private:
    std::map<std::string, std::string> documents;

    std::string readMessage() {
        std::string line;
        int contentLength = 0;

        while (std::getline(std::cin, line) && line != "\r") {
            if (line.find("Content-Length: ") == 0) {
                contentLength = std::stoi(line.substr(16));
            }
        }

        if (contentLength == 0) return "";

        std::string content(contentLength, '\0');
        std::cin.read(&content[0], contentLength);
        return content;
    }

    void sendMessage(const std::string& content) {
        std::cout << "Content-Length: " << content.length() << "\r\n\r\n";
        std::cout << content << std::flush;
    }

    void analyzeDiagnostics(const std::string& uri, const std::string& text) {
        std::stringstream diagnostics;
        diagnostics << "[";

        try {
            // KEY: Use wasp's analyze() to parse and validate the code!
            String codeStr(text.c_str());
            Node& ast = analyze(codeStr);

            // Parsing succeeded - could traverse AST for warnings, etc.

        } catch (const std::exception& e) {
            // Parse error - create diagnostic
            if (diagnostics.tellp() > 1) diagnostics << ",";
            diagnostics << "{";
            diagnostics << "\"range\":{\"start\":{\"line\":0,\"character\":0},\"end\":{\"line\":0,\"character\":0}},";
            diagnostics << "\"severity\":1,";
            diagnostics << "\"message\":\"" << escapeJSON(e.what()) << "\"";
            diagnostics << "}";
        }

        diagnostics << "]";

        // Send diagnostics
        std::stringstream response;
        response << "{\"jsonrpc\":\"2.0\",\"method\":\"textDocument/publishDiagnostics\",";
        response << "\"params\":{\"uri\":\"" << escapeJSON(uri) << "\",\"diagnostics\":" << diagnostics.str() << "}}";
        sendMessage(response.str());
    }

    void handleMessage(const std::string& msg) {
        // Simple JSON parsing - look for key patterns
        auto findValue = [&](const std::string& key) -> std::string {
            std::string search = "\"" + key + "\":";
            size_t pos = msg.find(search);
            if (pos == std::string::npos) return "";

            pos += search.length();
            while (pos < msg.length() && (msg[pos] == ' ' || msg[pos] == '\t')) pos++;

            if (msg[pos] == '"') {
                pos++;
                size_t end = msg.find('"', pos);
                if (end != std::string::npos) return msg.substr(pos, end - pos);
            } else if (std::isdigit(msg[pos]) || msg[pos] == '-') {
                size_t end = pos;
                while (end < msg.length() && (std::isdigit(msg[end]) || msg[end] == '.')) end++;
                return msg.substr(pos, end - pos);
            }
            return "";
        };

        std::string method = findValue("method");
        std::string id = findValue("id");

        if (method == "initialize") {
            std::stringstream resp;
            resp << "{\"jsonrpc\":\"2.0\",\"id\":" << id << ",\"result\":{";
            resp << "\"capabilities\":{\"textDocumentSync\":1,";
            resp << "\"diagnosticProvider\":{\"interFileDependencies\":false,\"workspaceDiagnostics\":false}},";
            resp << "\"serverInfo\":{\"name\":\"wasp-lsp\",\"version\":\"0.1.0\"}}}";
            sendMessage(resp.str());

        } else if (method == "textDocument/didOpen" || method == "textDocument/didChange") {
            std::string uri = findValue("uri");
            std::string text = findValue("text");
            if (!uri.empty() && !text.empty()) {
                documents[uri] = text;
                analyzeDiagnostics(uri, text);
            }

        } else if (method == "shutdown") {
            sendMessage("{\"jsonrpc\":\"2.0\",\"id\":" + id + ",\"result\":null}");

        } else if (method == "exit") {
            exit(0);
        }
    }

public:
    void run() {
        while (true) {
            std::string message = readMessage();
            if (message.empty()) break;
            try {
                handleMessage(message);
            } catch (const std::exception& e) {
                std::cerr << "LSP Error: " << e.what() << std::endl;
            }
        }
    }
};

int lsp_main() {
    LSPServer server;
    server.run();
    return 0;
}
