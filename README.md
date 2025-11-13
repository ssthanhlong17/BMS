# BMS
Dự án này tạo ra cấu trúc dữ liệu và hàm giúp chuyển toàn bộ thông tin từ hệ thống quản lý pin (BMS) sang định dạng JSON để hiển thị hoặc truyền đi.
Các phần chính bao gồm:
- Đo lường cell và pack
- Tính toán SOC, SOH
- Theo dõi trạng thái sạc/xả và cân bằng
- Giám sát các mức bảo vệ (over/under voltage, over current, over temperature...)
- Sinh cảnh báo tự động khi có lỗi hoặc mất cân bằng giữa các cell
=> Tất cả logic và xử lý đều nằm trong file BMS_DATA.h.

## Hướng dẫn test dữ liệu BMS

Nhánh test: **file-test**

Cấu trúc dữ liệu lấy từ file **`bms_data`**.

### 1. maintest1.cpp — Test dữ liệu trên Serial Monitor

File này dùng để kiểm tra việc đọc và xử lý dữ liệu BMS trực tiếp trên Serial Monitor.

### 2. maintest2.cpp — Test dữ liệu gửi lên Webserver

File này dùng để test chức năng gửi dữ liệu BMS lên webserver.

