/* Minimal single-threaded HTTP server in C++
   - Serves files from ../fronend directory
   - Provides simple REST endpoints for flashcards at /cards
   - Persistence in cards.txt (one card per two lines: front then back)

   Build (Windows with MinGW):
     g++ server.cpp -o server.exe -lws2_32
   Build (Linux/macOS):
     g++ server.cpp -o server

   Run:
     ./server
   Then open http://localhost:8080/
*/

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

#ifdef _WIN32
#define _WIN32_WINNT 0x0601
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
typedef SOCKET socket_t;
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
typedef int socket_t;
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#endif

using namespace std;

struct Card { string front, back; };

vector<Card> cards;
const string dataFile = "cards.txt";
const string frontendDir = "../fronend"; // served root

void loadCards(){
    cards.clear();
    ifstream ifs(dataFile);
    if(!ifs) return;
    string a,b;
    while(true){
        if(!getline(ifs,a)) break;
        if(!getline(ifs,b)) b = "";
        // trim CR
        if(!a.empty() && a.back()=='\r') a.pop_back();
        if(!b.empty() && b.back()=='\r') b.pop_back();
        cards.push_back({a,b});
    }
}

void saveCards(){
    ofstream ofs(dataFile);
    for(auto &c: cards){
        ofs << c.front << "\n" << c.back << "\n";
    }
}

string statusLine(int code){
    switch(code){
        case 200: return "HTTP/1.1 200 OK\r\n";
        case 201: return "HTTP/1.1 201 Created\r\n";
        case 400: return "HTTP/1.1 400 Bad Request\r\n";
        case 404: return "HTTP/1.1 404 Not Found\r\n";
        case 500: return "HTTP/1.1 500 Internal Server Error\r\n";
        default: return "HTTP/1.1 200 OK\r\n";
    }
}

string readFileToString(const string& path){
    ifstream ifs(path, ios::binary);
    if(!ifs) return string();
    stringstream ss; ss << ifs.rdbuf();
    return ss.str();
}

string escapeJson(const string &s){
    string out; for(char c: s){
        switch(c){
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default: out.push_back(c);
        }
    }
    return out;
}

// naive extract of JSON fields: looks for "front":"..." etc
string extractJsonField(const string &body, const string &field){
    auto key = '"' + field + '"';
    size_t p = body.find(key);
    if(p==string::npos) return string();
    p = body.find(':', p);
    if(p==string::npos) return string();
    size_t q = body.find('"', p);
    if(q==string::npos) return string();
    q++; // start
    string out;
    while(q < body.size()){
        char c = body[q++];
        if(c == '"') break;
        if(c == '\\' && q < body.size()){
            char nc = body[q++];
            switch(nc){ case 'n': out.push_back('\n'); break; case 'r': out.push_back('\r'); break; case 't': out.push_back('\t'); break; default: out.push_back(nc); }
        } else out.push_back(c);
    }
    return out;
}

void sendAll(socket_t s, const string &data){
    const char* buf = data.c_str();
    size_t left = data.size();
    while(left>0){
#ifdef _WIN32
        int sent = send(s, buf, (int)left, 0);
#else
        ssize_t sent = ::send(s, buf, left, 0);
#endif
        if(sent <= 0) break;
        buf += sent; left -= sent;
    }
}

int main(){
#ifdef _WIN32
    WSADATA wsaData;
    if(WSAStartup(MAKEWORD(2,2), &wsaData) != 0){ cerr<<"WSAStartup failed\n"; return 1; }
#endif
    loadCards();
    int port = 8080;
    socket_t listenSock = socket(AF_INET, SOCK_STREAM, 0);
    if(listenSock == INVALID_SOCKET){ cerr<<"Failed to create socket\n"; return 1; }
    sockaddr_in addr; addr.sin_family = AF_INET; addr.sin_addr.s_addr = INADDR_ANY; addr.sin_port = htons(port);
    int opt = 1;
#ifdef _WIN32
    setsockopt(listenSock, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
#else
    setsockopt(listenSock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif
    if(bind(listenSock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR){ cerr<<"Bind failed\n"; return 1; }
    if(listen(listenSock, 10) == SOCKET_ERROR){ cerr<<"Listen failed\n"; return 1; }
    cout << "Server started at http://localhost:" << port << "/\n";

    while(true){
        sockaddr_in client; socklen_t clen = sizeof(client);
#ifdef _WIN32
        SOCKET clientSock = accept(listenSock, (sockaddr*)&client, &clen);
#else
        int clientSock = accept(listenSock, (sockaddr*)&client, &clen);
#endif
        if(clientSock == INVALID_SOCKET) continue;
        // read request (very naive, reads up to 64KB)
        string req;
        char buffer[65536];
#ifdef _WIN32
        int r = recv(clientSock, buffer, sizeof(buffer)-1, 0);
#else
        ssize_t r = recv(clientSock, buffer, sizeof(buffer)-1, 0);
#endif
        if(r>0){ buffer[r]=0; req.assign(buffer, r); }
        else { 
#ifdef _WIN32
            closesocket(clientSock);
#else
            close(clientSock);
#endif
            continue;
        }
        // parse request line
        istringstream reqs(req);
        string reqLine; getline(reqs, reqLine);
        if(!reqLine.empty() && reqLine.back()=='\r') reqLine.pop_back();
        string method, path, httpv;
        {
            istringstream l(reqLine);
            l >> method >> path >> httpv;
        }
        // parse headers
        string headers;
        string line;
        size_t contentLength = 0;
        while(getline(reqs, line) && line != "\r" && !line.empty()){
            headers += line + "\n";
            if(line.find("Content-Length:")!=string::npos){
                auto p = line.find(':');
                if(p!=string::npos) contentLength = stoi(line.substr(p+1));
            }
        }
        // read body if any (may be after the first recv; try to extract from the original buffer)
        size_t headerPos = req.find("\r\n\r\n");
        string body;
        if(headerPos!=string::npos){
            body = req.substr(headerPos+4);
            // if body incomplete, we'd need to recv more; skipping for simplicity
        }

        // Routing
        if(method=="GET" && (path=="/" || path=="/index.html")){
            string content = readFileToString(frontendDir + "/index.html");
            if(content.empty()){ string resp = statusLine(404) + "Content-Length: 0\r\n\r\n"; sendAll(clientSock, resp); }
            else{
                string resp = statusLine(200) + "Content-Type: text/html; charset=utf-8\r\nContent-Length: " + to_string(content.size()) + "\r\n\r\n" + content;
                sendAll(clientSock, resp);
            }
        } else if(method=="GET" && path.rfind("/static/",0)==0){
            string rel = path.substr(strlen("/static/"));
            string content = readFileToString(frontendDir + "/" + rel);
            if(content.empty()){ string resp = statusLine(404) + "Content-Length: 0\r\n\r\n"; sendAll(clientSock, resp); }
            else{
                // naive content type
                string ctype = "text/plain";
                if(rel.rfind(".js")!=string::npos) ctype = "application/javascript";
                else if(rel.rfind(".css")!=string::npos) ctype = "text/css";
                else if(rel.rfind(".html")!=string::npos) ctype = "text/html";
                string resp = statusLine(200) + "Content-Type: " + ctype + "; charset=utf-8\r\nContent-Length: " + to_string(content.size()) + "\r\n\r\n" + content;
                sendAll(clientSock, resp);
            }
        } else if(path=="/cards" && method=="GET"){
            // return JSON array
            stringstream out; out << "[";
            for(size_t i=0;i<cards.size();++i){
                if(i) out<<",";
                out << "{\"id\":"<<i<<",\"front\":\""<<escapeJson(cards[i].front)<<"\",\"back\":\""<<escapeJson(cards[i].back)<<"\"}";
            }
            out << "]";
            string bodyOut = out.str();
            string resp = statusLine(200) + "Content-Type: application/json; charset=utf-8\r\nContent-Length: " + to_string(bodyOut.size()) + "\r\n\r\n" + bodyOut;
            sendAll(clientSock, resp);
        } else if(path=="/cards" && method=="POST"){
            // accept JSON body {"front":"...","back":"..."}
            string f = extractJsonField(body, "front");
            string b = extractJsonField(body, "back");
            if(f.empty()){ string resp = statusLine(400) + "Content-Length: 0\r\n\r\n"; sendAll(clientSock, resp); }
            else{
                cards.push_back({f,b}); saveCards();
                string resp = statusLine(201) + "Content-Length: 0\r\n\r\n"; sendAll(clientSock, resp);
            }
        } else if(path.rfind("/cards?id=",0)==0 && method=="DELETE"){
            string idstr = path.substr(strlen("/cards?id="));
            int id = stoi(idstr);
            if(id<0 || id >= (int)cards.size()){ string resp = statusLine(404) + "Content-Length: 0\r\n\r\n"; sendAll(clientSock, resp); }
            else{ cards.erase(cards.begin()+id); saveCards(); string resp = statusLine(200) + "Content-Length: 0\r\n\r\n"; sendAll(clientSock, resp); }
        } else {
            string resp = statusLine(404) + "Content-Length: 0\r\n\r\n";
            sendAll(clientSock, resp);
        }

#ifdef _WIN32
        closesocket(clientSock);
#else
        close(clientSock);
#endif
    }

#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}
