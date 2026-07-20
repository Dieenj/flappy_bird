# 🐤 Flappy Bird trên STM32F429I-DISCOVERY

Game **Flappy Bird** chạy trực tiếp trên kit **STM32F429I-DISCOVERY** (STM32F429ZIT6), xây dựng bằng **TouchGFX** + **FreeRTOS**.

- Màn hình LCD TFT 2.4" 320×240 (ILI9341), cảm ứng điện trở (STMPE811), chạy mượt 60 FPS.
- Điều khiển: **chạm màn hình** để chim bay lên, **nút USER (PA0)** để tạm dừng/tiếp tục.
- Phản hồi LED: LED xanh (PG13) nháy khi ghi điểm, LED đỏ (PG14) nháy khi va chạm, cả hai nháy khi lên cấp.
- 6 cấp độ tự động tăng theo điểm (Easy → HELL), có ống dao động lên xuống từ 12 điểm.
- Kiến trúc Model–View–Presenter, logic game tách khỏi HAL → chạy được cả trên **simulator PC** lẫn board thật.

Chi tiết thiết kế xem file [BaoCao_FlappyBird.md](BaoCao_FlappyBird.md).

## Phần cứng yêu cầu

| Thành phần | Ghi chú |
|-----------|---------|
| Kit STM32F429I-DISCOVERY (rev D01) | Đã tích hợp sẵn LCD, cảm ứng, SDRAM 8MB, nút USER, LED — **không cần mạch ngoài** |
| Cáp USB Mini-B | Cấp nguồn + nạp code qua ST-LINK/V2-B tích hợp trên kit |

## Công cụ phát triển (version)

| Tool | Version | Ghi chú |
|------|---------|---------|
| STM32CubeMX | **6.18.0** | Cấu hình ngoại vi, sinh code từ file `.ioc` |
| TouchGFX Designer (X-CUBE-TOUCHGFX) | **4.26.1** | Thiết kế UI, generate code đồ họa, chạy simulator |
| STM32CubeIDE | 1.x (mới nhất) | Build + debug + nạp board (project mặc định cấu hình cho CubeIDE) |
| STM32CubeProgrammer | mới nhất | Cần cài nếu muốn nạp board trực tiếp từ TouchGFX Designer |
| FreeRTOS (CMSIS-RTOS V2) | đi kèm gói CubeMX | Đã có sẵn trong repo (`Middlewares/`) |

> Cũng hỗ trợ EWARM (≥ 8.50.9) và MDK-ARM — mở file `.ioc` bằng CubeMX và đổi Toolchain nếu cần.

## Cài đặt & chạy

### 1. Clone repo

```bash
git clone https://github.com/Dieenj/flappy_bird.git
```

### 2. Generate code TouchGFX

Một số thư mục sinh tự động (`TouchGFX/generated/`, `TouchGFX/simulator/`, thư viện TouchGFX trong `Middlewares/ST/`) **không đưa lên git**, cần generate lại:

1. Mở file `TouchGFX/FlappyBird.touchgfx` bằng **TouchGFX Designer 4.26.1**.
2. Bấm **Generate Code** (Ctrl+F4 hoặc Ctrl+G).

### 3. Chạy thử trên PC (simulator — không cần kit)

Trong TouchGFX Designer bấm **Run Simulator** (F5). Chuột trái = chạm màn hình. (Trên simulator không có nút PA0/LED.)

### 4. Build & nạp lên board

**Cách 1 — STM32CubeIDE (khuyên dùng):**
1. Mở STM32CubeIDE → *File → Import → Existing Projects into Workspace* → chọn thư mục `STM32CubeIDE/` của repo.
2. Cắm kit vào máy tính qua cổng USB ST-LINK.
3. Bấm **Run** (hoặc Debug) — IDE sẽ build và nạp firmware xuống board.

**Cách 2 — Nạp trực tiếp từ TouchGFX Designer:**
1. Cài STM32CubeProgrammer (thêm vào PATH).
2. Trong TouchGFX Designer bấm **Run Target** (F6) — Designer sẽ build bằng GCC và flash qua ST-LINK.

### 5. Chơi game

| Thao tác | Chức năng |
|----------|-----------|
| Chạm màn hình | Bắt đầu chơi / chim bay lên / Retry sau khi thua |
| Nút USER (màu xanh, PA0) | Tạm dừng ↔ tiếp tục (có đếm ngược 3-2-1-GO!) |
| Nút Home/Resume/Retry trên màn hình | Điều hướng menu Pause / Game Over |

## Cấu trúc thư mục chính

```
├── Core/                  # main.c, cấu hình clock/ngoại vi, FreeRTOS task
├── Drivers/               # HAL STM32F4, BSP (ILI9341, STMPE811), CMSIS
├── Middlewares/           # FreeRTOS
├── TouchGFX/
│   ├── FlappyBird.touchgfx    # Project TouchGFX Designer
│   ├── gui/                   # ★ Code game tự viết (MVP)
│   │   ├── include/gui/model/GameEngine.hpp   # Toàn bộ logic game
│   │   ├── src/game_screen/   # GameView, GamePresenter (PA0, LED)
│   │   └── src/welcome_screen/
│   └── target/            # HAL TouchGFX cho board (LTDC, DMA2D, touch)
├── STM32CubeIDE/          # Project STM32CubeIDE
├── STM32F429I_DISCO_REV_D01.ioc   # File cấu hình CubeMX
└── BaoCao_FlappyBird.md   # Báo cáo đồ án chi tiết
```

## Tác giả

| | |
|---|---|
| **Sinh viên thực hiện** | Nguyễn Đăng Diện — MSSV 20225805 |
| **Giảng viên hướng dẫn** | Thầy Đỗ Công Thuần |

Project do một mình thực hiện toàn bộ: game engine, UI TouchGFX, cấu hình phần cứng (CubeMX, FreeRTOS, PA0, LED), kiểm thử và báo cáo.

## Demo

- 🎬 Video demo: *(cập nhật link)*
- 🖼 Ảnh chạy thực tế: *(cập nhật)*
