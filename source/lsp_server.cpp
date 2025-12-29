#include "lsp_server.h"
#include "Angle.h"
#include "Node.h"
#include "ErrorHandler.h"
#include <iostream>
#include <sstream>
#include <string>
#include <map>

// Simple JSON builder helpers
std::string escape_json_string(const std::string& str) {
    std::stringstream ss;
    ss << "\"";
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
    ss << "\"";
    return ss.str();
}

class LSPServer {
private:
    std::map<std::string, std::string> documents;
    int nextRequestId = 1;

    std::string readMessage() {
        std::string line;
        int contentLength = 0;

        // Read headers
        while (std::getline(std::cin, line) && line != "\r") {
            if (line.find("Content-Length: ") == 0) {
                contentLength = std::stoi(line.substr(16));
            }
        }

        if (contentLength == 0) return "";

        // Read content
        std::string content(contentLength, '\0');
        std::cin.read(&content[0], contentLength);
        return content;
    }

    void sendMessage(const std::string& content) {
        std::cout << "Content-Length: " << content.length() << "\r\n";
        std::cout << "\r\n";
        std::cout << content << std::flush;
    }

    void sendResponse(int id, const std::string& result) {
        std::stringstream ss;
        ss << "{\"jsonrpc\":\"2.0\",\"id\":" << id << ",\"result\":" << result << "}";
        sendMessage(ss.str());
    }

    void sendNotification(const std::string& method, const std::string& params) {
        std::stringstream ss;
        ss << "{\"jsonrpc\":\"2.0\",\"method\":" << escape_json_string(method)
           << ",\"params\":" << params << "}";
        sendMessage(ss.str());
    }

    void handleInitialize(int id) {
        std::stringstream result;
        result << "{";
        result << "\"capabilities\":{";
        result << "\"textDocumentSync\":1,";  // Full sync
        result << "\"diagnosticProvider\":{\"interFileDependencies\":false,\"workspaceDiagnostics\":false}";
        result << "},";
        result << "\"serverInfo\":{\"name\":\"wasp-lsp\",\"version\":\"0.1.0\"}";
        result << "}";
        sendResponse(id, result.str());
    }

    void handleDidOpen(const std::string& uri, const std::string& text) {
        documents[uri] = text;
        analyzeDiagnostics(uri, text);
    }

    void handleDidChange(const std::string& uri, const std::string& text) {
        documents[uri] = text;
        analyzeDiagnostics(uri, text);
    }

    void analyzeDiagnostics(const std::string& uri, const std::string& text) {
        std::vector<std::string> diagnostics;

        try {
            // Call wasp's analyze function
            Node& ast = analyze((String&)text);

            // Check for errors in AST
            // (In real implementation, traverse AST and collect errors)

        } catch (const std::exception& e) {
            // Create diagnostic for parse error
            std::stringstream diag;
            diag << "{";
            diag << "\"range\":{\"start\":{\"line\":0,\"character\":0},\"end\":{\"line\":0,\"character\":0}},";
            diag << "\"severity\":1,";  // Error
            diag << "\"message\":" << escape_json_string(e.what());
            diag << "}";
            diagnostics.push_back(diag.str());
        }

        // Send diagnostics
        std::stringstream params;
        params << "{\"uri\":" << escape_json_string(uri) << ",\"diagnostics\":[";
        for (size_t i = 0; i < diagnostics.size(); i++) {
            if (i > 0) params << ",";
            params << diagnostics[i];
        }
        params << "]}";

        sendNotification("textDocument/publishDiagnostics", params.str());
    }

    // Simple JSON parsing (minimal, just for LSP)
    std::string extractString(const std::string& json, const std::string& key) {
        std::string search = "\"" + key + "\":\"";
        size_t start = json.find(search);
        if (start == std::string::npos) return "";
        start += search.length();
        size_t end = json.find("\"", start);
        if (end == std::string::npos) return "";
        return json.substr(start, end - start);
    }

    int extractInt(const std::string& json, const std::string& key) {
        std::string search = "\"" + key + "\":";
        size_t start = json.find(search);
        if (start == std::string::npos) return -1;
        start += search.length();
        size_t end = start;
        while (end < json.length() && std::isdigit(json[end])) end++;
        return std::stoi(json.substr(start, end - start));
    }

public:
    void run() {
        while (true) {
            std::string message = readMessage();
            if (message.empty()) break;

            // Extract method and id
            std::string method = extractString(message, "method");
            int id = extractInt(message, "id");

            if (method == "initialize") {
                handleInitialize(id);
            } else if (method == "initialized") {
                // Nothing to do
            } else if (method == "textDocument/didOpen") {
                std::string uri = extractString(message, "uri");
                std::string text = extractString(message, "text");
                handleDidOpen(uri, text);
            } else if (method == "textDocument/didChange") {
                std::string uri = extractString(message, "uri");
                std::string text = extractString(message, "text");
                handleDidChange(uri, text);
            } else if (method == "shutdown") {
                sendResponse(id, "null");
            } else if (method == "exit") {
                break;
            }
        }
    }
};

int lsp_main() {
    LSPServer server;
    server.run();
    return 0;
}
