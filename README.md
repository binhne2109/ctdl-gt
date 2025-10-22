# Flashcards (C++ + minimal web UI)

This project contains:

- `nhom3.cpp` - original console C++ flashcard app using a doubly-linked list.
- `backend_cpp/server.cpp` - a minimal single-threaded C++ HTTP server that serves the static frontend and provides simple REST endpoints for cards.
- `fronend/` - static frontend (index.html, main.js, style.css) which interacts with the C++ server.

How to build and run (Linux / macOS / Windows with MinGW):

1. Build the server

   g++ backend_cpp/server.cpp -o backend_cpp/server

2. Run the server from the `team` directory (so the `fronend` folder is at `../fronend` relative to the built binary):

   backend_cpp/server

3. Open http://localhost:8080/ in a browser.

Notes:
- The C++ server is intentionally minimal and not production-ready. It implements a few REST endpoints:
  - GET /cards
  - POST /cards  (body JSON {front, back})
  - DELETE /cards?id=N
- Cards are stored in `cards.txt` in the server working directory.
- If you prefer the console app, compile `nhom3.cpp` with g++:

   g++ nhom3.cpp -o nhom3

