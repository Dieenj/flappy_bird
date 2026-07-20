# BÁO CÁO ĐỒ ÁN: GAME FLAPPY BIRD TRÊN KIT STM32F429I-DISCOVERY

**Nhóm thực hiện:** *(điền tên các thành viên)*
**Ngày báo cáo:** 20/07/2026

---

## 1. Giới thiệu đề tài

### 1.1. Mô tả sản phẩm / mục tiêu của project

Project xây dựng trò chơi **Flappy Bird** chạy trực tiếp trên vi điều khiển
**STM32F429ZIT6** (kit **STM32F429I-DISCOVERY**), hiển thị trên màn hình LCD TFT
2.4" tích hợp sẵn trên kit, điều khiển bằng **cảm ứng chạm** và **nút bấm USER
(PA0)**, phản hồi sự kiện game bằng **LED trên board**.

Mục tiêu chính:
- Vận dụng kiến thức hệ thống nhúng: cấu hình ngoại vi STM32 (LTDC, DMA2D, FMC/SDRAM, SPI, I2C, GPIO, Timer), RTOS (FreeRTOS) và thư viện đồ họa **TouchGFX**.
- Xây dựng game engine thời gian thực chạy ổn định **60 FPS** trên MCU không có FPU mạnh và RAM hạn chế.
- Tổ chức phần mềm theo mô hình chuẩn **Model–View–Presenter (MVP)** của TouchGFX, tách biệt logic game khỏi giao diện để có thể chạy được cả trên **simulator PC** lẫn **board thật**.

### 1.2. Yêu cầu chức năng

| # | Yêu cầu | Mô tả |
|---|---------|-------|
| F1 | Chơi game Flappy Bird | Chim rơi theo trọng lực; chạm màn hình để chim bay lên (flap); tránh các cặp ống nước di chuyển từ phải sang trái. |
| F2 | Tính điểm | Mỗi lần vượt qua một ống được +1 điểm; hiển thị điểm hiện tại trên màn hình khi chơi; lưu và hiển thị **điểm cao nhất (Best)**. |
| F3 | Hệ thống cấp độ | 6 cấp độ (Easy → HELL) tự động tăng theo điểm số: khe hở ống hẹp dần (110 → 65 px), khoảng cách ống ngắn dần, tốc độ cuộn tăng theo √(điểm). |
| F4 | Ống di chuyển | Từ 12 điểm trở lên, xuất hiện ngẫu nhiên (35%) loại ống **dao động lên xuống** theo hàm sin (màu cam, phân biệt với ống thường màu xanh). |
| F5 | Màn hình chào (Welcome) | Màn hình bắt đầu; chạm để vào game. |
| F6 | Tạm dừng / tiếp tục | Nhấn nút **USER PA0** để pause; menu Pause có nút **Resume** (đếm ngược 3-2-1-GO! trước khi chơi tiếp) và nút **Home**. |
| F7 | Màn hình thua (Game Over) | Khi va chạm ống / trần / đất: hiển thị điểm, điểm cao nhất; sau ~0,7 s hiện nút **Retry** và **Home** (chống bấm nhầm). |
| F8 | Phản hồi LED | LED xanh (PG13) nháy khi ghi điểm; LED đỏ (PG14) nháy khi va chạm; cả hai nháy khi lên cấp và khi đếm ngược. |
| F9 | Đầu vào | Cảm ứng điện trở STMPE811 (chạm để flap, bấm nút menu); nút USER PA0 (pause/resume, có chống dội bằng phát hiện sườn). |
| F10 | Đầu ra | LCD TFT 320×240 16bpp (toàn bộ đồ họa game); 2 LED trạng thái. |

### 1.3. Yêu cầu phi chức năng

| # | Yêu cầu | Mô tả |
|---|---------|-------|
| N1 | Hiệu năng thời gian thực | Game chạy mượt ở 60 FPS (tick 16,6 ms), không giật/rớt khung hình khi có tối đa 5 cặp ống trên màn hình. |
| N2 | Độ cứng vững | Không treo/crash khi thao tác liên tục (spam chạm, bấm PA0 liên tục, pause giữa lúc chết...); máy trạng thái game (WELCOME/PLAYING/PAUSED/COUNTDOWN/DEAD) xử lý đầy đủ mọi chuyển trạng thái. |
| N2b | Tính tất định | Không cấp phát động trong vòng lặp game; số ống giới hạn cứng `MAX_PIPES = 5`; toán học (sqrt, sin) tự cài đặt bằng xấp xỉ, không phụ thuộc libm. |
| N3 | Độ hoàn thiện | Giao diện đầy đủ: welcome, HUD điểm, menu pause, đếm ngược, màn hình game-over; chim có animation vỗ cánh; ống có nắp (cap) và 2 màu phân loại. |
| N4 | Tính khả chuyển | Toàn bộ logic game nằm trong `GameEngine` thuần C++ (không phụ thuộc HAL); phần cứng (GPIO/LED) được cô lập bằng `#ifndef SIMULATOR` → build và chạy được trên simulator PC để phát triển nhanh. |
| N5 | Tài nguyên | Chạy trong giới hạn RAM nội + SDRAM 8 MB của kit; dùng double-buffering trên SDRAM. |
| N6 | Bảo trì | Tuân thủ kiến trúc MVP của TouchGFX; tham số gameplay (trọng lực, tốc độ, bảng cấp độ) gom thành hằng số ở đầu `GameEngine.hpp` dễ tinh chỉnh. |

---

## 2. Thiết kế

### 2.1. Phân chia chức năng phần cứng / phần mềm

| Chức năng | Phần cứng | Phần mềm |
|-----------|-----------|----------|
| Hiển thị đồ họa | LCD ILI9341 + LTDC (quét màn hình) + DMA2D/Chrom-ART (tô/copy pixel) + SDRAM (framebuffer) | TouchGFX render widget, quản lý invalidate vùng vẽ, double-buffering |
| Cảm ứng chạm | STMPE811 (đọc tọa độ, giao tiếp I2C3) | Driver đọc tọa độ, TouchGFX chuyển thành ClickEvent, View xử lý theo vùng nút |
| Nút pause | Nút USER nối PA0 (phần cứng có sẵn mạch kéo xuống) | Polling mỗi tick + phát hiện sườn lên (edge detect) để chống dội |
| Logic game (vật lý, va chạm, điểm, cấp độ) | — | `GameEngine` (thuần phần mềm) |
| Nhịp thời gian 60 Hz | TIM7 (timebase HAL) + LTDC VSYNC | TouchGFX tick → `handleTickEvent()` |
| Phản hồi sự kiện | LED PG13/PG14 | Presenter điều khiển bật/tắt theo bộ đếm tick |
| Sinh số ngẫu nhiên | Seed từ `HAL_GetTick()` | PRNG xorshift32 tự cài đặt |
| Đa nhiệm | — | FreeRTOS: task GUI (TouchGFX) + default task |

Nguyên tắc: **phần cứng lo đường dữ liệu tốc độ cao** (quét LCD, blit pixel — nếu làm bằng CPU sẽ không đủ 60 FPS), **phần mềm lo logic** (game engine, máy trạng thái, UI).

### 2.2. Thiết kế phần cứng

Project dùng nguyên kit STM32F429I-DISCOVERY (không cần mạch ngoài), gồm các module sau:

**Bảng module / linh kiện:**

| Module | Linh kiện | Giao tiếp với MCU | Địa chỉ / tốc độ |
|--------|-----------|-------------------|------------------|
| MCU | STM32F429ZIT6 (Cortex-M4F, 2 MB Flash, 256 KB RAM) | — | SYSCLK = **180 MHz** |
| Màn hình LCD 2.4" 320×240 | ILI9341 | **LTDC** (RGB565 song song 16 bit) — dữ liệu điểm ảnh; **SPI5** — lệnh cấu hình ban đầu | LTDC pixel clock ~6,4 MHz; SPI5 vài MHz (chỉ dùng lúc khởi tạo) |
| Cảm ứng điện trở | STMPE811 | **I2C3** | Địa chỉ slave **0x82** (8-bit), tốc độ chuẩn 100 kHz |
| SDRAM 8 MB (framebuffer) | IS42S16400J | **FMC** (SDRAM bank 2) | Base address **0xD0000000**, bus 16 bit, clock SDRAM = HCLK/2 = 90 MHz |
| Tăng tốc đồ họa | DMA2D (Chrom-ART, tích hợp trong MCU) | AHB | Truy cập trực tiếp SDRAM/Flash |
| Nút USER | B1 | GPIO **PA0** (input, mức tích cực cao) | Polling 60 Hz |
| LED xanh / đỏ | LD3 / LD4 | GPIO **PG13 / PG14** (output push-pull) | — |
| Timebase HAL | TIM7 | APB1 | Ngắt 1 kHz |
| CRC | Khối CRC nội | AHB | (TouchGFX yêu cầu) |
| Nạp/debug | ST-LINK/V2-B tích hợp | SWD | — |

**Sơ đồ ghép nối (khối):**

```
                    ┌──────────────────────────────────┐
                    │        STM32F429ZIT6 (180MHz)     │
   ┌─────────┐ SPI5 │  ┌──────┐   ┌───────┐  ┌──────┐  │
   │ ILI9341 │◄─────┼──┤ SPI5 │   │ LTDC  │  │DMA2D │  │
   │  LCD    │ RGB  │  └──────┘   └───┬───┘  └──┬───┘  │
   │ 320x240 │◄─────┼────────────────┘          │      │  FMC(16bit,90MHz)  ┌──────────┐
   └─────────┘      │            AHB bus ───────┴──────┼───────────────────►│ SDRAM 8MB │
   ┌─────────┐ I2C3 │  ┌──────┐                        │   0xD0000000       │(framebuf) │
   │STMPE811 │◄─────┼──┤ I2C3 │  ┌──────┐  ┌────────┐  │                    └──────────┘
   │ (touch) │ 0x82 │  └──────┘  │ TIM7 │  │  CRC   │  │
   └─────────┘      │            └──────┘  └────────┘  │
   [B1 USER]───PA0──►                                  │
   [LD3 green]◄─PG13─┤ GPIO                            │
   [LD4 red]  ◄─PG14─┤                                 │
                    └──────────────────────────────────┘
```

**Luồng dữ liệu hiển thị:** CPU/TouchGFX vẽ vào framebuffer trong SDRAM (qua FMC), DMA2D tăng tốc tô màu/copy, LTDC đọc framebuffer qua bus AHB và quét liên tục ra LCD theo chuẩn RGB song song, đồng bộ theo VSYNC. Chiến lược **double buffering**: vẽ vào buffer sau trong khi LTDC quét buffer trước, hoán đổi tại VSYNC → không bị xé hình (tearing).

### 2.3. Thiết kế phần mềm

**Kiến trúc tổng thể (MVP của TouchGFX + FreeRTOS):**

```
┌─────────────────────────────────────────────────────────┐
│ FreeRTOS                                                │
│  ├── defaultTask (idle)                                 │
│  └── GUI_Task ──► TouchGFX main loop (60Hz, theo VSYNC) │
│         │                                               │
│  ┌──────▼──────────  MVP  ─────────────────────────┐    │
│  │  Model ──── GameEngine (logic thuần, no HAL)    │    │
│  │    ▲                                            │    │
│  │  Presenter (GamePresenter / WelcomePresenter)   │    │
│  │    │   • đọc PA0, điều khiển LED                │    │
│  │    ▼   • gọi engine.tick() mỗi frame            │    │
│  │  View (GameView / WelcomeView)                  │    │
│  │        • vẽ chim/ống/overlay, nhận ClickEvent   │    │
│  └─────────────────────────────────────────────────┘    │
│  TouchGFX HAL ◄── LTDC / DMA2D / STMPE811 / SDRAM       │
└─────────────────────────────────────────────────────────┘
```

**Các module phần mềm chính:**

| Module | File | Chức năng |
|--------|------|-----------|
| Game Engine | `TouchGFX/gui/include/gui/model/GameEngine.hpp` | Máy trạng thái 5 trạng thái; vật lý chim (gravity 0.20, jump −4.0, terminal 12.0 px/tick); sinh/di chuyển/xóa ống; ống dao động sin; va chạm hình tròn–chữ nhật; điểm & bảng 6 cấp độ; tốc độ lerp theo √score; PRNG xorshift; sqrt/sin xấp xỉ tự viết |
| Model | `gui/src/model/Model.cpp` | Sở hữu GameEngine, seed PRNG bằng `HAL_GetTick()` (board) / `time()` (simulator) |
| Game Presenter | `gui/src/game_screen/GamePresenter.cpp` | Cầu nối HW–logic: đọc PA0 (edge detect), tiêu thụ event của engine (score/hit/levelup/countdown) → nháy LED; gọi `tick()` + `updateDisplay()` mỗi frame |
| Game View | `gui/src/game_screen/GameView.cpp` | Vẽ chim (4 widget + cánh animation 12 tick), 5×4 widget ống, HUD điểm, overlay Pause/Dead/Countdown; xử lý chạm theo vùng tọa độ nút |
| Welcome Screen | `gui/src/welcome_screen/` | Màn hình chào, chạm để vào game |
| Cấu hình hệ thống | `Core/Src/main.c`, `freertos.c` | Clock 180 MHz, init GPIO/CRC/I2C3/SPI5/FMC/LTDC/DMA2D/TIM7, tạo task FreeRTOS |
| TouchGFX generated | `TouchGFX/generated/` | Code sinh tự động từ TouchGFX Designer (layout widget, ảnh, font, text) |

**Máy trạng thái game (biểu đồ luồng chính):**

```
          tap                 va chạm (ống/trần/đất)
 WELCOME ─────► PLAYING ─────────────────────► DEAD
    ▲            │  ▲                           │
    │       PA0  │  │ đếm ngược xong            │ chờ ≥0.7s
    │            ▼  │ (3→2→1→GO)                ▼
    │          PAUSED ──Resume──► COUNTDOWN   [Retry]──► PLAYING (reset)
    │            │                              │
    └────Home────┴──────────────────────────────┘ [Home]
```

**Luồng xử lý mỗi tick (16,6 ms):**

```
VSYNC → handleTickEvent()
  1. Đọc PA0, phát hiện sườn lên → togglePause()
  2. engine.tick():
     - cập nhật tốc độ (lerp theo √score)
     - vật lý chim (vy += g; y += vy)
     - dịch ống sang trái; ống MOVING dao động sin
     - qua ống → +điểm, kiểm tra lên cấp
     - xóa ống ra khỏi màn, spawn ống mới theo khoảng cách cấp độ
     - kiểm tra va chạm → DEAD
  3. Tiêu thụ event → nháy LED tương ứng
  4. view.updateDisplay(): vẽ chim + ống + overlay theo trạng thái
```

---

## 3. Cài đặt / xây dựng hệ thống

### 3.1. GitHub & môi trường phát triển

- **Link repository:** *(điền link GitHub sau khi push, kèm file README.md)*
- Nội dung README cần có: hướng dẫn clone, mở project, build, nạp board; danh sách tool và version:

| Tool | Version |
|------|---------|
| STM32CubeMX | 6.18.0 |
| X-CUBE-TOUCHGFX (TouchGFX Designer) | **4.26.1** |
| STM32CubeIDE (hoặc EWARM ≥ 8.50.9 / MDK-ARM) | *(điền version dùng thực tế)* |
| STM32CubeProgrammer | *(điền version)* — dùng để nạp từ TouchGFX Designer |
| FreeRTOS (CMSIS-RTOS V2) | đi kèm gói CubeMX |

**Các bước cài đặt tóm tắt:** clone repo → mở `TouchGFX/*.part` bằng TouchGFX Designer (chỉnh UI, generate code) → mở project bằng STM32CubeIDE → Build → cắm kit qua USB ST-LINK → Run/Flash. Có thể chạy thử trên PC bằng nút **Run Simulator** trong TouchGFX Designer.

### 3.2. Mô tả các module phần mềm chính

1. **GameEngine** (`GameEngine.hpp`, ~300 dòng): toàn bộ logic game trong một class thuần C++, không include HAL — đây là điểm mấu chốt giúp chạy được trên cả simulator và board. Mảng ống tĩnh `Pipe _pipes[5]`, không cấp phát động. Sự kiện (ghi điểm, va chạm, lên cấp, đếm ngược) theo mẫu *consume-once flag* để Presenter đọc đúng một lần.
2. **GamePresenter**: lớp duy nhất chạm vào GPIO (PA0, LED PG13/PG14), bọc trong `#ifndef SIMULATOR`. Xử lý chống dội nút bằng so sánh trạng thái tick trước/tick này.
3. **GameView**: ánh xạ trạng thái engine → widget TouchGFX. Ống được vẽ bằng 4 Box/ống (thân trên, nắp trên, thân dưới, nắp dưới) × 5 ống = 20 widget, chỉ `invalidate()` vùng thay đổi để tiết kiệm băng thông vẽ.
4. **Khởi tạo hệ thống** (`main.c`): cấu hình clock 180 MHz, các ngoại vi LTDC/DMA2D/FMC/I2C3/SPI5/TIM7/CRC, tạo 2 task FreeRTOS; `TouchGFX_Task` chạy vòng lặp đồ họa.

### 3.3. Đóng góp của từng thành viên

*(Điền theo thực tế nhóm — thể hiện tương ứng ở số commit trên GitHub)*

| Thành viên | Công việc | Số commit |
|-----------|-----------|-----------|
| *(Tên 1)* | Game engine: vật lý, va chạm, cấp độ, máy trạng thái | |
| *(Tên 2)* | UI TouchGFX: thiết kế màn hình, GameView, animation | |
| *(Tên 3)* | Phần cứng/hệ thống: CubeMX, FreeRTOS, PA0, LED, nạp board | |
| *(Tên 4)* | Kiểm thử, quay demo, viết báo cáo & README | |

### 3.4. Kết quả

- **Video demo:** *(điền link YouTube/Drive)*
- **Ảnh chạy thực tế:** *(chèn ảnh các màn hình: Welcome, đang chơi, Pause, Countdown, Game Over)*
- Game chạy ổn định 60 FPS trên board, đầy đủ các chức năng F1–F10.

### 3.5. Đánh giá

**Mức độ đạt yêu cầu:** Đạt toàn bộ yêu cầu chức năng đề ra (gameplay, điểm/best, 6 cấp độ, ống di chuyển, pause bằng PA0, đếm ngược, game-over có Retry/Home, LED phản hồi) và các yêu cầu phi chức năng chính (60 FPS, không cấp phát động, chạy được cả simulator lẫn board).

**Ưu điểm:**
- Kiến trúc sạch: logic game tách hoàn toàn khỏi HAL → dễ test trên PC, dễ port.
- Gameplay hoàn thiện: độ khó tăng dần mượt (lerp tốc độ theo √score), ống dao động tạo thử thách, chống bấm nhầm sau khi chết.
- Tối ưu cho nhúng: toán học xấp xỉ tự viết, bộ nhớ tĩnh, chỉ invalidate vùng thay đổi.
- Tận dụng tốt phần cứng kit: DMA2D + double buffering trên SDRAM cho hình ảnh mượt, không tearing.

**Nhược điểm / hướng phát triển:**
- Chưa có **âm thanh** (kit không có codec audio; có thể thêm buzzer PWM hoặc DAC + loa ngoài).
- Điểm cao nhất **mất khi tắt nguồn** — có thể lưu vào Flash nội hoặc backup SRAM.
- Đồ họa dùng Box màu đơn giản thay vì sprite ảnh — có thể thay bằng ảnh bitmap cho đẹp hơn.
- Tọa độ vùng nút chạm đang hard-code trong `GameView::handleClickEvent` — nên chuyển sang widget Button của TouchGFX.
