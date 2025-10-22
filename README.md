# Ứng dụng Học Thẻ Từ Vựng (Flashcards)

Ứng dụng giúp học từ vựng bằng thẻ (flashcards), có 2 phiên bản:
- Console app (`nhom3.cpp`): Chạy trong terminal
- Web app (server + fronend): Giao diện web đẹp, dễ dùng

## Cách chạy Console App

```powershell
# Biên dịch
g++ nhom3.cpp -o nhom3.exe

# Chạy
./nhom3.exe
```

## Cách chạy Web App

### Cách 1: Chạy file server có sẵn
Nếu bạn có sẵn file `server.exe` (ví dụ ở `C:\temp\server.exe`):

```powershell
# Dừng server cũ nếu đang chạy
Get-Process -Name server -ErrorAction SilentlyContinue | Stop-Process -Force

# Khởi động server mới (ẩn cửa sổ terminal)
Start-Process -FilePath C:\temp\server.exe -WindowStyle Hidden

# Mở trình duyệt
Start-Process "http://localhost:8080"
```

### Cách 2: Biên dịch và chạy từ source

```powershell
# Biên dịch server (Windows với MinGW)
g++ -std=c++11 backend_cpp/server.cpp -o backend_cpp/server.exe -lws2_32

# Chạy server (từ thư mục gốc của project)
Start-Process -FilePath backend_cpp/server.exe -WindowStyle Hidden

# Mở web UI trong trình duyệt
Start-Process "http://localhost:8080"
```

## Cách sử dụng Web UI

1. Sau khi chạy server, mở trình duyệt vào http://localhost:8080
2. Thao tác với thẻ:
   - Bấm vào thẻ hoặc nút "Lật" để xem mặt sau
   - Dùng nút "Trước/Sau" để chuyển thẻ
   - Bấm "Xóa" để xóa thẻ hiện tại
   - Điền form "Thêm thẻ mới" để tạo thẻ
   - Bấm vào thẻ trong danh sách để chọn nhanh

## Quản lý Server

- Server chạy ở cổng 8080
- Thẻ được lưu vào file `cards.txt` (tự động tạo)
- Để dừng server:
  ```powershell
  Get-Process -Name server | Stop-Process
  ```
- Hoặc tìm và tắt process "server" trong Task Manager

## Gỡ lỗi thường gặp

1. **Lỗi "port already in use"**: Server cũ đang chạy
   ```powershell
   # Dừng server cũ
   Get-Process -Name server | Stop-Process
   ```

2. **Web UI không load**: Kiểm tra server có chạy không
   ```powershell
   # Kiểm tra server status
   Get-Process -Name server
   # Kiểm tra API
   Invoke-WebRequest -UseBasicParsing http://localhost:8080/cards
   ```

3. **Lỗi biên dịch với đường dẫn Unicode**: 
   - Biên dịch trong thư mục không dấu (ví dụ `C:\temp`)
   - Copy file `.exe` về thư mục project sau khi biên dịch

## Cấu trúc Project

- `nhom3.cpp` - Console app (C++)
- `backend_cpp/server.cpp` - Web server (C++)
- `fronend/` - Web UI (HTML/JS/CSS)
  - `index.html` - Giao diện chính
  - `style.css` - Styles và animation
  - `main.js` - Logic client-side

## Tính năng

- [x] Thêm/xóa/xem thẻ từ vựng
- [x] Lật thẻ với animation 3D
- [x] Lưu tự động vào file
- [x] Giao diện web responsive
- [x] Server C++ nhẹ, nhanh
