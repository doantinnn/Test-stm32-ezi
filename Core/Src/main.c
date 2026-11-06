/**
  ******************************************************************************
  * @file    main.c
  * @brief   Main Application for 3-Axis Ezi-SERVOII Plus-E UDP Motion Control
  *          Axes:
  *            - Axis X (Index 0): Infeed Axis (Đưa hàng từ ngoài vào máy)
  *            - Axis Y (Index 1): Dispensing Head (Lên/xuống để tra keo)
  *            - Axis Z (Index 2): Outfeed Axis (Đưa hàng ra bên ngoài)
  *          Hardware: E7502 IO Extension Board / STM32F767ZI (TOP ENGINEERING)
  ******************************************************************************
  */

#include "main.h"
#include <stdio.h>
#include "UI_Server.h"
#include "CmdSet_Process.h"

/* Global Network Interface Handle */
struct netif gnetif;

/* Global Ezi-SERVO Driver Handles & Monitored Variables for 3 Axes */
EziServo_Handle_t g_ezi_servo[NUM_AXES];
volatile EziServo_AllStatus_t g_driver_st[NUM_AXES];
volatile int32_t  g_driver_act_pos[NUM_AXES] = {0, 0, 0};
volatile int32_t  g_driver_cmd_pos[NUM_AXES] = {0, 0, 0};
volatile uint32_t g_driver_axis_status[NUM_AXES] = {0, 0, 0};
volatile uint32_t g_driver_alarm_code[NUM_AXES] = {0, 0, 0};
volatile uint32_t g_driver_io_inputs[NUM_AXES] = {0, 0, 0};
volatile uint32_t g_driver_io_outputs[NUM_AXES] = {0, 0, 0};
volatile uint32_t g_udp_rx_packet_count[NUM_AXES] = {0, 0, 0};
volatile int32_t  g_driver_last_res[NUM_AXES] = {0, 0, 0};
volatile uint32_t g_driver_comm_status[NUM_AXES] = {0, 0, 0};

/* IP address & Name mapping for each axis */
const char * const g_axis_ips[NUM_AXES] = {
    DRIVER_X_IP_ADDR, /* Trục X (Infeed): 192.168.0.2 */
    DRIVER_Y_IP_ADDR, /* Trục Y (Dispense): 192.168.0.3 */
    DRIVER_Z_IP_ADDR  /* Trục Z (Outfeed): 192.168.0.4 */
};

const char * const g_axis_names[NUM_AXES] = {
    "Axis-X (Infeed)",
    "Axis-Y (Dispense)",
    "Axis-Z (Outfeed)"
};

/* ==============================================================================
 * OPERATING MODE & MANUAL JOG DEFINITIONS & LIVE WATCH VARIABLES
 * ============================================================================== */
typedef enum {
    OP_MODE_MANUAL = 0,   /* Chế độ BẰNG TAY (Jog / Move qua UI Server hoặc Live Watch) */
    OP_MODE_AUTO   = 1    /* Chế độ TỰ ĐỘNG (Chạy theo chu trình máy tra keo 3 trục) */
} OperationMode_t;

typedef enum {
    JOG_STATE_IDLE    = 0,  /* Động cơ đang dừng */
    JOG_STATE_FORWARD = 1,  /* Đang Jog tiến (+) */
    JOG_STATE_REVERSE = 2   /* Đang Jog lùi (-) */
} JogState_t;

/* Mode Variables for IAR Live Watch */
volatile uint32_t g_operation_mode = OP_MODE_MANUAL;        // 0 = MANUAL, 1 = AUTO
volatile uint32_t g_mode_switch_trigger = 0;                // Gõ 1 để đảo chế độ MANUAL <-> AUTO
volatile uint32_t g_mode_switch_pending = MODE_SWITCH_NONE; // 0 = None, 1 = Chờ hết chu trình Auto, 2 = Chờ hết Manual

/* Jog Variables per Axis */
volatile uint32_t g_jog_speed_pps[NUM_AXES] = { 10000, 10000, 10000 };
volatile uint32_t g_jog_state[NUM_AXES]     = { JOG_STATE_IDLE, JOG_STATE_IDLE, JOG_STATE_IDLE };
volatile int32_t  g_jog_last_cmd_res        = 0;

/* Automated 3-Axis Glue Dispensing Machine Coordinates & Timing */
volatile int32_t  g_pos_x_load    = 0;       // Vị trí trục X chờ nhận hàng ngoài máy (pulse)
volatile int32_t  g_pos_x_work    = 50000;   // Vị trí trục X đưa hàng vào trạm tra keo trong máy (pulse)
volatile int32_t  g_pos_y_safe    = 0;       // Vị trí trục Y nâng lên an toàn (pulse)
volatile int32_t  g_pos_y_disp    = 20000;   // Vị trí trục Y hạ xuống sát sản phẩm để tra keo (pulse)
volatile int32_t  g_pos_z_wait    = 0;       // Vị trí trục Z chờ trong máy (pulse)
volatile int32_t  g_pos_z_out     = 50000;   // Vị trí trục Z đẩy sản phẩm ra ngoài máy (pulse)

volatile uint32_t g_speed_x_pps   = 20000;   // Tốc độ trục X đưa hàng (pps)
volatile uint32_t g_speed_y_pps   = 10000;   // Tốc độ trục Y lên/xuống (pps)
volatile uint32_t g_speed_z_pps   = 20000;   // Tốc độ trục Z đưa hàng ra (pps)
volatile uint32_t g_dispense_time_ms = 1000; // Thời gian dừng tra keo (ms)

volatile uint32_t g_auto_cycle_trigger = 0;  // 1 = Chạy 1 chu trình, 2 = Chạy liên tục, 0 = Dừng
volatile uint32_t g_auto_cycle_count   = 0;  // Đếm số chu trình sản phẩm hoàn thành
volatile uint32_t g_auto_step          = 0;  // Bước hiện tại trong chu trình tra keo

/* Driver CN1 Pushbutton Configuration (START & STOP) */
volatile uint32_t g_driver_in_btn_enable  = 1; // 1 = Bật đọc nút START/STOP từ CN1 Driver, 0 = Tắt
volatile uint32_t g_driver_in_start_bit   = 4; // Vị trí bit của nút START trên CN1 (mặc định Bit 4 = IN0 / PT0)
volatile uint32_t g_driver_in_stop_bit    = 5; // Vị trí bit của nút STOP trên CN1 (mặc định Bit 5 = IN1 / PT1)
volatile uint32_t g_driver_in_active_high = 1; // 1 = Nhấn nút thì bit lên 1 (Active High), 0 = Active Low

/* Live Watch Quick Start & Stop Triggers */
volatile uint32_t g_btn_start_trigger     = 0; // Gõ 1 để START Auto Cycle từ Live Watch
volatile uint32_t g_btn_stop_trigger      = 0; // Gõ 1 để STOP Auto Cycle & Motion từ Live Watch

/* Interactive Command Testing from IAR Live Watch */
volatile uint32_t g_user_cmd_axis = 0;          // Chọn trục test trên Live Watch: 0 = X, 1 = Y, 2 = Z, 3 = ALL
volatile uint32_t g_user_cmd_trigger = 0;       // Nhập số 1..18 vào đây trên Live Watch
volatile uint32_t g_user_cmd_last_executed = 0; // Lưu mã lệnh vừa gửi
volatile int32_t  g_user_cmd_res = 0;           // Kết quả trả về (0 = EZI_OK)

/* Private Function Prototypes */
static void SystemClock_Config(void);
static void LwIP_Init(void);
static void EziServo_MultiAxisInit(void);

/**
  * @brief  Request operation mode switch with safe cycle-completion guarantee
  *         When switching from AUTO -> MANUAL, finishes current Auto cycle (all axes return to safe home).
  *         When switching from MANUAL -> AUTO, waits for any active manual move to finish.
  * @param  target_mode: OP_MODE_MANUAL (0) or OP_MODE_AUTO (1)
  */
void Request_OperationModeSwitch(uint32_t target_mode)
{
    if (target_mode == g_operation_mode && g_mode_switch_pending == MODE_SWITCH_NONE) {
        return;
    }

    if (target_mode == OP_MODE_MANUAL) {
        /* YÊU CẦU CHUYỂN SANG MANUAL:
         * Kiểm tra xem chu trình Auto 3 trục có đang chạy không.
         * Nếu chu trình đang chạy, KHÔNG NGẮT GIỮA CHỪNG mà đánh dấu PENDING và chờ hoàn thành chu trình. */
        bool any_moving = false;
        for (int i = 0; i < NUM_AXES; i++) {
            if (g_driver_st[i].axis_status & FLAG_MOTION_MOVING) {
                any_moving = true;
                break;
            }
        }
        bool auto_running = (g_auto_cycle_trigger != 0) || (g_auto_step != 0) || any_moving;

        if (auto_running) {
            g_mode_switch_pending = MODE_SWITCH_TO_MANUAL;
            if (g_auto_cycle_trigger == 2) {
                /* Đang chạy lặp vô hạn -> Chuyển thành chạy nốt chu trình hiện tại rồi dừng tại vị trí an toàn */
                g_auto_cycle_trigger = 1;
            }
            printf("[MODE] >>> Switch to MANUAL requested! Finishing current 3-Axis Auto cycle... [PENDING]\r\n");
        } else {
            /* Chu trình Auto đã dừng / đang rảnh -> Chuyển ngay lập tức */
            g_operation_mode = OP_MODE_MANUAL;
            g_mode_switch_pending = MODE_SWITCH_NONE;
            printf("[MODE] => Switched to MANUAL mode! (Manual motions UNLOCKED on all 3 axes)\r\n");
        }
    }
    else if (target_mode == OP_MODE_AUTO) {
        /* YÊU CẦU CHUYỂN SANG AUTO:
         * Kiểm tra xem có trục nào đang di chuyển thủ công không. */
        bool manual_moving = false;
        for (int i = 0; i < NUM_AXES; i++) {
            if ((g_jog_state[i] != JOG_STATE_IDLE) || (g_driver_st[i].axis_status & FLAG_MOTION_MOVING)) {
                manual_moving = true;
                if (g_jog_state[i] != JOG_STATE_IDLE) {
                    EziServo_MoveStop(&g_ezi_servo[i]);
                    g_jog_state[i] = JOG_STATE_IDLE;
                }
            }
        }

        if (manual_moving) {
            g_mode_switch_pending = MODE_SWITCH_TO_AUTO;
            printf("[MODE] >>> Switch to AUTO requested! Waiting for manual motion to complete... [PENDING]\r\n");
        } else {
            g_operation_mode = OP_MODE_AUTO;
            g_mode_switch_pending = MODE_SWITCH_NONE;
            printf("[MODE] => Switched to AUTO mode! (All manual motions are now LOCKED)\r\n");
        }
    }
}

/**
  * @brief  Main application entry point
  */
int main(void)
{
    /* 1. MCU Core Initialization */
    HAL_Init();
    SystemClock_Config();

    /* 2. Initialize IO Extension Board Hardware */
    IOBoard_Init();
    IOBoard_UART_Init(115200);

    /* 3. Startup Banner over RS-232 */
    printf("\r\n======================================================\r\n");
    printf("   TOP ENGINEERING - E7502 IO EXTENSION BOARD         \r\n");
    printf("   STM32F767ZI 3-AXIS (X, Y, Z) GLUE DISPENSING UNIT  \r\n");
    printf("======================================================\r\n");
    printf("[SYS] System Clock: 216 MHz (HSE = 25 MHz Crystal)\r\n");
    printf("[SYS] Board ID: %d (STM32 IP: %s)\r\n", 
           IOBoard_GetBoardID(), STM32_IP_ADDR);
    printf("[SYS] Configured Axes:\r\n");
    printf("   - Axis X (Infeed)   : %s:%d\r\n", DRIVER_X_IP_ADDR, DRIVER_UDP_PORT);
    printf("   - Axis Y (Dispense) : %s:%d\r\n", DRIVER_Y_IP_ADDR, DRIVER_UDP_PORT);
    printf("   - Axis Z (Outfeed)  : %s:%d\r\n", DRIVER_Z_IP_ADDR, DRIVER_UDP_PORT);

    /* 4. Initialize LwIP TCP/IP Stack & Ethernet Interface (DP83848 MII) */
    LwIP_Init();

    /* 5. Wait for Ethernet Link Up */
    printf("\r\n[ETH] Waiting for Ethernet Link Up...\r\n");
    uint32_t link_wait_start = HAL_GetTick();
    while (!netif_is_link_up(&gnetif) && (HAL_GetTick() - link_wait_start < 2000)) {
        ethernetif_set_link(&gnetif);
        HAL_Delay(100);
    }
    netif_set_up(&gnetif);
    netif_set_link_up(&gnetif);
    printf("[ETH] Link is UP! Connected to Driver subnet.\r\n");

    /* 6. Initialize 3-Axis Ezi-SERVO Drivers */
    EziServo_MultiAxisInit();

    /* 7. Initialize PC UI Communication Server (Ethernet UDP 5000 + RS-232 UART) */
    UI_Server_Init();

    /* 8. Infinite Loop - Network Processing & Real-time Diagnostic Monitor */
    uint32_t monitor_tick = HAL_GetTick();
    uint32_t print_tick   = HAL_GetTick();

    while (1)
    {
        /* Process incoming Ethernet frames */
        ethernetif_input(&gnetif);

        /* Process LwIP background timers */
        sys_check_timeouts();

        /* Process UI Communication Server (RS-232 UART + Ethernet UDP) */
        UI_Server_Process();

        /* Process Command Engine (dispatches pending commands & generates response) */
        CMD_Process();

        /* 1. Mode Switch Trigger (Gõ 1 vào g_mode_switch_trigger để đảo chế độ MANUAL <-> AUTO) */
        if (g_mode_switch_trigger != 0) {
            uint32_t target_mode = (g_operation_mode == OP_MODE_MANUAL) ? OP_MODE_AUTO : OP_MODE_MANUAL;
            Request_OperationModeSwitch(target_mode);
            g_mode_switch_trigger = 0;
        }

        /* 1b. Live Watch Direct START & STOP Triggers */
        if (g_btn_start_trigger != 0) {
            g_btn_start_trigger = 0;
            Request_OperationModeSwitch(OP_MODE_AUTO);
            g_auto_cycle_trigger = 1;
            g_auto_step = 0;
            printf("[TRIGGER] => START Auto Cycle activated from Live Watch!\r\n");
        }
        if (g_btn_stop_trigger != 0) {
            g_btn_stop_trigger = 0;
            g_auto_cycle_trigger = 0;
            g_auto_step = 0;
            g_mode_switch_pending = MODE_SWITCH_NONE;
            for (int i = 0; i < NUM_AXES; i++) {
                g_jog_state[i] = JOG_STATE_IDLE;
                if (g_ezi_servo[i].is_initialized) {
                    EziServo_MoveStop(&g_ezi_servo[i]);
                }
            }
            printf("[TRIGGER] => STOP Auto Cycle & Motion on all 3 axes from Live Watch!\r\n");
        }

        /* 2. Giám sát khi người dùng sửa trực tiếp biến g_operation_mode trong Live Watch */
        static uint32_t s_prev_op_mode = OP_MODE_MANUAL;
        if (g_operation_mode != s_prev_op_mode) {
            uint32_t req_mode = g_operation_mode;
            g_operation_mode = s_prev_op_mode; // Khôi phục lại tạm thời để đưa qua kiểm tra hoàn tất chu trình
            Request_OperationModeSwitch(req_mode);
            s_prev_op_mode = g_operation_mode;
        }

        /* 2b. Kiểm tra hoàn tất chu trình để chốt chuyển đổi chế độ (Pending Mode Switch Processor) */
        if (g_mode_switch_pending == MODE_SWITCH_TO_MANUAL) {
            /* Điều kiện hoàn thành Auto: trigger = 0, step = 0, và cả 3 motor đã dừng hẳn */
            bool all_stopped = true;
            for (int i = 0; i < NUM_AXES; i++) {
                if (g_driver_st[i].axis_status & FLAG_MOTION_MOVING) {
                    all_stopped = false;
                    break;
                }
            }
            if ((g_auto_cycle_trigger == 0) && (g_auto_step == 0) && all_stopped) {
                g_operation_mode = OP_MODE_MANUAL;
                s_prev_op_mode = OP_MODE_MANUAL;
                g_mode_switch_pending = MODE_SWITCH_NONE;
                printf("[MODE] => 3-Axis Auto cycle COMPLETED! Switched to MANUAL mode (Manual motions UNLOCKED).\r\n");
            }
        }
        else if (g_mode_switch_pending == MODE_SWITCH_TO_AUTO) {
            /* Điều kiện hoàn thành Manual: tất cả jog idle và cả 3 motor đã dừng hẳn */
            bool all_manual_done = true;
            for (int i = 0; i < NUM_AXES; i++) {
                if ((g_jog_state[i] != JOG_STATE_IDLE) || (g_driver_st[i].axis_status & FLAG_MOTION_MOVING)) {
                    all_manual_done = false;
                    break;
                }
            }
            if (all_manual_done) {
                g_operation_mode = OP_MODE_AUTO;
                s_prev_op_mode = OP_MODE_AUTO;
                g_mode_switch_pending = MODE_SWITCH_NONE;
                printf("[MODE] => Manual motions COMPLETED! Switched to AUTO mode (Manual motions LOCKED).\r\n");
            }
        }

        /* 3. Motion Mode Execution */
        if (g_operation_mode == OP_MODE_MANUAL) {
            /* CHẾ ĐỘ BẰNG TAY (MANUAL): Sẵn sàng nhận lệnh Jog/Move độc lập từng trục qua UI hoặc Live Watch */
        }
        else if (g_operation_mode == OP_MODE_AUTO) {
            /* --- CHẾ ĐỘ TỰ ĐỘNG (3-AXIS AUTOMATED GLUE DISPENSING MACHINE) --- */
            /* Dừng mọi chuyển động manual đang có nếu vô tình còn tồn đọng */
            for (int i = 0; i < NUM_AXES; i++) {
                if (g_jog_state[i] != JOG_STATE_IDLE && g_ezi_servo[i].is_initialized) {
                    EziServo_MoveStop(&g_ezi_servo[i]);
                    g_jog_state[i] = JOG_STATE_IDLE;
                }
            }

            static uint32_t s_auto_dwell_tick = 0;
            if (g_auto_cycle_trigger != 0) {
                switch (g_auto_step) {
                    case 0: /* BƯỚC 0: Khởi động chu trình & Kiểm tra an toàn */
                        /* Đảm bảo Servo cả 3 trục đều BẬT */
                        for (int i = 0; i < NUM_AXES; i++) {
                            if (g_ezi_servo[i].is_initialized && !(g_driver_axis_status[i] & FLAG_SERVO_ON)) {
                                EziServo_SetEnable(&g_ezi_servo[i], true);
                            }
                        }
                        /* QUY TẮC AN TOÀN TUYỆT ĐỐI: Trục Y (Đầu tra keo) phải ở vị trí nâng an toàn trước khi X, Z chạy */
                        if (g_driver_act_pos[AXIS_Y] != g_pos_y_safe) {
                            EziServo_MoveAbs(&g_ezi_servo[AXIS_Y], g_pos_y_safe, g_speed_y_pps);
                        }
                        g_auto_step = 1;
                        break;

                    case 1: /* Chờ trục Y lên vị trí an toàn xong */
                        if (!(g_driver_st[AXIS_Y].axis_status & FLAG_MOTION_MOVING)) {
                            /* BƯỚC 1: TRỤC X ĐƯA HÀNG TỪ NGOÀI VÀO VỊ TRÍ TRA KEO TRONG MÁY */
                            printf("[AUTO] Step 1: Axis-X feeding product to work station (%ld pulse)...\r\n", (long)g_pos_x_work);
                            EziServo_MoveAbs(&g_ezi_servo[AXIS_X], g_pos_x_work, g_speed_x_pps);
                            g_auto_step = 2;
                        }
                        break;

                    case 2: /* Chờ trục X đến vị trí làm việc trong máy */
                        if (!(g_driver_st[AXIS_X].axis_status & FLAG_MOTION_MOVING)) {
                            s_auto_dwell_tick = HAL_GetTick();
                            g_auto_step = 3;
                        }
                        break;

                    case 3: /* Dwell 200ms ổn định phôi -> BƯỚC 2: TRỤC Y HẠ XUỐNG VỊ TRÍ TRA KEO */
                        if (HAL_GetTick() - s_auto_dwell_tick >= 200) {
                            printf("[AUTO] Step 2: Axis-Y descending to dispensing position (%ld pulse)...\r\n", (long)g_pos_y_disp);
                            EziServo_MoveAbs(&g_ezi_servo[AXIS_Y], g_pos_y_disp, g_speed_y_pps);
                            g_auto_step = 4;
                        }
                        break;

                    case 4: /* Chờ trục Y hạ xuống xong -> BƯỚC 3: THỰC HIỆN TRA KEO (DISPENSING DWELL) */
                        if (!(g_driver_st[AXIS_Y].axis_status & FLAG_MOTION_MOVING)) {
                            printf("[AUTO] Step 3: Dispensing glue in progress (%lu ms)...\r\n", (unsigned long)g_dispense_time_ms);
                            s_auto_dwell_tick = HAL_GetTick();
                            g_auto_step = 5;
                        }
                        break;

                    case 5: /* Chờ hết thời gian tra keo -> BƯỚC 4: TRỤC Y NÂNG LÊN VỊ TRÍ AN TOÀN */
                        if (HAL_GetTick() - s_auto_dwell_tick >= g_dispense_time_ms) {
                            printf("[AUTO] Step 4: Dispensing done. Axis-Y retracting up to safe position...\r\n");
                            EziServo_MoveAbs(&g_ezi_servo[AXIS_Y], g_pos_y_safe, g_speed_y_pps);
                            g_auto_step = 6;
                        }
                        break;

                    case 6: /* Chờ trục Y nhấc lên vị trí an toàn xong */
                        if (!(g_driver_st[AXIS_Y].axis_status & FLAG_MOTION_MOVING)) {
                            s_auto_dwell_tick = HAL_GetTick();
                            g_auto_step = 7;
                        }
                        break;

                    case 7: /* Dwell 200ms -> BƯỚC 5: TRỤC Z ĐƯA HÀNG RA BÊN NGOÀI MÁY, ĐỒNG THỜI TRỤC X LÙI VỀ GỐC */
                        if (HAL_GetTick() - s_auto_dwell_tick >= 200) {
                            printf("[AUTO] Step 5: Axis-Z ejecting finished product out (%ld pulse)...\r\n", (long)g_pos_z_out);
                            EziServo_MoveAbs(&g_ezi_servo[AXIS_Z], g_pos_z_out, g_speed_z_pps);
                            /* Trục X lùi về vị trí chờ nhận hàng ngoài máy */
                            EziServo_MoveAbs(&g_ezi_servo[AXIS_X], g_pos_x_load, g_speed_x_pps);
                            g_auto_step = 8;
                        }
                        break;

                    case 8: /* Chờ trục Z đẩy hàng ra ngoài xong và trục X về gốc */
                        if (!(g_driver_st[AXIS_Z].axis_status & FLAG_MOTION_MOVING) &&
                            !(g_driver_st[AXIS_X].axis_status & FLAG_MOTION_MOVING)) {
                            s_auto_dwell_tick = HAL_GetTick();
                            g_auto_step = 9;
                        }
                        break;

                    case 9: /* Dwell 500ms nhả phôi -> BƯỚC 6: TRỤC Z LÙI VỀ VỊ TRÍ CHỜ TRONG MÁY */
                        if (HAL_GetTick() - s_auto_dwell_tick >= 500) {
                            printf("[AUTO] Step 6: Axis-Z returning to wait position in machine...\r\n");
                            EziServo_MoveAbs(&g_ezi_servo[AXIS_Z], g_pos_z_wait, g_speed_z_pps);
                            g_auto_step = 10;
                        }
                        break;

                    case 10: /* Chờ trục Z về vị trí chờ xong -> HOÀN TẤT 1 CHU TRÌNH SẢN PHẨM */
                        if (!(g_driver_st[AXIS_Z].axis_status & FLAG_MOTION_MOVING)) {
                            g_auto_cycle_count++;
                            printf("[AUTO] === CYCLE #%lu COMPLETED! Product finished successfully ===\r\n", 
                                   (unsigned long)g_auto_cycle_count);

                            if (g_auto_cycle_trigger == 1 || g_mode_switch_pending == MODE_SWITCH_TO_MANUAL) {
                                /* 1 cycle duy nhất hoặc đang chờ chuyển sang MANUAL -> Kết thúc an toàn */
                                g_auto_cycle_trigger = 0;
                                g_auto_step = 0;
                            } else {
                                /* Chạy liên tục (trigger == 2) -> Lặp lại từ bước 1 */
                                g_auto_step = 0;
                            }
                        }
                        break;

                    default:
                        g_auto_step = 0;
                        break;
                }
            } else {
                g_auto_step = 0;
            }
        }

        /* --- XỬ LÝ LỆNH KIỂM THỬ TƯƠNG TÁC TỪ LIVE WATCH --- */
        if (g_user_cmd_trigger != 0) {
            uint32_t cmd = g_user_cmd_trigger;
            uint32_t ax  = g_user_cmd_axis;
            g_user_cmd_last_executed = cmd;
            int cmd_res = EZI_OK;

            /* Khóa các lệnh chuyển động Manual khi đang ở chế độ AUTO */
            if (g_operation_mode == OP_MODE_AUTO && 
                (cmd == 5 || cmd == 6 || cmd == 7 || cmd == 10 || cmd == 11)) {
                printf("[AUTO LOCKED] Command %lu rejected! Manual motion is LOCKED in AUTO mode.\r\n", (unsigned long)cmd);
                cmd_res = EZI_ERR_PARAM;
            }
            else {
                switch (cmd) {
                    case 1: /* Bật Servo ON */
                        if (ax >= NUM_AXES) {
                            for (int i = 0; i < NUM_AXES; i++) EziServo_SetEnable(&g_ezi_servo[i], true);
                        } else {
                            cmd_res = EziServo_SetEnable(&g_ezi_servo[ax], true);
                        }
                        break;

                    case 2: /* Tắt Servo OFF */
                        if (ax >= NUM_AXES) {
                            for (int i = 0; i < NUM_AXES; i++) EziServo_SetEnable(&g_ezi_servo[i], false);
                        } else {
                            cmd_res = EziServo_SetEnable(&g_ezi_servo[ax], false);
                        }
                        break;

                    case 3: /* Reset Alarm */
                        if (ax >= NUM_AXES) {
                            for (int i = 0; i < NUM_AXES; i++) EziServo_AlarmReset(&g_ezi_servo[i]);
                        } else {
                            cmd_res = EziServo_AlarmReset(&g_ezi_servo[ax]);
                        }
                        break;

                    case 4: /* Đặt gốc tọa độ về 0 (Clear Pos) */
                        if (ax >= NUM_AXES) {
                            for (int i = 0; i < NUM_AXES; i++) EziServo_ClearPosition(&g_ezi_servo[i]);
                        } else {
                            cmd_res = EziServo_ClearPosition(&g_ezi_servo[ax]);
                        }
                        break;

                    case 5: /* Chạy tiến 10,000 pulse trên trục được chọn */
                        if (ax < NUM_AXES) {
                            cmd_res = EziServo_MoveAbs(&g_ezi_servo[ax], 10000, 5000);
                        }
                        break;

                    case 6: /* Chạy tiến 50,000 pulse trên trục được chọn */
                        if (ax < NUM_AXES) {
                            cmd_res = EziServo_MoveAbs(&g_ezi_servo[ax], 50000, 20000);
                        }
                        break;

                    case 7: /* Chạy quay về gốc 0 pulse trên trục được chọn */
                        if (ax < NUM_AXES) {
                            cmd_res = EziServo_MoveAbs(&g_ezi_servo[ax], 0, 20000);
                        }
                        break;

                    case 8: /* Dừng khẩn cấp Emergency Stop trên tất cả các trục */
                        for (int i = 0; i < NUM_AXES; i++) {
                            EziServo_EmergencyStop(&g_ezi_servo[i]);
                            g_jog_state[i] = JOG_STATE_IDLE;
                        }
                        g_auto_cycle_trigger = 0;
                        g_auto_step = 0;
                        g_mode_switch_pending = MODE_SWITCH_NONE;
                        break;

                    case 10: /* JOG+ trên trục được chọn */
                        if (ax < NUM_AXES) {
                            if (!(g_driver_axis_status[ax] & FLAG_SERVO_ON)) {
                                EziServo_SetEnable(&g_ezi_servo[ax], true);
                                HAL_Delay(10);
                            }
                            cmd_res = EziServo_MoveVelocity(&g_ezi_servo[ax], g_jog_speed_pps[ax], 1);
                            g_jog_state[ax] = JOG_STATE_FORWARD;
                        }
                        break;

                    case 11: /* JOG- trên trục được chọn */
                        if (ax < NUM_AXES) {
                            if (!(g_driver_axis_status[ax] & FLAG_SERVO_ON)) {
                                EziServo_SetEnable(&g_ezi_servo[ax], true);
                                HAL_Delay(10);
                            }
                            cmd_res = EziServo_MoveVelocity(&g_ezi_servo[ax], g_jog_speed_pps[ax], 0);
                            g_jog_state[ax] = JOG_STATE_REVERSE;
                        }
                        break;

                    case 12: /* Dừng JOG */
                        if (ax >= NUM_AXES) {
                            for (int i = 0; i < NUM_AXES; i++) {
                                EziServo_MoveStop(&g_ezi_servo[i]);
                                g_jog_state[i] = JOG_STATE_IDLE;
                            }
                        } else {
                            cmd_res = EziServo_MoveStop(&g_ezi_servo[ax]);
                            g_jog_state[ax] = JOG_STATE_IDLE;
                        }
                        break;

                    case 13: /* Đảo chế độ (Toggle MANUAL <-> AUTO) */
                        Request_OperationModeSwitch((g_operation_mode == OP_MODE_MANUAL) ? OP_MODE_AUTO : OP_MODE_MANUAL);
                        cmd_res = EZI_OK;
                        break;

                    case 14: /* Chuyển sang MANUAL */
                        Request_OperationModeSwitch(OP_MODE_MANUAL);
                        cmd_res = EZI_OK;
                        break;

                    case 15: /* Chuyển sang AUTO */
                        Request_OperationModeSwitch(OP_MODE_AUTO);
                        cmd_res = EZI_OK;
                        break;

                    case 16: /* Kích hoạt 3-Axis AUTO cycle 1 lần */
                        Request_OperationModeSwitch(OP_MODE_AUTO);
                        g_auto_cycle_trigger = 1;
                        cmd_res = EZI_OK;
                        break;

                    case 17: /* Kích hoạt 3-Axis AUTO cycle liên tục */
                        Request_OperationModeSwitch(OP_MODE_AUTO);
                        g_auto_cycle_trigger = 2;
                        cmd_res = EZI_OK;
                        break;

                    case 18: /* Dừng AUTO cycle */
                        g_auto_cycle_trigger = 0;
                        g_auto_step = 0;
                        g_mode_switch_pending = MODE_SWITCH_NONE;
                        for (int i = 0; i < NUM_AXES; i++) {
                            EziServo_MoveStop(&g_ezi_servo[i]);
                            g_jog_state[i] = JOG_STATE_IDLE;
                        }
                        break;

                    default:
                        cmd_res = EZI_ERR_PARAM;
                        break;
                }
            }

            g_user_cmd_res = cmd_res;
            g_user_cmd_trigger = 0; // Xóa cờ về 0 để sẵn sàng cho lần gõ tiếp theo
        }

        /* 20Hz (50ms) High-Speed Real-time UDP Query & Diagnostic Monitoring cho cả 3 Driver */
        if (HAL_GetTick() - monitor_tick >= 50) {
            monitor_tick = HAL_GetTick();

            for (int ax = 0; ax < NUM_AXES; ax++) {
                if (g_ezi_servo[ax].is_initialized) {
                    int res = EziServo_GetAllStatus(&g_ezi_servo[ax], (EziServo_AllStatus_t *)&g_driver_st[ax]);

                    g_driver_act_pos[ax]     = g_driver_st[ax].act_pos;
                    g_driver_cmd_pos[ax]     = g_driver_st[ax].cmd_pos;
                    g_driver_axis_status[ax] = g_driver_st[ax].axis_status;
                    g_driver_io_inputs[ax]   = g_driver_st[ax].input_status;
                    g_driver_io_outputs[ax]  = g_driver_st[ax].output_status;
                    g_driver_last_res[ax]    = res;

                    if (res == EZI_OK || res == EZI_ERR_STATUS) {
                        g_udp_rx_packet_count[ax]++;
                        g_driver_comm_status[ax] = g_ezi_servo[ax].rx_frame.comm_status;
                    }

                    if (g_driver_st[ax].axis_status & (FLAG_DRIVE_FAULT | FLAG_ERROR_ALL)) {
                        uint8_t alm = 0;
                        if (EziServo_GetAlarmType(&g_ezi_servo[ax], &alm) == EZI_OK) {
                            g_driver_alarm_code[ax] = alm;
                        }
                    } else {
                        g_driver_alarm_code[ax] = 0;
                    }
                }
            }

            /* --- XỬ LÝ NÚT NHẤN START & STOP NỐI TRỰC TIẾP VÀO DRIVER CN1 (Mặc định đọc trên Driver X) --- */
            static uint32_t s_prev_driver_inputs = 0;
            if (g_driver_in_btn_enable) {
                uint32_t curr_inputs = g_driver_st[AXIS_X].input_status;
                uint32_t start_mask  = (1UL << g_driver_in_start_bit);
                uint32_t stop_mask   = (1UL << g_driver_in_stop_bit);

                bool start_edge = false;
                bool stop_edge  = false;

                if (g_driver_in_active_high) {
                    if ((curr_inputs & start_mask) && !(s_prev_driver_inputs & start_mask)) start_edge = true;
                    if ((curr_inputs & stop_mask) && !(s_prev_driver_inputs & stop_mask)) stop_edge = true;
                } else {
                    if (!(curr_inputs & start_mask) && (s_prev_driver_inputs & start_mask)) start_edge = true;
                    if (!(curr_inputs & stop_mask) && (s_prev_driver_inputs & stop_mask)) stop_edge = true;
                }
                s_prev_driver_inputs = curr_inputs;

                if (start_edge) {
                    printf("[CN1 INPUT] >>> START BUTTON PRESSED! Starting 3-Axis Auto Cycle...\r\n");
                    Request_OperationModeSwitch(OP_MODE_AUTO);
                    g_auto_cycle_trigger = 1;
                    g_auto_step = 0;
                }

                if (stop_edge) {
                    printf("[CN1 INPUT] >>> STOP BUTTON PRESSED! Stopping Motion & Auto Cycle on all 3 axes...\r\n");
                    g_auto_cycle_trigger = 0;
                    g_auto_step = 0;
                    g_mode_switch_pending = MODE_SWITCH_NONE;
                    for (int i = 0; i < NUM_AXES; i++) {
                        g_jog_state[i] = JOG_STATE_IDLE;
                        if (g_ezi_servo[i].is_initialized) {
                            EziServo_MoveStop(&g_ezi_servo[i]);
                        }
                    }
                }
            }

            /* In log trạng thái định kỳ 1 giây/lần ra UART */
            if (HAL_GetTick() - print_tick >= 1000) {
                print_tick = HAL_GetTick();
                printf("[STATUS 3-AXIS] X(Infeed): %ld | Y(Dispense): %ld | Z(Outfeed): %ld | Mode: %s | Cycle: %lu\r\n",
                       (long)g_driver_act_pos[AXIS_X],
                       (long)g_driver_act_pos[AXIS_Y],
                       (long)g_driver_act_pos[AXIS_Z],
                       (g_operation_mode == OP_MODE_AUTO) ? "AUTO" : "MANUAL",
                       (unsigned long)g_auto_cycle_count);
            }
        }
    }
}

/**
  * @brief  Initialize UDP Communication for 3 Ezi-SERVOII Plus-E Drivers
  */
static void EziServo_MultiAxisInit(void)
{
    printf("\r\n======================================================\r\n");
    printf("   3-AXIS Ezi-SERVO INITIALIZATION & DIAGNOSTICS      \r\n");
    printf("======================================================\r\n");

    for (int i = 0; i < NUM_AXES; i++) {
        printf("\r\n[AXIS %d: %s] Connecting to IP: %s:%d...\r\n", 
               i, g_axis_names[i], g_axis_ips[i], DRIVER_UDP_PORT);
        int res = EziServo_Init(&g_ezi_servo[i], g_axis_ips[i], DRIVER_UDP_PORT);
        if (res == EZI_OK) {
            printf("  -> [PASS] Socket allocated for %s\r\n", g_axis_names[i]);

            /* Query initial status */
            for (int r = 0; r < 3; r++) {
                ethernetif_input(&gnetif);
                sys_check_timeouts();
                if (EziServo_GetAllStatus(&g_ezi_servo[i], (EziServo_AllStatus_t *)&g_driver_st[i]) == EZI_OK) {
                    printf("  -> [PASS] %s RESPONDED! ActPos: %ld | AxisStatus: 0x%08X\r\n",
                           g_axis_names[i], (long)g_driver_st[i].act_pos, (unsigned int)g_driver_st[i].axis_status);
                    break;
                }
                HAL_Delay(50);
            }
        } else {
            printf("  -> [FAIL] Cannot allocate socket for %s (Code: %d)\r\n", g_axis_names[i], res);
        }
    }
}

/**
  * @brief System Clock Configuration (216MHz from 25MHz HSE)
  */
static void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState       = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState   = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource  = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM       = 25;
    RCC_OscInitStruct.PLL.PLLN       = 432;
    RCC_OscInitStruct.PLL.PLLP       = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ       = 9;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    if (HAL_PWREx_EnableOverDrive() != HAL_OK)
    {
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                       RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
  * @brief  Initialize LwIP Stack with Static IP
  */
static void LwIP_Init(void)
{
    ip_addr_t ipaddr, netmask, gw;

    lwip_init();

    ipaddr_aton(STM32_IP_ADDR, &ipaddr);
    ipaddr_aton(STM32_NETMASK, &netmask);
    ipaddr_aton(STM32_GATEWAY, &gw);

    netif_add(&gnetif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &ethernet_input);
    netif_set_default(&gnetif);
    netif_set_up(&gnetif);
    netif_set_link_up(&gnetif);
}

/**
  * @brief Error Handler
  */
void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
        IOBoard_ResetAllOutputs();
    }
}
