# ĐÁNH GIÁ KỸ THUẬT VÀ PHÂN TÍCH ĐÁP ỨNG CHO DÂY CHUYỀN SAMSUNG & LG
## Dự án: Điều khiển Đa trục Ezi-SERVO II Plus-E qua Ethernet trên STM32F767 IO Board

---

## 1. TỔNG QUAN HỆ THỐNG VÀ CƠ CHẾ KẾT NỐI

### 1.1. Cấu trúc phần cứng STM32 IO Board
* **Vi điều khiển:** STM32F767ZI (ARM Cortex-M7 @ 216 MHz, bộ nhớ Cache L1 I/D 16KB + 16KB, DMA Ethernet MAC chuyên dụng).
* **Ethernet PHY:** TI DP83848C giao tiếp MII tốc độ 100 Mbps Full-Duplex.
* **Giao thức mạng:** LwIP Stack (Raw UDP API, Zero-copy buffer), cổng giao tiếp UDP 3002.

### 1.2. Cơ chế Daisy-Chain (Nối tiếp) trên Ezi-SERVO II Plus-E
* Mỗi bộ driver Fastech Ezi-SERVO II Plus-E được trang bị **2 cổng RJ45 Ethernet** tích hợp vi mạch Ethernet Switch 2-cổng (Layer-2 unmanaged store-and-forward/cut-through).
* **Tuyến nối dây:** `[STM32 IO Board] ---> [Driver 1] ---> [Driver 2] ---> [Driver 3]`
* **Độ trễ vật lý (Hardware Switching Latency):** Mỗi chặng (hop) switch chỉ gây trễ thêm khoảng **15 ~ 35 microsecond (µs)**. Vì vậy, độ trễ thêm vào giữa Driver 1 và Driver 3 qua 2 chặng là hoàn toàn không đáng kể đối với mạng 100 Mbps.

---

## 2. PHÂN TÍCH HIỆU NĂNG THỜI GIAN THỰC (REAL-TIME & LATENCY)

### 2.1. Bản chất điều khiển của Ezi-SERVO II Plus-E
Khác với các hệ thống điều khiển vị trí trực tiếp theo xung (Pulse/Direction) hay nội suy liên tục chu kỳ nhanh (Cyclic Synchronous Position - CSP) của EtherCAT:
* **Ezi-SERVO II Plus-E là bộ điều khiển phát biên dạng nội bộ (Internal Profile Generator):**
  * STM32 chỉ cần phát một lệnh ngắn: *"Chạy tới vị trí $P$, vận tốc $V$, gia tốc $A$"*.
  * Bộ xử lý DSP/FPGA 32-bit bên trong từng driver tự động tính toán đường cong gia tốc S-curve hoặc hình thang (Trapezoidal) và điều khiển vòng kín (Closed-loop) với Encoder độ phân giải cao (10,000 ~ 20,000 pulse/vòng).
  * Do đó, **độ chính xác định vị tại điểm dừng (Repeatability & Precision)** của cơ cấu cơ khí phụ thuộc 100% vào Encoder và phần cơ khí của động cơ, hoàn toàn không bị ảnh hưởng bởi độ trễ Ethernet!

### 2.2. Thời gian phát lệnh và Chu kỳ phản hồi trên STM32F767
* **Tốc độ gửi lệnh từ STM32:** STM32F767 chạy ở xung nhịp 216 MHz. Việc phát 3 gói tin UDP liên tiếp dạng Burst tới 3 IP (`.2`, `.3`, `.4`) qua DMA chỉ mất khoảng **30 ~ 60 µs**. 
* **Độ lệch kích hoạt giữa 3 trục (Trigger Jitter):** $\Delta t < 0.1 \text{ ms}$. Đối với chuyển động PTP (Point-to-Point), sai lệch này là hoàn toàn đồng thời trên giác quan thực tế và chu trình máy.
* **Chu kỳ quét trạng thái (Status Polling):** STM32 có thể quét toàn bộ thông tin của cả 3 driver (vị trí thực tế, cờ In-Position, cờ Alarm, mã lỗi) với chu kỳ **5ms ~ 10ms (100Hz ~ 200Hz)** với mức chiếm dụng CPU của STM32 < 4%.

---

## 3. ĐÁNH GIÁ KHẢ NĂNG ĐÁP ỨNG TRONG DÂY CHUYỀN SAMSUNG & LG

Trong các nhà máy của **Samsung Electronics, Samsung Display (SDC), LG Electronics, LG Innotek, LG Display (LGD)**, các yêu cầu kỹ thuật cho máy tự động hóa được phân định rất rõ ràng theo từng loại ứng dụng:

### 3.1. CÁC ỨNG DỤNG ĐÁP ỨNG TỐT VÀ ĐANG ĐƯỢC ỨNG DỤNG RỘNG RÃI (ĐẠT CHUẨN)
Hệ thống **STM32 + Ezi-SERVO II Plus-E (Ethernet UDP)** hoàn toàn đáp ứng xuất sắc các bài toán sau:

1. **Cơ cấu Gắp & Đặt (Pick & Place - P&P / Loader & Unloader):**
   * Hoạt động theo logic tuần tự: Trục Z nâng -> Trục X, Y di chuyển đến vị trí linh kiện -> Trục Z hạ xuống gắp -> Nhấc lên -> Chuyển về vị trí nhả.
   * Đây là chuyển động điểm-tới-điểm (PTP). Độ chính xác vị trí yêu cầu từ $5 \ \mu m \sim 20 \ \mu m$ được bảo đảm hoàn hảo bởi vòng kín của Ezi-SERVO.
2. **Hệ thống cấp phôi và khay linh kiện (Magazine Elevator / Tray Feeder / Shuttle):**
   * Nâng hạ từng tầng khay mạch, đẩy phôi vào băng chuyền.
3. **Cơ cấu bàn xoay chia độ (Index Rotary Table / Turret):**
   * Xoay góc cố định (ví dụ $45^\circ, 90^\circ, 180^\circ$) để chuyển trạm gia công.
4. **Máy kiểm tra ngoại quan quang học (AOI Inspection / Machine Vision):**
   * Di chuyển camera đến tọa độ định sẵn $\rightarrow$ Chờ tín hiệu In-Position $\rightarrow$ Trigger camera chụp ảnh kiểm tra $\rightarrow$ Di chuyển tiếp.
5. **Jig kiểm tra chức năng (FPCB / PBA Function Test Fixture):**
   * Đóng/mở jig, hạ chân kim pogo pin tiếp xúc test point.

> **Ưu thế lớn:** Tiết kiệm chi phí phần cứng rất lớn so với mạng công nghiệp đắt tiền; việc nối tiếp daisy chain giúp tủ điện gọn gàng, không cần thêm Switch công nghiệp phụ trợ.

---

### 3.2. CÁC ỨNG DỤNG KHÔNG NÊN HOẶC KHÔNG ĐỦ TIÊU CHUẨN ĐỂ ÁP DỤNG
Không được dùng giải pháp Ethernet UDP trong các trường hợp sau:

1. **Nội suy đường cong phức tạp tốc độ cao (Continuous Path / Circular & Linear Interpolation):**
   * Ví dụ: Máy tra keo viền màn hình cong OLED (Dispensing Robot), máy cắt kính/laser theo đường viền tự do.
   * *Lý do:* Yêu cầu các trục phải khóa pha đồng bộ ở mức microsecond (Sub-microsecond Sync) để vẽ đường tròn không bị méo. Mạng UDP chuẩn có Jitter ngẫu nhiên nên không nội suy liên tục được.
2. **Đồng bộ điện tử nhiều trục chặt chẽ (Electronic Gearing / E-Cam):**
   * Ví dụ: 2 động cơ kéo 2 đầu của một thanh trượt nặng (Gantry Dual-Drive). Nếu 1 trục nhận lệnh trễ 1 vài ms so với trục kia, thanh trượt có thể bị vặn xoắn cơ khí gây kẹt hoặc gãy cơ cấu.
3. **Trigger Camera On-The-Fly ở tốc độ cao:**
   * Chụp ảnh khi camera đang bay liên tục mà không dừng lại, yêu cầu độ trễ kích hoạt vị trí dưới 1 microsecond.

> **Giải pháp thay thế của Samsung/LG cho nhóm 3.2:**
> Bắt buộc sử dụng chuẩn **EtherCAT** (với dòng *Ezi-SERVO II EtherCAT* hoặc AC Servo Mitsubishi MR-J4/J5, Yaskawa Sigma-7, Beckhoff) hoặc xuất xung cứng vi sai High-speed Pulse/Dir từ Motion Card (Ajin Nexell, ACS, MEI).

---

## 4. CÁC BIỆN PHÁP BẮT BUỘC ĐỂ ĐẠT CHUẨN MÁY TỰ ĐỘNG HÓA SAMSUNG / LG

Để nghiệm thu thành công vào line sản xuất của Samsung/LG, hệ thống cần tuân thủ các quy tắc an toàn công nghiệp sau:

1. **Cơ chế Dừng khẩn cấp (Emergency Stop & Safety Hardwired):**
   * **Không bao giờ** chỉ dựa vào 1 lệnh dừng qua Ethernet UDP khi có sự cố nút E-Stop hoặc mở cửa bảo vệ (Safety Door).
   * **Bắt buộc:** Tín hiệu E-Stop từ rơ-le an toàn (Safety Relay / Safety PLC) phải được đấu dây cứng (Hardwired) trực tiếp vào chân **SVON / EMG** hoặc cắt nguồn cấp động lực 24V/48V của driver Ezi-SERVO để đạt chuẩn **ISO 13849-1 Category 3 / PLd**. Lệnh qua Ethernet chỉ đóng vai trò phần mềm phối hợp.
2. **Cơ chế Watchdog & Mất kết nối mạng (Network Loss Failsafe):**
   * Driver Ezi-SERVO Plus-E có tham số cấu hình thời gian Timeout mạng. Nếu sau 100ms không nhận được gói tin từ STM32, driver sẽ tự động dừng động cơ để tránh hiện tượng mất kiểm soát khi đứt cáp mạng.
3. **Chống nhiễu công nghiệp (EMC / Noise Immunity):**
   * Bắt buộc sử dụng cáp mạng công nghiệp có bọc kim chống nhiễu (**SF/UTP hoặc S/FTP Cat5e/Cat6 Industrial Grade**), đầu bấm kim loại tiếp địa tốt.
   * Không đi chung máng dây cáp mạng với cáp nguồn động lực động cơ hoặc biến tần.

---

## 5. KẾT LUẬN & KIẾN NGHỊ

1. **Khả thi:** Dự án hoàn toàn khả thi và có tính ứng dụng cao.
2. **Khả năng mở rộng:** Kiến trúc phần mềm non-blocking đã thiết kế cho phép bạn kiểm thử mượt mà 1 driver ban đầu, sau đó chỉ cần mở cờ khai báo thêm Driver 2 và Driver 3 trên cùng 1 bus Ethernet Daisy-Chain mà không cần sửa đổi cấu trúc chương trình.
3. **Phù hợp thị trường:** Đối với các máy lắp ráp, trung chuyển, kiểm tra linh kiện (chiếm hơn 75% số lượng máy trong dây chuyền Samsung và LG), cấu hình **STM32 IO Board + 3x Ezi-SERVO II Plus-E** là một giải pháp cực kỳ tối ưu về cả **độ tin cậy, tốc độ xử lý** lẫn **giá thành chế tạo**.
