# BMS
Dự án này tạo ra cấu trúc dữ liệu và hàm giúp chuyển toàn bộ thông tin từ hệ thống quản lý pin (BMS) sang định dạng JSON để hiển thị hoặc truyền đi.
Các phần chính bao gồm:
- Đo lường cell và pack
- Tính toán SOC, SOH
- Theo dõi trạng thái sạc/xả và cân bằng
- Giám sát các mức bảo vệ (over/under voltage, over current, over temperature...)
- Sinh cảnh báo tự động khi có lỗi hoặc mất cân bằng giữa các cell
=> Tất cả logic và xử lý đều nằm trong file BMS_DATA.h.

## 🧠 Ghi chú cho người phát triển
Nếu bạn muốn **tùy chỉnh cấu trúc JSON** (thêm, bớt hoặc đổi tên trường dữ liệu),  
có thể sử dụng công cụ **[ArduinoJson Assistant](https://arduinojson.org/v6/assistant/#/step4)** để:
1. Tạo mẫu JSON mong muốn  
2. Xem gợi ý cách khai báo `JsonDocument` tương ứng  
3. Dựa vào mẫu đó, bạn chỉ cần áp dụng **kiến thức lập trình C/C++ cơ bản**  
   để xây dựng hoặc mở rộng `struct BMSData` (hoặc `BMSStatus`) cho phù hợp.
Với công cụ này, bạn có thể dễ dàng định dạng JSON, thêm trường mới (như thời gian, ID thiết bị, trạng thái lỗi, v.v.) mà không cần am hiểu sâu về thư viện ArduinoJson.
