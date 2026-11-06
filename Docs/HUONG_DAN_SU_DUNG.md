# HƯỚNG DẪN BIÊN DỊCH VÀ CHẠY THỬ NGHIỆM 1 DRIVER TRÊN IAR WORKBENCH
## Dự án: Điều khiển Ezi-SERVO II Plus-E trên Bo mạch STM32F767 IO Board

---

## 📁 1. Cấu trúc thư mục dự án

Dự án được tổ chức thành các thư mục độc lập, khoa học, giúp bạn dễ dàng theo dõi và chỉnh sửa:

```text
Test stm32-servo/
│
├── Core/                                  <-- MÃ NGUỒN CHÍNH CỦA BẠN
│   ├── Inc/
│   │   ├── ezi_servo_driver.h             # Header giao thức Fastech UDP (CMD, Frame, Struct)
│   │   ├── eziservo_config.h              # Cấu hình IP Driver, Port 3002, Timeout
│   │   ├── IO_Board.h                     # Định nghĩa phần cứng bo mạch E7502 (LED, Relay, UART)
│   │   ├── lwipopts.h                     # Cấu hình stack LwIP Bare-metal tối ưu (NO_SYS=1)
│   │   ├── main.h                         # Khai báo hệ thống chính
│   │   └── UserDefine.h                   # Cấu hình IP tĩnh MCU (192.168.0.100)
│   └── Src/
│       ├── main.c                         # Chương trình chính & Kịch bản Test 1 Driver tự động
│       ├── ezi_servo_driver.c             # Triển khai các API điều khiển Ezi-SERVO qua UDP
│       └── IO_Board.c                     # Khởi tạo Clock 216MHz, UART1 RS-232, LED chẩn đoán
│
├── Library/                               <-- THƯ VIỆN HỆ THỐNG VÀ PHẦN CỨNG
│   ├── 00_User/                           # Drivers ngoại vi bo mạch, MII DP83848 (ethernetif.c)
│   ├── 01_CMSIS/                          # Core ARM Cortex-M7 & thanh ghi STM32F767
│   ├── 02_HalDriver/                      # STM32F7xx HAL Driver chính hãng từ ST
│   └── 04_ThirdParty/LwIP/                # Mã nguồn mạng LwIP TCP/IP Stack
│
├── EWARM/                                 <-- PROJECT IAR EMBEDDED WORKBENCH
│   ├── eziservo_ioboard.eww               # IAR Workspace (Mở file này để lập trình)
│   ├── eziservo_ioboard.ewp               # IAR Project file
│   ├── eziservo_ioboard.ewd               # Cấu hình nạp ST-Link / J-Link Debugger
│   └── stm32f767xx_flash.icf              # Linker script phân bổ Flash 2MB / RAM 512KB
│
├── Docs/                                  <-- TÀI LIỆU KỸ THUẬT & PHÂN TÍCH
│   ├── TECHNICAL_ANALYSIS_SAMSUNG_LG.md   # Báo cáo kỹ thuật chi tiết theo tiêu chuẩn Samsung/LG
│   └── HUONG_DAN_SU_DUNG.md               # Tài liệu hướng dẫn này
│
└── build_iar.bat                          # Script build tự động 1-click bằng IarBuild.exe
```

---

## 🔌 2. Kết nối phần cứng cho 1 Driver

1. **Nguồn cấp:**
   * Cấp nguồn 24VDC vào cầu đấu nguồn mạch STM32 IO Board và cầu đấu nguồn Driver Ezi-SERVO II Plus-E (chân PWR).
2. **Cáp mạng Ethernet:**
   * Cắm cáp mạng RJ45 (loại Cat5e/Cat6 SF/UTP chống nhiễu) từ **Cổng Ethernet trên bo mạch STM32** trực tiếp vào **Cổng IN của Driver Ezi-SERVO II Plus-E**.
3. **Cài đặt ID trên Driver:**
   * Xoay công tắc chọn ID trên Driver (**SW1 và SW2**) về giá trị: **`2`** *(Tương ứng địa chỉ IP: `192.168.0.2`)*.
4. **Cáp Debug RS-232:**
   * Cắm cáp DB9 RS-232 từ cổng **J2 (USART1)** của bo mạch vào máy tính qua cáp chuyển đổi USB-to-RS232.
   * Cài đặt phần mềm Terminal (Putty / Hercules / MobaXterm): Baudrate **115200**, Data bits **8**, Parity **None**, Stop bits **1**.

---

## 🛠️ 3. Mở và Biên dịch dự án trên IAR Embedded Workbench

### Cách 1: Mở trên giao diện IAR IDE (Khuyến nghị)
1. Mở phần mềm **IAR Embedded Workbench for ARM** (phiên bản v8.x hoặc v9.x).
2. Chọn menu **File $\rightarrow$ Open Workspace...**
3. Trỏ tới file: `c:\Users\TOPV.LAPTOP.142\Desktop\Test stm32-servo\EWARM\eziservo_ioboard.eww`.
4. Trong cửa sổ Workspace (bên trái), chọn cấu hình: **`Release`** (hoặc `Debug`).
5. Nhấn phím tắt **`F7`** (Make) hoặc **`Ctrl + F7`** (Rebuild All).
6. File thực thi binary (`eziservo_ioboard.bin` / `.hex`) sẽ được tạo tại: `EWARM\Release\Exe\`.
7. Kết nối mạch nạp ST-Link/J-Link vào cổng nạp SWD trên bo mạch, nhấn **`Ctrl + D`** để Download và Debug.

### Cách 2: Chạy Script tự động
* Bạn có thể nhấp đúp trực tiếp vào file [`build_iar.bat`](file:///c:/Users/TOPV.LAPTOP.142/Desktop/Test%20stm32-servo/build_iar.bat) ở thư mục gốc của dự án. Script sẽ tự động tìm kiếm `IarBuild.exe` trên máy tính và tiến hành biên dịch ra file firmware hoàn chỉnh.

---

## 🚀 4. Kịch bản chạy thử nghiệm trên 1 Driver (Đã tích hợp sẵn trong main.c)

Khi cấp nguồn cho hệ thống, chương trình trong [`main.c`](file:///c:/Users/TOPV.LAPTOP.142/Desktop/Test%20stm32-servo/Core/Src/main.c) sẽ tự động chạy quy trình kiểm thử 6 bước rất rõ ràng:

```
[SYS] Khởi động STM32F767 @ 216 MHz...
[LED] Tự kiểm tra 4 kênh LED chẩn đoán J100..J103 trong 2 giây.
[ETH] Khởi tạo LwIP (IP STM32: 192.168.0.100).
[ETH] Đợi kết nối Ethernet Link Up... -> LINK UP! (Đèn J100 BẬT).

======================================================
   Ezi-SERVO COMMAND DIAGNOSTIC TEST (1 DRIVER TEST)
======================================================
[TEST 1/6] Khởi tạo Socket UDP Port 3002 -> PASS
[TEST 2/6] Gửi lệnh Ping trạng thái (CMD 0x43 - FAS_GetAllStatus) -> Nhận phản hồi OK từ Driver!
[TEST 3/6] Đọc thông số cài đặt Driver:
           - Độ phân giải encoder (Resolution): 10,000 pulse/rev
           - Tốc độ tối đa (Max Speed): 500,000 pps
           - Trạng thái chân Input / Output CN1
[TEST 4/6] Gửi lệnh Reset Alarm (CMD 0x2B)
[TEST 5/6] Gửi lệnh Bật Servo ON (CMD 0x2A)
[TEST 6/6] Thử nghiệm điều khiển chuyển động:
           - Xóa vị trí về 0 (CMD 0x56)
           - Chạy vị trí tương đối +10,000 pulse (CMD 0x35)
           - Chạy vị trí tuyệt đối +50,000 pulse (CMD 0x34)
           - Đọc encoder thực tế trả về (Actual Position) và cờ In-Position.
```

### Giám sát thời gian thực bằng đèn LED trên bo mạch:
* **J100:** Sáng cố định $\rightarrow$ Báo hiệu hệ thống và mạng Ethernet Link UP.
* **J101:** Nhấp nháy theo từng gói tin UDP trao đổi với Driver $\rightarrow$ Đang truyền nhận dữ liệu tốt.
* **J102:** Sáng khi Driver có cảnh báo Alarm (ví dụ chưa cắm giắc motor).
* **J103:** Nhấp nháy 1 Hz (500ms ON / 500ms OFF) $\rightarrow$ Nhịp tim MCU STM32 đang hoạt động ổn định.

---

## 🕹️ 5. Hướng dẫn Đấu nối Nút nhấn và Chạy Chế độ Bằng tay (Manual JOG)

### Sơ đồ đấu nối nút bấm trên bo mạch IO (E7502 IO Extension Board):
Bo mạch E7502 sử dụng các đầu nối chuẩn Molex 3 chân (5267-03) cho tín hiệu đầu vào cảm biến/nút bấm, cách ly quang qua IC Optocoupler `PS2801-1`:

* **Cổng J10 (Nút JOG+ / Chạy tiến):**
  * **Chân 1 (P24V):** Nguồn +24VDC cấp cho nút bấm.
  * **Chân 2 (INPUT_P_0):** Tín hiệu đầu vào (kết nối tới chân vi điều khiển **`PB5`** qua cách ly quang U5).
  * **Chân 3 (GND):** 0VDC.
  * 👉 **Cách đấu nút nhấn JOG+:** Nối tiếp điểm thường mở (NO) của nút nhấn giữa **Chân 1 (+24V)** và **Chân 2 (INPUT_P_0)**. Khi bấm nút, dòng 24V đi qua LED quang $\rightarrow$ kéo chân `PB5` xuống mức 0 (Active LOW).

* **Cổng J11 (Nút JOG- / Chạy lùi):**
  * **Chân 1 (P24V):** Nguồn +24VDC cấp cho nút bấm.
  * **Chân 2 (INPUT_P_1):** Tín hiệu đầu vào (kết nối tới chân vi điều khiển **`PB6`** qua cách ly quang U7).
  * **Chân 3 (GND):** 0VDC.
  * 👉 **Cách đấu nút nhấn JOG-:** Nối tiếp điểm thường mở (NO) của nút nhấn giữa **Chân 1 (+24V)** và **Chân 2 (INPUT_P_1)**. Khi bấm nút, dòng 24V đi qua LED quang $\rightarrow$ kéo chân `PB6` xuống mức 0 (Active LOW).

---

## ⚙️ 6. Nguyên lý Hoạt động của Chế độ Chạy Bằng tay & Chạy Tự động

### A. Chế độ Chạy Bằng tay (`g_operation_mode = 0` - Mặc định):
1. **Bấm giữ nút J10 (JOG+):**
   * STM32 tự động kiểm tra trạng thái Servo. Nếu Servo đang tắt, STM32 sẽ tự động bật Servo ON (khóa trục).
   * Phát lệnh `FAS_MoveVelocity(g_jog_speed_pps, 1)` qua UDP. Động cơ quay đều theo chiều thuận với tốc độ `g_jog_speed_pps` (mặc định 10,000 pps = 1 vòng/giây).
2. **Nhả nút J10:**
   * STM32 phát lệnh `FAS_MoveStop()` qua UDP. Động cơ giảm tốc êm ái và dừng lại.
3. **Bấm giữ nút J11 (JOG-):**
   * Động cơ quay đều theo chiều nghịch với tốc độ `g_jog_speed_pps`.
4. **Nhả nút J11:**
   * Động cơ giảm tốc êm ái và dừng lại.
5. **Khóa liên động an toàn (Interlock):**
   * Nếu người vận hành vô tình bấm đồng thời cả 2 nút J10 và J11 $\rightarrow$ Động cơ dừng ngay lập tức, ngăn ngừa xung đột.
   * Nếu Driver đang có lỗi (Alarm) $\rightarrow$ Khóa không cho phép Jog cho đến khi xóa lỗi.

### B. Chế độ Chạy Tự động (`g_operation_mode = 1`):
* Chuyển giá trị `g_operation_mode` trên Live Watch sang `1`.
* Gõ `g_auto_cycle_trigger = 1`: Động cơ sẽ tự động thực hiện 1 chu trình: Chạy tới Điểm A (+50,000 pulse) $\rightarrow$ Dừng chờ 500ms $\rightarrow$ Chạy về Điểm B (0 pulse) $\rightarrow$ Hoàn thành chu trình (`g_auto_cycle_count` tăng 1).
* Gõ `g_auto_cycle_trigger = 2`: Động cơ chạy chu trình lặp đi lặp lại liên tục.
* Gõ `g_auto_cycle_trigger = 0`: Dừng chu trình tự động.

---

## 🔍 7. Bảng biến Live Watch trên IAR để theo dõi và điều khiển

Khi vào chế độ Debug trên IAR (`Ctrl + D`), mở cửa sổ **View $\rightarrow$ Watch $\rightarrow$ Live Watch** và thêm các biến toàn cục sau:

| Tên biến trong code | Kiểu dữ liệu | Ý nghĩa hiển thị & Cách tương tác |
| :--- | :--- | :--- |
| `g_operation_mode` | `uint32_t` | **Chế độ hoạt động:** `0` = Manual Jog (Bằng tay), `1` = Auto (Tự động) |
| `g_jog_speed_pps` | `uint32_t` | **Tốc độ JOG (pulse/s):** Mặc định `10000` (có thể nhập đổi trực tiếp) |
| `g_btn_active_level` | `uint32_t` | **Mức tích cực của nút:** Mặc định `0` = Active LOW (chuẩn Opto: Bấm = 0V, Nhả = 3.3V); `1` = Active HIGH |
| `g_btn_jog_pos_raw` | `uint32_t` | **Giá trị logic thực tế chân PB5 (J10):** `1` khi nhả, `0` khi bấm |
| `g_btn_jog_neg_raw` | `uint32_t` | **Giá trị logic thực tế chân PB6 (J11):** `1` khi nhả, `0` khi bấm |
| `g_btn_jog_pos_pressed` | `uint32_t` | **Trạng thái nút J10:** `1` khi đang nhấn nút, `0` khi nhả |
| `g_btn_jog_neg_pressed` | `uint32_t` | **Trạng thái nút J11:** `1` khi đang nhấn nút, `0` khi nhả |
| `g_jog_state` | `uint32_t` | **Trạng thái Jog:** `0` = Dừng, `1` = Đang Jog+, `2` = Đang Jog- |
| `g_driver_act_pos` | `int32_t` | **Vị trí thực tế Encoder:** Tăng/giảm trực tiếp khi động cơ quay |
| `g_driver_axis_status` | `uint32_t` | **Cờ trạng thái Driver:** Bit 8 = Servo ON, Bit 13 = Moving |
| `g_driver_alarm_code` | `uint32_t` | **Mã lỗi Driver:** `0` = Bình thường (không có lỗi) |
| `g_udp_rx_packet_count` | `uint32_t` | **Gói tin UDP:** Tăng liên tục khi truyền nhận ổn định |
| `g_user_cmd_trigger` | `uint32_t` | **Bảng lệnh gõ thủ công:**<br>• `1`: Bật Servo ON<br>• `2`: Tắt Servo OFF<br>• `3`: Reset Alarm<br>• `4`: Xóa tọa độ về 0<br>• `5`: Chạy tiến 1 vòng (+10,000)<br>• `6`: Chạy tiến 5 vòng (+50,000)<br>• `7`: Chạy về 0<br>• `8`: Dừng khẩn cấp (Emergency Stop)<br>• `10`: Bắt đầu Jog+ qua phần mềm<br>• `11`: Bắt đầu Jog- qua phần mềm<br>• `12`: Dừng Jog qua phần mềm |

---

## 🔄 8. Mở rộng lên 3 Driver (Samsung / LG Daisy-Chain)

Sau khi kiểm tra thành công điều khiển bằng tay trên 1 Driver:
1. Cắm cáp mạng từ cổng **OUT** của Driver 1 sang cổng **IN** của Driver 2, và từ **OUT** của Driver 2 sang **IN** của Driver 3.
2. Xoay công tắc ID của Driver 2 thành **`3`** (`192.168.0.3`) và Driver 3 thành **`4`** (`192.168.0.4`).
3. Dùng module đa trục [`ezi_servo_multi.h`](file:///c:/Users/TOPV.LAPTOP.142/Desktop/Test%20stm32-servo/Core/Inc/ezi_servo_multi.h) để điều khiển đồng bộ cả 3 trục chuyển động.
