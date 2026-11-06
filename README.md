# E7502 IO Extension Board - Ezi-SERVO Controller

Dự án firmware điều khiển động cơ servo Ezi-SERVO và mở rộng IO dựa trên vi điều khiển STM32F767 (ARM Cortex-M7).

## 🚀 Giới thiệu & Tính năng chính

- **MCU**: STM32F767 (Cortex-M7, 216MHz)
- **Môi trường phát triển**: IAR Embedded Workbench for ARM (EWARM)
- **Giao tiếp mạng**: Ethernet TCP/IP với LwIP Stack
- **Bộ nhớ mở rộng**: Micron N25Q QSPI/SPI NOR Flash
- **Tính năng điều khiển**:
  - Giao tiếp và điều khiển động cơ bước/servo Ezi-SERVO (Fastech)
  - Quản lý các chân IO mở rộng công nghiệp (Input, Output, Alarm, Emergency)
  - Giao thức điều khiển dạng bản tin (Command/Response) kết nối với UI / PC Software qua Socket Ethernet

## 📂 Cấu trúc thư mục

```text
├── Core/
│   ├── Inc/                 # Header files cấu hình & xử lý lệnh (CmdSet_*, IO_Board, UI_Server...)
│   └── Src/                 # Source files chương trình chính & xử lý logic
├── Docs/                    # Tài liệu kỹ thuật, hướng dẫn sử dụng và giao thức UI
│   ├── COMMUNICATION_PROTOCOL_FOR_UI.md
│   ├── HUONG_DAN_SU_DUNG.md
│   └── TECHNICAL_ANALYSIS_SAMSUNG_LG.md
├── EWARM/                   # Workspace và cấu hình dự án IAR Embedded Workbench
│   └── eziservo_ioboard.eww # IAR Workspace file
├── Library/                 # Thư viện CMSIS, HAL Driver, FreeRTOS, LwIP, Flash driver
├── build_iar.bat            # Script tự động build firmware bằng IAR CLI
├── setup_git.bat            # Script khởi tạo Git & tải lên GitHub
└── .gitignore               # Bộ lọc bỏ qua file build tạm và file cấu hình cá nhân
```

## 🛠 Hướng dẫn biên dịch (Build)

1. Mở file workspace `EWARM/eziservo_ioboard.eww` bằng **IAR Embedded Workbench for ARM**.
2. Chọn cấu hình **Release** (hoặc Debug).
3. Nhấn **F7** (Make) để biên dịch.
4. Hoặc chạy file tự động build `build_iar.bat`. File nhị phân `.bin` sẽ được tạo tại `EWARM/Release/Exe/eziservo_ioboard.bin`.
