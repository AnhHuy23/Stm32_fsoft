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
2. Mở file .ioc bằng STM32CubeIDE và generate code.

3. Build và nạp vào MCU

---

## Cấu trúc thư mục 
  ```bash
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
  ```

---

## Hướng dẫn sử dụng

### Các chế độ hoạt động

Hệ thống hỗ trợ 2 chế độ tưới cây:

- **AUTO (Tự động):**
  - Hệ thống tự động đọc giá trị từ cảm biến độ ẩm đất.
  - Nếu độ ẩm **< 30%**, hệ thống sẽ tự động kích hoạt **bơm tưới cây**.
  - Dữ liệu và trạng thái hiển thị trên **LCD 16x2**.

- **MANUAL (Thủ công):**
  - Cho phép **người dùng hẹn giờ tưới cây theo lịch trình tùy chọn**.
  - Đến thời gian đã hẹn, hệ thống sẽ **tự động kích hoạt bơm**, kể cả trong chế độ thủ công.

###  Điều khiển bằng nút nhấn

| Nút | Chân GPIO | Chức năng |
|-----|-----------|-----------|
| MODE  | PA12 | Chuyển đổi giữa chế độ **AUTO** và **MANUAL** |
| UP    | PA13 | Tăng giá trị **giờ/phút** khi cài đặt hẹn giờ |
| DOWN  | PA15 | Giảm giá trị **giờ/phút** khi cài đặt hẹn giờ |
| LEFT  | PA8  | Di chuyển con trỏ sang trái (chọn giờ/phút) |
| RIGHT | PA14 | Di chuyển con trỏ sang phải (chọn giờ/phút) |

### Giao diện hiển thị LCD

- **Chế độ hiện tại**: AUTO hoặc MANUAL
- **Độ ẩm đất hiện tại**
- **Giờ hẹn tưới (nếu có)**
- **Trạng thái bơm**: ON hoặc OFF

---
## Tài liệu tham khảo

- [Datasheet STM32F401CCU6](https://www.st.com/resource/en/datasheet/stm32f401cc.pdf)  
- [Reference Manual STM32F401xB/C](https://www.st.com/resource/en/reference_manual/dm00096844-stm32f401xbc-and-stm32f401xcc-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)  
- [DS3231 RTC Datasheet](https://datasheets.maximintegrated.com/en/ds/DS3231.pdf)  
- [LCD 16x2 - Components101](https://components101.com/displays/lcd-16x2)  
- [Soil Moisture Sensor - LastMinuteEngineers](https://lastminuteengineers.com/soil-moisture-sensor-arduino-tutorial/)  
- [Kết nối STM32 với LabVIEW – NI Forum](https://forums.ni.com/t5/LabVIEW/Interfacing-STM32-with-LabVIEW/td-p/3680566)  
- [Mô phỏng STM32 trên Proteus – The Engineering Projects](https://www.theengineeringprojects.com/2019/10/simulation-of-stm32-microcontroller-in-proteus.html)

---

## Đóng góp

| Họ và tên                  | Nhiệm vụ chính                                                                              |
| -------------------------- | ------------------------------------------------------------------------------------------- |
| **Lê Anh Huy**             | Lập trình vi điều khiển STM32 (I2C, DS3231, v.v.), viết báo cáo.                            |
| **Lê Trương Nguyễn Hoàng** | Lập trình vi điều khiển STM32 (điều khiển relay, xử lý nút bấm), làm slide trình bày (PPT). |
| **Đỗ Nhất Anh**            | Thiết kế giao diện điều khiển trên LabVIEW, thiết kế và layout mạch PCB.                    |
| **Bùi Minh Huy**           | Lắp ráp mạch thực tế, thực nghiệm và khảo sát; lập trình STM32 (giao tiếp LCD và ADC).      | 
