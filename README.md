# BMS
Dự án này tạo ra cấu trúc dữ liệu và hàm giúp chuyển toàn bộ thông tin từ hệ thống quản lý pin (BMS) sang định dạng JSON để hiển thị hoặc truyền đi.
Các phần chính bao gồm:
- Đo lường cell và pack
- Tính toán SOC, SOH
- Theo dõi trạng thái sạc/xả và cân bằng
- Giám sát các mức bảo vệ (over/under voltage, over current, over temperature...)
- Sinh cảnh báo tự động khi có lỗi hoặc mất cân bằng giữa các cell
=> Tất cả logic và xử lý đều nằm trong file BMS_DATA.h.
