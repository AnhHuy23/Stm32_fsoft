# STM32_FSoft

Dự án này là một **mô hình tưới cây tự động** được xây dựng trên nền tảng **vi điều khiển STM32**, kết hợp với **cảm biến độ ẩm đất**, **màn hình LCD 16x2**, **RTC DS3231**, và có thể **kết nối với LabVIEW** để giám sát dữ liệu theo thời gian thực.

---

## Mục lục

- [Giới thiệu](#giới-thiệu)
- [Tính năng](#tính-năng)
- [Yêu cầu](#yêu-cầu)
- [Cài đặt](#cài-đặt)
- [Cấu trúc thư mục](#cấu-trúc-thư-mục)
- [Hướng dẫn sử dụng](#hướng-dẫn-sử-dụng)
- [Tài liệu tham khảo](#tài-liệu-tham-khảo)
- [Đóng góp](#đóng-góp)
- [Giấy phép](#giấy-phép)

---

## Giới thiệu

- **Sản phẩm tưới cây tự động bằng vi điều khiển STM32**, được lập trình bằng ngôn ngữ C trên STM32CubeIDE.
- Dữ liệu độ ẩm đất được đo bằng cảm biến và hiển thị trên LCD.
- Hệ thống tự động bật/tắt máy bơm (relay) theo ngưỡng độ ẩm.
- Thời gian thực được đồng bộ bằng mô-đun **DS3231 RTC**.
- Tùy chọn giao tiếp UART với máy tính, hỗ trợ truyền dữ liệu đến **LabVIEW** để giám sát hoặc điều khiển từ xa.

---

## Tính năng

- Cấu hình khởi tạo MCU: SystemClock, NVIC, SysTick
- Thư viện điều khiển:
  - **UART**: giao tiếp máy tính
  - **SPI / I²C**: kết nối cảm biến, RTC DS3231
  - **ADC**: đọc cảm biến độ ẩm đất
  - **GPIO**: LED, nút nhấn, relay
- Mẫu ứng dụng:
  - Đọc cảm biến độ ẩm đất và xuất kết quả qua LCD 16×2
  - Đồng bộ thời gian qua DS3231
  - Kết nối với LabVIEW để điều khiển và giám sát dữ liệu

---

## Yêu cầu

- Vi điều khiển: STM32F4-series (ví dụ STM32F401CC)
- Toolchain: STM32CubeIDE hoặc Keil MDK
- Thư viện HAL (STM32CubeMX)
- Phụ kiện: cảm biến độ ẩm, RTC DS3231, module LCD 16×2

---

## Cài đặt

1. Clone dự án:
   ```bash
   git clone https://github.com/AnhHuy23/Stm32_fsoft.git
   cd Stm32_fsoft

## Cấu trúc thư mục 

Stm32_fsoft/
├── Core/ // Mã nguồn chính (main.c, driver)
├── Drivers/ // HAL drivers từ STM32CubeMX
├── Examples/
│ ├── Soil_Moisture/ // Đọc độ ẩm, điều khiển bơm
│ ├── DS3231_RTC/ // Giao tiếp với DS3231
│ └── LCD_Display/ // Hiển thị LCD 16x2
├── Docs/ // Sơ đồ mạch, ảnh mô hình
├── README.md // Tệp mô tả này
└── .ioc // File cấu hình STM32CubeMX


