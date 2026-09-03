# ĐÂY SẼ LÀ NỘI DUNG GHI CHÚ CỦA DỰ ÁN

## Mô tả dự án
> Asdevlab sẽ là phần mềm điều khiển kính thiên văn điẹn tử DIY của tôi (có thể coi giống edusnap astro hoặc seestar). Kết hợp repo OnstepX - điều khiển chân kính và LiveStacker - live hình ảnh từ kính và livestacker xuất hình ảnh cuối cùng nhanh chóng đơn giản mà ko cần phải sử dụng các phần mềm thiên văn chuyên nghiệp.
> Bước đầu là xây dựng phần mềm trên laptop dùng web UI, sau này có thể phát triển thành app android.
> Phần cứng của kính thiên văn điện tử  (tập trung vào OnstepX)
    > Hệ thống điều khiển chân kính: ESP32 38 chân dev kit, Stepper motor drive TMC2209 hoặc A4988 (vì chúng cho độ phân giải cao), Stepper motor sử dụng NEMA17 hoặc 28BYJ, cảm biến quang chữ U làm cảm biến home. Trong đó thêm (đây là những phần ko có trong onstepx) màn hình OLED 4 pin 0.96" I2C hiện trạng thái, flip mirror quan sát qua thị kính và camera.
    > Hệ thống kín thiên văn: hiện tại sử dụng camera usb ov5643 , focuser sử dụng 28BYJ và drive UNL2003 hoặc 2 loại drive tương tự bên trên. Tập trung vào camera usb uvc và camera điện thoại android, những loại khác phát triển sau.
    > Hỗ trợ ALT-AZ và EQ nếu có thể ko thì chỉ dùng ALT-AZ
> Phần mềm của kính thiên văn điện tử : ASDEVLAB
    > Giao tiếp giữa firmware và software
    > Cho phép Autofocus (sử dụng thuật toán tối ưu cho sao, các vật thể bề mặt, và scenery) và Manualfocus. 
    > Tôi ưu LiveStaking và xử lý ảnh tốt hơn nếu có thể.


1. Mục tiêu của asdevlab: 
> Asdevlab sẽ là phần mềm điều khiển kính thiên văn điẹn tử DIY của tôi (có thể coi giống edusnap hoặc seestar). Kết hợp repo OnstepX - điều khiển chân kính và LiveStacker - live hình ảnh từ kính và livestacker xuất hình ảnh cuối cùng nhanh chóng đơn giản mà ko cần phải sử dụng các phần mềm thiên văn chuyên nghiệp. Trong quá trình phát triển có thể lấy ý tưởng, core của một số dự án mã nguồn mở khác ví dụ tritongoto, astroshader - mã nguồn đóng nên chỉ tham khảo...
> Bước đầu là xây dựng phần mềm trên laptop dùng web UI, sau này có thể phát triển thành app điện thoại.
> Phần cứng của kính thiên văn điện tử  (tập trung vào OnstepX)
    + Hệ thống điều khiển chân kính: ESP32 38 chân dev kit, Stepper motor drive TMC2209 hoặc A4988 (vì chúng cho độ phân giải cao), Stepper motor sử dụng NEMA17 hoặc 28BYJ, cảm biến quang chữ U làm cảm biến home. Trong đó thêm (đây là những phần ko có trong onstepx) màn hình OLED 4 pin 0.96" I2C hiện trạng thái, flip mirror quan sát qua thị kính và camera.
    + Hệ thống kính thiên văn: hiện tại sử dụng camera usb ov5643 , focuser sử dụng 28BYJ và drive UNL2003 hoặc 2 loại drive tương tự bên trên. Tập trung vào camera usb, còn camera android phát triển sau
    + Hỗ trợ ALT-AZ và EQ nếu có thể ko thì chỉ dùng ALT-AZ
> Phần mềm của kính thiên văn điện tử : ASDEVLAB
    + Giao tiếp giữa firmware và software
    + Cho phép Autofocus (sử dụng thuật toán tối ưu cho sao, các vật thể bề mặt, và scenery) và Manualfocus. 
    + Tối ưu LiveStaking và xử lý ảnh tốt hơn nếu có thể.
    + Có catalog thiên thể riêng tự tổng hợp.
2. Đối tượng sử dụng: Người mới và người chơi bán chuyên
3. Asdevlab chỉ hỗ trợ duy nhất onstepx
4. Open source một phần và có plugin thương mại một phần (chưa biêt nên chọn thương mại gì)
5. Triết lý của dự án: Gọn, có khả năng mở rộng trung bình, dễ bảo trì.
6. Camera như mô tả bên trên hỗ trợ USB UVC còn Camera điện thoại android phát triển sau
7. Hỗ trợ; Linux (Android, Window phát triển sau)
8. Giao diện web UI
9. Tích hợp mã nguôn trực tiếp
10. Catalog đang phát triển, dùng libnova để tính toán tọa độ,..
11. Plate Solver dùng như của LiveStacker (ASTAP có G05), giúp alignment dễ dàng
12. Nhiều chức năng trong onstepx ko cần thiết, asdevlab chỉ hỗ trợ 1 phần lớn chức năng ví dụ như ko cần đo nhiệt độ, thời tiết, thời gian địa điểm asdevlab lấy từ máy ngườ dùng... và phát triển thêm các lệnh lx200 như flipmirror, mountmode
13. Qua mỗi bước phải có audit báo cáo thường xuyên, Unit test, refatoring định kì, trong code phải có comment. Điều ko muốn xuất hiện trong dự án: spaghetti code, nợ kỹ thuật, cấu trúc cây ngày càng phức tạp chia nhỏ ra quá mức,..

## Các từ khóa mới

- API (viết tắt của Application Programming Interface, Giao diện lập trình ứng dụng) là cầu nối trung gian cho phép các phần mềm, ứng dụng hoặc hệ thống khác nhau "giao tiếp" và trao đổi dữ liệu với nhau.
    + Khi bạn sử dụng một ứng dụng trên điện thoại (ví dụ: đặt xe trên ứng dụng gọi xe, tra cứu thời tiết), ứng dụng sẽ kết nối với Internet và gửi yêu cầu tới máy chủ. Máy chủ sẽ xử lý, lấy dữ liệu và gửi trả kết quả về điện thoại của bạn thông qua API.
- Báo cáo audit (kiểm toán/đánh giá) là tài liệu chính thức tổng hợp kết quả kiểm tra, đối chiếu dữ liệu của một hệ thống, quy trình hoặc tổ chức so với các tiêu chuẩn quy định. Mục đích là chỉ ra lỗi sai, rủi ro và đề xuất giải pháp khắc phục.
- ASCOM driver: lớp trung gian giao tiếp giữa phần mềm điều khiển khác với onstepx, nhưng vì là driver của wwindows nên ko sử dụng => asdevlab lựa chọn gửi lệnh LX200 thô giống triton goto. Như vậy Asdevlab sẽ giao tiếp trực tiếp với OnStepX bằng cách gửi các lệnh thô LX200 thông qua kết nối Wi-F imà không cần "bộ dịch" như ASCOM ở giữa
- Schema có nghĩa là lược đồ, được hiểu là một bản thiết kế hoặc cấu trúc logic dùng để tổ chức, lưu trữ và diễn giải thông tin. Dùng trong catalog thiên thể

## Mô hình chạy web

[ Người dùng ] ──(Chỉ mở 1 Web duy nhất: http://localhost:8080)──> [ OpenLiveStacker (Laptop Linux) ]
                                                                             │
               ┌─────────────────────────────────────────────────────────────┤
               ▼ (Xử lý tại chỗ)                                             ▼ (Bắn lệnh qua Wifi/Serial)
       [ Live Stacking ]                                              [ Gửi lệnh LX200 / INDI ]
   (Nhận ảnh từ Camera UVC)                                                  │
                                                                             ▼
                                                                  [ ESP32 (Firmware OnStepX) ]
                                                                       (Quay Motor Bước)

                                                                    
## Open Live Stacker


### OnstepX
> Các file Config.h và Extended.config.h đều là file cấu hình ko có thuật toán, chúng định nghĩa:
    + Driver
    + Gear ratio
    + Motor
    + Pin
    + Wifi/Bluetooth
    + Tracking
    + Limits
> Telescope/ chia làm 6 module:
    + Tracking: Tính tốc độ quay của trục RA
    + GÔTO: Tính toán để di chuyển
    + Coordinates: Quản lý tọa độ
    + LX200: Giao thức lệnh thứ được gử i đi tới Onstepx từ phần mềm
    + Mount State: Idle (chờ lệnh), Tracking, Slewing, Parking, Homing, Guiding
    + Focuser: Position, Stop
> Motor
    + Là lớp điều khiển động cơ (Ko sửa)
> HAL
    + HAL là lớp nằm giữa phần mềm và phần cứng, giúp chương trình không phải biết chi tiết cách điều khiển từng loại vi điều khiển hay từng chân GPIO




### LiveStacker

## Cấu trúc thu mục của OnstepX
Các nhánh chính của dự án:
- `Config.h`: định nghĩa cấu hình chính cho firmware.
- `Extended.config.h`: cấu hình mở rộng bổ sung cho các tính năng nâng cao.
- `OnStepX.ino`: điểm vào sketch Arduino/ESP, khởi tạo và chạy firmware.
- `README.md`: tài liệu tổng quan về dự án.
- `LICENSE`: giấy phép sử dụng.
- `docs/`: ghi chú và hướng dẫn vận hành, lệnh, cảm biến, CAN, homing...
- `src/`: mã nguồn chính của OnStepX.
  - `HAL/`: tầng trừu tượng phần cứng cho nhiều loại board (ESP, STM32, Teensy, Atmel...).
  - `lib/`: thư viện chức năng chung (axis, motor, encoder, mạng, cảm biến, v.v.).
  - `libApp/`: module ứng dụng cụ thể (lệnh, nhiệt độ, thời tiết).
  - `pinmaps/`: định nghĩa chân cho các bo mạch và kiểm tra pinmap.
  - `plugins/`: hệ thống plugin và ví dụ giao diện website.
  - `telescope/`: chức năng điều khiển kính thiên văn, mount, focuser, rotator.
  - `Validate.h`: kiểm tra hợp lệ cấu hình và định nghĩa.

OnStepX
├── Config.h
├── docs                 
│   ├── CAN_NOTES.md
│   ├── COMMAND_REFERENCE.md       
│   ├── GEM LIMITS EXAMPLE EAST OF PIER.jpg
│   ├── GEM LIMITS EXAMPLE KEEP OUT ZONE.jpg
│   ├── GEM LIMITS EXAMPLE WEST OF PIER.jpg
│   ├── GOTO_NOTES.md          
│   ├── HOMING_NOTES.md
│   ├── PEC_NOTES.md
│   ├── SENSING_NOTES.md
│   ├── SERIAL_NOTES.md   
│   ├── SERVO_SETUP.md
│   └── STARTUP_AUTHORITY_NOTES.md
├── Extended.config.h          
├── LICENSE                   
├── OnStepX.ino                           
├── README.md                              
└── src
    ├── Common.h                         
    ├── Config.defaults.h
    ├── Constants.h
    ├── HAL                        
    │   ├── arduinoM0
    │   │   └── ArduinoM0.h
    │   ├── atmel
    │   │   ├── Mega2560.h
    │   │   └── Mega328.h
    │   ├── default
    │   │   └── Default.h
    │   ├── esp                         //
    │   │   ├── ESP32Libraries2.h
    │   │   ├── ESP32Libraries3.h
    │   │   ├── ESP32UnoR4WiFi.h
    │   │   └── ESP8266.h
    │   ├── HAL_ANALOG.h                //
    │   ├── HAL.cpp                     //
    │   ├── HAL_FAST_TICKS.h            //
    │   ├── HAL.h                       //
    │   ├── mbed
    │   │   ├── Rpi2040.h
    │   │   └── Rpi2350.h
    │   ├── stm32
    │   │   ├── STM32F103.h
    │   │   ├── STM32F303.h
    │   │   ├── STM32F407.h
    │   │   ├── STM32F446.h
    │   │   ├── STM32F4x1.h
    │   │   └── STM32H7xx.h
    │   └── teensy
    │       ├── Teensy3.2.h
    │       ├── Teensy3.5.h
    │       ├── Teensy3.6.h
    │       ├── Teensy4.0.h
    │       └── Teensy4.1.h
    ├── lib
    │   ├── 1wire                       // Liên quan đến đo nhiệt độ OTA hoặc bù focus
    │   │   ├── 1Wire.cpp
    │   │   └── 1Wire.h
    │   ├── analog                      // Quản lý ADC: joystick, điện áp nguồn, pin,..
    │   │   ├── Analog.cpp
    │   │   └── Analog.h
    │   ├── axis                        // Gồm 2 trục
    │   │   ├── Axis.command.cpp        // Lệnh trục
    │   │   ├── Axis.cpp                // Quản lý trạng thái: position, target, speed, acceleration, atate
    │   │   ├── Axis.h                  // 
    │   │   └── motor                   // 
    │   │       ├── Drivers.h
    │   │       ├── kTech
    │   │       │   ├── KTech.cpp
    │   │       │   └── KTech.h
    │   │       ├── mksServo
    │   │       │   ├── MksServo.cpp
    │   │       │   └── MksServo.h
    │   │       ├── Motor.cpp           // Lớp cơ sở: move(), stop(),..
    │   │       ├── Motor.h
    │   │       ├── oDrive
    │   │       │   ├── ODriveCanPlus.cpp
    │   │       │   ├── ODriveCanPlus.h
    │   │       │   ├── ODrive.cpp
    │   │       │   ├── ODrive.h
    │   │       │   ├── ODriveNew.cpp
    │   │       │   └── ODriveNew.h
    │   │       ├── servo
    │   │       │   ├── dc
    │   │       │   │   ├── calibration
    │   │       │   │   │   ├── TrackingVelocity.cpp
    │   │       │   │   │   └── TrackingVelocity.h
    │   │       │   │   ├── DcServoDriver.cpp
    │   │       │   │   ├── DcServoDriver.h
    │   │       │   │   ├── eE
    │   │       │   │   │   ├── EE.cpp
    │   │       │   │   │   └── EE.h
    │   │       │   │   ├── pE
    │   │       │   │   │   ├── PE.cpp
    │   │       │   │   │   └── PE.h
    │   │       │   │   └── tmc
    │   │       │   │       ├── tmc2130
    │   │       │   │       │   ├── Tmc2130.cpp
    │   │       │   │       │   └── Tmc2130.h
    │   │       │   │       └── tmc5160
    │   │       │   │           ├── Tmc5160.cpp
    │   │       │   │           └── Tmc5160.h
    │   │       │   ├── feedback
    │   │       │   │   ├── DualPid
    │   │       │   │   │   ├── DualPid.cpp
    │   │       │   │   │   └── DualPid.h
    │   │       │   │   ├── FeedbackBase.cpp
    │   │       │   │   ├── FeedbackBase.h
    │   │       │   │   ├── Feedback.h
    │   │       │   │   └── Pid
    │   │       │   │       ├── Pid.cpp
    │   │       │   │       └── Pid.h
    │   │       │   ├── filter
    │   │       │   │   ├── FilterBase.cpp
    │   │       │   │   ├── FilterBase.h
    │   │       │   │   ├── Filter.h
    │   │       │   │   ├── Kalman
    │   │       │   │   │   ├── Kalman.cpp
    │   │       │   │   │   └── Kalman.h
    │   │       │   │   ├── Learning
    │   │       │   │   │   ├── Learning.cpp
    │   │       │   │   │   └── Learning.h
    │   │       │   │   └── Rolling
    │   │       │   │       ├── Rolling.cpp
    │   │       │   │       └── Rolling.h
    │   │       │   ├── kTech
    │   │       │   │   ├── KTech.cpp
    │   │       │   │   └── KTech.h
    │   │       │   ├── Servo.cpp
    │   │       │   ├── ServoDriver.cpp
    │   │       │   ├── ServoDriver.h
    │   │       │   ├── Servo.h
    │   │       │   └── tmc
    │   │       │       ├── tmc2209
    │   │       │       │   ├── Tmc2209.cpp
    │   │       │       │   └── Tmc2209.h
    │   │       │       ├── tmc5160
    │   │       │       │   ├── Tmc5160.cpp
    │   │       │       │   └── Tmc5160.h
    │   │       │       ├── TmcServoDriver.cpp
    │   │       │       └── TmcServoDriver.h
    │   │       └── stepDir
    │   │           ├── generic
    │   │           │   ├── Generic.cpp
    │   │           │   └── Generic.h
    │   │           ├── StepDir.cpp
    │   │           ├── StepDirDriver.cpp
    │   │           ├── StepDirDriver.h
    │   │           ├── StepDir.h
    │   │           └── tmc
    │   │               ├── legacy
    │   │               │   ├── tmc2130
    │   │               │   │   ├── Tmc2130.cpp
    │   │               │   │   └── Tmc2130.h
    │   │               │   ├── tmc2209
    │   │               │   │   ├── Tmc2209.cpp
    │   │               │   │   └── Tmc2209.h
    │   │               │   ├── tmc5160
    │   │               │   │   ├── Tmc5160.cpp
    │   │               │   │   └── Tmc5160.h
    │   │               │   ├── TmcSPI.cpp
    │   │               │   └── TmcSPI.h
    │   │               ├── TmcStepDirDriver.cpp
    │   │               ├── TmcStepDirDriver.h
    │   │               ├── TmcStepDirDriverNSG.cpp
    │   │               ├── TmcStepDirDriverNSG.h
    │   │               ├── TmcStepDirDriverSG.cpp
    │   │               ├── TmcStepDirDriverSG.h
    │   │               └── tmcStepper
    │   │                   ├── tmc2130
    │   │                   │   ├── Tmc2130.cpp
    │   │                   │   └── Tmc2130.h
    │   │                   ├── tmc2160
    │   │                   │   ├── Tmc2160.cpp
    │   │                   │   └── Tmc2160.h
    │   │                   ├── tmc2208
    │   │                   │   ├── tmc2208.cpp
    │   │                   │   └── tmc2208.h
    │   │                   ├── tmc2209
    │   │                   │   ├── Tmc2209.cpp
    │   │                   │   └── Tmc2209.h
    │   │                   ├── tmc2660
    │   │                   │   ├── Tmc2660.cpp
    │   │                   │   └── Tmc2660.h
    │   │                   ├── tmc5160
    │   │                   │   ├── Tmc5160.cpp
    │   │                   │   └── Tmc5160.h
    │   │                   └── tmc5161
    │   │                       ├── Tmc5161.cpp
    │   │                       └── Tmc5161.h
    │   ├── bluetooth
    │   │   ├── Bluetooth.defaults.h
    │   │   ├── BluetoothManager.cpp
    │   │   └── BluetoothManager.h
    │   ├── calendars
    │   │   ├── Calendars.cpp
    │   │   └── Calendars.h
    │   ├── canPlus
    │   │   ├── CanPlusBase.cpp
    │   │   ├── CanPlusBase.h
    │   │   ├── CanPlus.h
    │   │   ├── esp32
    │   │   │   ├── Esp32.cpp
    │   │   │   └── Esp32.h
    │   │   ├── mcp2515
    │   │   │   ├── Mcp2515.cpp
    │   │   │   └── Mcp2515.h
    │   │   ├── san
    │   │   │   ├── San.cpp
    │   │   │   └── San.h
    │   │   └── teensy4
    │   │       ├── Can0.cpp
    │   │       ├── Can0.h
    │   │       ├── Can1.cpp
    │   │       ├── Can1.h
    │   │       ├── Can2.cpp
    │   │       ├── Can2.h
    │   │       ├── Can3.cpp
    │   │       └── Can3.h
    │   ├── canTransport
    │   │   ├── CanTransportBase.cpp
    │   │   ├── CanTransportBase.h
    │   │   ├── CanTransportClient.cpp
    │   │   ├── CanTransportClient.h
    │   │   ├── CanTransportServer.cpp
    │   │   └── CanTransportServer.h
    │   ├── commands
    │   │   ├── BufferCmds.cpp
    │   │   ├── BufferCmds.h
    │   │   ├── CommandErrors.h
    │   │   ├── commands.ino
    │   │   ├── SerialWrapper.cpp
    │   │   └── SerialWrapper.h
    │   ├── Constants.h
    │   ├── convert
    │   │   ├── Convert.cpp
    │   │   └── Convert.h
    │   ├── debug
    │   │   ├── Debug.cpp
    │   │   └── Debug.h
    │   ├── encoder
    │   │   ├── bissc
    │   │   │   ├── As37h39bb.cpp
    │   │   │   ├── As37h39bb.h
    │   │   │   ├── Asc85.cpp
    │   │   │   ├── Asc85.h
    │   │   │   ├── Bissc.cpp
    │   │   │   ├── Bissc.h
    │   │   │   ├── Jtw24.cpp
    │   │   │   ├── Jtw24.h
    │   │   │   ├── Jtw26.cpp
    │   │   │   └── Jtw26.h
    │   │   ├── cwCcw
    │   │   │   ├── CwCcw.cpp
    │   │   │   └── CwCcw.h
    │   │   ├── EncoderBase.cpp
    │   │   ├── EncoderBase.h
    │   │   ├── Encoder.h
    │   │   ├── ktech
    │   │   │   ├── KTech.cpp
    │   │   │   └── KTech.h
    │   │   ├── pulseDir
    │   │   │   ├── PulseDir.cpp
    │   │   │   └── PulseDir.h
    │   │   ├── pulseOnly
    │   │   │   ├── PulseOnly.cpp
    │   │   │   └── PulseOnly.h
    │   │   ├── quadrature
    │   │   │   ├── Quadrature.cpp
    │   │   │   └── Quadrature.h
    │   │   ├── quadratureEsp32
    │   │   │   ├── QuadratureEsp32.cpp
    │   │   │   └── QuadratureEsp32.h
    │   │   ├── serialBridge
    │   │   │   ├── SerialBridge.cpp
    │   │   │   └── SerialBridge.h
    │   │   └── virtualEnc
    │   │       ├── VirtualEnc.cpp
    │   │       └── VirtualEnc.h
    │   ├── ethernet
    │   │   ├── cmdServer
    │   │   │   ├── CmdServer.cpp
    │   │   │   └── CmdServer.h
    │   │   ├── EthernetManager.cpp
    │   │   ├── EthernetManager.defaults.h
    │   │   ├── EthernetManager.h
    │   │   └── webServer
    │   │       ├── WebServer.cpp
    │   │       └── WebServer.h
    │   ├── gpioEx
    │   │   ├── ds2413
    │   │   │   ├── Ds2413.cpp
    │   │   │   └── Ds2413.h
    │   │   ├── GpioBase.cpp
    │   │   ├── GpioBase.h
    │   │   ├── GpioEx.h
    │   │   ├── mcp23008
    │   │   │   ├── Mcp23008.cpp
    │   │   │   └── Mcp23008.h
    │   │   ├── mcp23017
    │   │   │   ├── Mcp23017.cpp
    │   │   │   └── Mcp23017.h
    │   │   ├── pcf8574
    │   │   │   ├── Pcf8574.cpp
    │   │   │   └── Pcf8574.h
    │   │   ├── pcf8575
    │   │   │   ├── Pcf8575.cpp
    │   │   │   └── Pcf8575.h
    │   │   ├── ssr74HC595
    │   │   │   ├── Ssr74HC595.cpp
    │   │   │   └── Ssr74HC595.h
    │   │   ├── sws
    │   │   │   ├── Sws.cpp
    │   │   │   └── Sws.h
    │   │   └── tca9555
    │   │       ├── Tca9555.cpp
    │   │       └── Tca9555.h
    │   ├── Macros.h
    │   ├── math
    │   │   ├── Crc.cpp
    │   │   └── Crc.h
    │   ├── nv
    │   │   ├── device
    │   │   │   ├── 24xxI2C.h
    │   │   │   ├── DeviceNull.h
    │   │   │   ├── EepromArduino.h
    │   │   │   ├── EepromEmuEsp.h
    │   │   │   ├── EepromEmuM0.h
    │   │   │   ├── Mb85rcI2C.h
    │   │   │   ├── NvDeviceBase.h
    │   │   │   ├── ShimCached.h
    │   │   │   └── ShimDelayedCommit.h
    │   │   ├── NvConfig.h
    │   │   ├── Nv.cpp
    │   │   ├── Nv.h
    │   │   ├── NvIvPartition.cpp
    │   │   ├── NvIvPartition.h
    │   │   ├── NvKvPartition16.cpp
    │   │   ├── NvKvPartition16.h
    │   │   ├── NvKvPartition32.cpp
    │   │   ├── NvKvPartition32.h
    │   │   ├── NvVolume.cpp
    │   │   └── NvVolume.h
    │   ├── pushButton
    │   │   ├── PushButton.cpp
    │   │   └── PushButton.h
    │   ├── sense
    │   │   ├── Sense.cpp
    │   │   └── Sense.h
    │   ├── serial
    │   │   ├── Serial_IP_Ethernet_Client.cpp
    │   │   ├── Serial_IP_Ethernet_Client.h
    │   │   ├── Serial_IP_Ethernet.cpp
    │   │   ├── Serial_IP_Ethernet.h
    │   │   ├── Serial_IP_Wifi_Client.cpp
    │   │   ├── Serial_IP_Wifi_Client.h
    │   │   ├── Serial_IP_Wifi.cpp
    │   │   ├── Serial_IP_Wifi.h
    │   │   ├── Serial_Local.cpp
    │   │   ├── Serial_Local.h
    │   │   ├── Serial_MEGA2560.cpp
    │   │   ├── Serial_MEGA2560.h
    │   │   ├── Serial_ST4_Master.cpp
    │   │   ├── Serial_ST4_Master.h
    │   │   ├── Serial_ST4_Slave.cpp
    │   │   └── Serial_ST4_Slave.h
    │   ├── softSpi
    │   │   ├── Pins.h
    │   │   ├── SoftSpi.cpp
    │   │   └── SoftSpi.h
    │   ├── sound
    │   │   ├── Sound.cpp
    │   │   └── Sound.h
    │   ├── tasks
    │   │   ├── HAL_ATMEGA328_HWTIMER.h
    │   │   ├── HAL_EMPTY_HWTIMER.h
    │   │   ├── HAL_ESP32_HWTIMER.h
    │   │   ├── HAL_ESP32_V3_HWTIMER.h
    │   │   ├── HAL_HWTIMERS.h
    │   │   ├── HAL_MEGA2560_HWTIMER.h
    │   │   ├── HAL_PROFILER.h
    │   │   ├── HAL_STM32_HWTIMER.h
    │   │   ├── HAL_TEENSY_HWTIMER.h
    │   │   ├── OnTask.cpp
    │   │   ├── OnTaskExample.ino.txt
    │   │   └── OnTask.h
    │   ├── tls
    │   │   ├── ds3231
    │   │   │   ├── DS3231.cpp
    │   │   │   └── DS3231.h
    │   │   ├── ds3234
    │   │   │   ├── DS3234.cpp
    │   │   │   └── DS3234.h
    │   │   ├── gps
    │   │   │   ├── GPS.cpp
    │   │   │   └── GPS.h
    │   │   ├── ntp
    │   │   │   ├── NTP.cpp
    │   │   │   └── NTP.h
    │   │   ├── PPS.cpp
    │   │   ├── PPS.h
    │   │   ├── sd3031
    │   │   │   ├── SD3031.cpp
    │   │   │   └── SD3031.h
    │   │   ├── teensy
    │   │   │   ├── Teensy.cpp
    │   │   │   └── Teensy.h
    │   │   ├── TlsBase.cpp
    │   │   ├── TlsBase.h
    │   │   └── Tls.h
    │   ├── watchdog
    │   │   ├── Watchdog.cpp
    │   │   └── Watchdog.h
    │   └── wifi
    │       ├── cmdServer
    │       │   ├── CmdServer.cpp
    │       │   └── CmdServer.h
    │       ├── webServer
    │       │   ├── WebServer.cpp
    │       │   └── WebServer.h
    │       ├── WifiManager.cpp
    │       ├── WifiManager.defaults.h
    │       └── WifiManager.h
    ├── libApp
    │   ├── commands
    │   │   ├── CommandBroker.cpp
    │   │   ├── CommandBroker.h
    │   │   ├── ProcessCmds.cpp
    │   │   └── ProcessCmds.h
    │   ├── temperature
    │   │   ├── Ds1820.cpp
    │   │   ├── Ds1820.h
    │   │   ├── Temperature.cpp
    │   │   ├── Temperature.h
    │   │   ├── Thermistor.cpp
    │   │   └── Thermistor.h
    │   └── weather
    │       ├── Weather.cpp
    │       └── Weather.h
    ├── pinmaps
    │   ├── Models.h
    │   ├── Pins.CNC3.h
    │   ├── Pins.defaults.h
    │   ├── Pins.FYSETC_E4.h
    │   ├── Pins.FYSETC_S6.h
    │   ├── Pins.JTWPCB2.h
    │   ├── Pins.Manticore.h
    │   ├── Pins.MaxESP3.h
    │   ├── Pins.MaxESP4.h
    │   ├── Pins.MaxPCB36.h
    │   ├── Pins.MaxPCB3.h
    │   ├── Pins.MaxPCB4.h
    │   ├── Pins.MaxPCB.h
    │   ├── Pins.MaxSTM.h
    │   ├── Pins.MicroScope.h
    │   ├── Pins.MiniPCB.h
    │   ├── Pins.OctopusPro.h
    │   ├── Pins.SAL_XB1.h
    │   ├── Pins.SKR_PRO.h
    │   ├── Pins.STM32B.h
    │   └── Validate.h
    ├── plugins
    │   ├── Plugins.config.h
    │   ├── sample
    │   │   ├── Sample.cpp
    │   │   └── Sample.h
    │   └── website
    │       ├── Common.h
    │       ├── Config.h
    │       ├── Constants.h
    │       ├── libApp
    │       │   ├── cmd
    │       │   │   ├── Cmd.cpp
    │       │   │   └── Cmd.h
    │       │   ├── misc
    │       │   │   ├── Misc.cpp
    │       │   │   └── Misc.h
    │       │   └── status
    │       │       ├── StateAuxiliary.cpp
    │       │       ├── StateController.cpp
    │       │       ├── State.cpp
    │       │       ├── StateFocuser.cpp
    │       │       ├── State.h
    │       │       ├── StateMount.cpp
    │       │       ├── StateRotator.cpp
    │       │       ├── Status.cpp
    │       │       ├── Status.h
    │       │       ├── Version.cpp
    │       │       └── Version.h
    │       ├── locales
    │       │   ├── Locale.h
    │       │   ├── Locales.h
    │       │   ├── Strings_ca.h
    │       │   ├── Strings_cn.h
    │       │   ├── Strings_de.h
    │       │   ├── Strings_en.h
    │       │   ├── Strings_es.h
    │       │   ├── Strings_fr.h
    │       │   ├── Strings_it.h
    │       │   └── Strings_jp.h
    │       ├── pages
    │       │   ├── auxiliary
    │       │   │   ├── Auxiliary.cpp
    │       │   │   └── Auxiliary.h
    │       │   ├── Err404.cpp
    │       │   ├── focuser
    │       │   │   ├── BacklashTcfTile.cpp
    │       │   │   ├── BacklashTcfTile.h
    │       │   │   ├── Focuser.cpp
    │       │   │   ├── Focuser.h
    │       │   │   ├── HomeTile.cpp
    │       │   │   ├── HomeTile.h
    │       │   │   ├── SelectTile.cpp
    │       │   │   ├── SelectTile.h
    │       │   │   ├── SlewingTile.cpp
    │       │   │   └── SlewingTile.h
    │       │   ├── htmlHeaders.h
    │       │   ├── htmlMessages.h
    │       │   ├── htmlScripts.h
    │       │   ├── index
    │       │   │   ├── AmbientTile.cpp
    │       │   │   ├── AmbientTile.h
    │       │   │   ├── AxisTile.cpp
    │       │   │   ├── AxisTile.h
    │       │   │   ├── Index.cpp
    │       │   │   ├── Index.h
    │       │   │   ├── ServoTile.cpp
    │       │   │   ├── ServoTile.h
    │       │   │   ├── StatusTile.cpp
    │       │   │   └── StatusTile.h
    │       │   ├── KeyValue.cpp
    │       │   ├── KeyValue.h
    │       │   ├── LibraryHelp.cpp
    │       │   ├── LibraryHelp.h
    │       │   ├── mount
    │       │   │   ├── AlignTile.cpp
    │       │   │   ├── AlignTile.h
    │       │   │   ├── CoordinateTile.cpp
    │       │   │   ├── CoordinateTile.h
    │       │   │   ├── GotoTile.cpp
    │       │   │   ├── GotoTile.h
    │       │   │   ├── GuideTile.cpp
    │       │   │   ├── GuideTile.h
    │       │   │   ├── HomeParkTile.cpp
    │       │   │   ├── HomeParkTile.h
    │       │   │   ├── LibraryTile.cpp
    │       │   │   ├── LibraryTile.h
    │       │   │   ├── LimitsTile.cpp
    │       │   │   ├── LimitsTile.h
    │       │   │   ├── Mount.cpp
    │       │   │   ├── Mount.h
    │       │   │   ├── PecTile.cpp
    │       │   │   ├── PecTile.h
    │       │   │   ├── SiteTile.cpp
    │       │   │   ├── SiteTile.h
    │       │   │   ├── TrackingTile.cpp
    │       │   │   └── TrackingTile.h
    │       │   ├── network
    │       │   │   ├── Network.cpp
    │       │   │   └── Network.h
    │       │   ├── Page.cpp
    │       │   ├── Page.h
    │       │   ├── Pages.common.h
    │       │   ├── Pages.h
    │       │   └── rotator
    │       │       ├── BacklashTile.cpp
    │       │       ├── BacklashTile.h
    │       │       ├── DeRotatorTile.cpp
    │       │       ├── DeRotatorTile.h
    │       │       ├── HomeTile.cpp
    │       │       ├── HomeTile.h
    │       │       ├── Rotator.cpp
    │       │       ├── Rotator.h
    │       │       ├── SlewingTile.cpp
    │       │       └── SlewingTile.h
    │       ├── Website.cpp
    │       └── Website.h
    ├── telescope
    │   ├── addonFlasher
    │   │   ├── AddonFlasher.cpp
    │   │   └── AddonFlasher.h
    │   ├── auxiliary
    │   │   ├── dewHeater
    │   │   │   ├── DewHeater.cpp
    │   │   │   └── DewHeater.h
    │   │   ├── FeaturesBase.h
    │   │   ├── Features.h
    │   │   ├── intervalometer
    │   │   │   ├── Intervalometer.cpp
    │   │   │   └── Intervalometer.h
    │   │   ├── local
    │   │   │   ├── Features.can.command.cpp
    │   │   │   ├── Features.command.cpp
    │   │   │   ├── Features.cpp
    │   │   │   └── Features.h
    │   │   ├── powerMonitor
    │   │   │   ├── PowerMonitor.cpp
    │   │   │   └── PowerMonitor.h
    │   │   └── remote
    │   │       ├── Features.can.decode.cpp
    │   │       ├── Features.can.encode.cpp
    │   │       ├── Features.cpp
    │   │       └── Features.h
    │   ├── focuser
    │   │   ├── FocuserBase.h
    │   │   ├── Focuser.h
    │   │   ├── local
    │   │   │   ├── Focuser.axis4.cpp
    │   │   │   ├── Focuser.axis5.cpp
    │   │   │   ├── Focuser.axis6.cpp
    │   │   │   ├── Focuser.axis7.cpp
    │   │   │   ├── Focuser.axis8.cpp
    │   │   │   ├── Focuser.axis9.cpp
    │   │   │   ├── Focuser.axis.prototype.cpp
    │   │   │   ├── Focuser.can.command.cpp
    │   │   │   ├── Focuser.command.cpp
    │   │   │   ├── Focuser.cpp
    │   │   │   └── Focuser.h
    │   │   └── remote
    │   │       ├── Focuser.can.decode.cpp
    │   │       ├── Focuser.can.encode.cpp
    │   │       ├── Focuser.cpp
    │   │       └── Focuser.h
    │   ├── mount
    │   │   ├── coordinates
    │   │   │   ├── Align.hs.cpp
    │   │   │   ├── Align.hs.h
    │   │   │   ├── Align.ref.cpp
    │   │   │   ├── Align.ref.h
    │   │   │   ├── coordinates.ino
    │   │   │   ├── Transform.cpp
    │   │   │   └── Transform.h
    │   │   ├── goto
    │   │   │   ├── Goto.command.cpp
    │   │   │   ├── Goto.cpp
    │   │   │   └── Goto.h
    │   │   ├── guide
    │   │   │   ├── Guide.command.cpp
    │   │   │   ├── Guide.cpp
    │   │   │   └── Guide.h
    │   │   ├── home
    │   │   │   ├── Home.command.cpp
    │   │   │   ├── Home.cpp
    │   │   │   └── Home.h
    │   │   ├── library
    │   │   │   ├── Library.command.cpp
    │   │   │   ├── Library.cpp
    │   │   │   └── Library.h
    │   │   ├── limits
    │   │   │   ├── Limits.command.cpp
    │   │   │   ├── Limits.cpp
    │   │   │   └── Limits.h
    │   │   ├── Mount.axis.cpp
    │   │   ├── Mount.command.cpp
    │   │   ├── Mount.cpp
    │   │   ├── Mount.h
    │   │   ├── park
    │   │   │   ├── Park.command.cpp
    │   │   │   ├── Park.cpp
    │   │   │   └── Park.h
    │   │   ├── pec
    │   │   │   ├── Pec.command.cpp
    │   │   │   ├── Pec.cpp
    │   │   │   └── Pec.h
    │   │   ├── site
    │   │   │   ├── Site.command.cpp
    │   │   │   ├── Site.cpp
    │   │   │   └── Site.h
    │   │   ├── st4
    │   │   │   ├── St4.cpp
    │   │   │   └── St4.h
    │   │   └── status
    │   │       ├── Status.command.cpp
    │   │       ├── Status.cpp
    │   │       └── Status.h
    │   ├── rotator
    │   │   ├── local
    │   │   │   ├── Rotator.axis.cpp
    │   │   │   ├── Rotator.can.command.cpp
    │   │   │   ├── Rotator.command.cpp
    │   │   │   ├── Rotator.cpp
    │   │   │   └── Rotator.h
    │   │   ├── remote
    │   │   │   ├── Rotator.can.decode.cpp
    │   │   │   ├── Rotator.can.encode.cpp
    │   │   │   ├── Rotator.cpp
    │   │   │   └── Rotator.h
    │   │   ├── RotatorBase.h
    │   │   └── Rotator.h
    │   ├── Telescope.command.cpp
    │   ├── Telescope.cpp
    │   └── Telescope.h
    └── Validate.h

OpenLiveStaker
.
├── CMakeLists.txt
├── config.json
├── copyright.txt
├── deploy_win.py
├── docs
│   ├── conn-alpaca.png
│   ├── connection-charts.png
│   ├── processing.dot
│   ├── processing.png
│   ├── processing.svg
│   └── protcol.md
├── external
│   ├── OpenNGC
│   ├── vsop87-multilang
│   └── western_constellations_atlas_of_space
├── include
│   ├── allocator.h
│   ├── alpaca_client.h
│   ├── alpaca_focuser.h
│   ├── alpaca_mount.h
│   ├── astap_db_download_app.h
│   ├── astronomy_calc.h
│   ├── camera_ctl.h
│   ├── camera.h
│   ├── camera_iface.h
│   ├── common_data.h
│   ├── common_utils.h
│   ├── config_app.h
│   ├── ctl_app.h
│   ├── data_items.h
│   ├── downloader.h
│   ├── fits_guard.h
│   ├── fitsmat.h
│   ├── focuser_ctl.h
│   ├── focuser.h
│   ├── hot_removal.h
│   ├── httplib_for_ols.h
│   ├── indi_mount.h
│   ├── libraw_wrapper.h
│   ├── live_stretch.h
│   ├── mount_ctl.h
│   ├── mount.h
│   ├── ols.h
│   ├── os_util.h
│   ├── plate_solver_ctl_app.h
│   ├── plate_solver.h
│   ├── plate_solver_result.h
│   ├── polar_align.h
│   ├── post_processor.h
│   ├── processors.h
│   ├── rotation.h
│   ├── server_sent_events.h
│   ├── shift_bit.h
│   ├── simd_utils.h
│   ├── stacker_ctl_app.h
│   ├── stacker.h
│   ├── sw_bin.h
│   ├── sync_queue.h
│   ├── tiffmat.h
│   ├── util.h
│   ├── uvc_camera.h
│   ├── video_frame.h
│   ├── video_generator.h
│   └── video_stream.h
├── included_in_windows_distribution.txt
├── LICENSE
├── README.md
├── scripts
│   ├── icon.ico
│   ├── make_db.py
│   └── ols_gui
├── sim
│   ├── frame_00000000.jpeg
│   ├── frame_00000001.jpeg
│   ├── frame_00000002.jpeg
│   ├── frame_00000003.jpeg
│   ├── frame_00000004.jpeg
│   ├── frame_00000005.jpeg
│   ├── frame_00000006.jpeg
│   ├── frame_00000007.jpeg
│   ├── frame_00000008.jpeg
│   ├── frame_00000009.jpeg
│   ├── frame_00000010.jpeg
│   ├── frame_00000011.jpeg
│   ├── frame_00000012.jpeg
│   ├── frame_00000013.jpeg
│   ├── frame_00000014.jpeg
│   ├── frame_00000015.jpeg
│   ├── frame_00000016.jpeg
│   ├── frame_00000017.jpeg
│   ├── info.json
│   └── log.txt
├── src
│   ├── allocator.cpp
│   ├── alpaca_camera.cpp
│   ├── alpaca_client.cpp
│   ├── alpaca_focuser.cpp
│   ├── alpaca_mount.cpp
│   ├── android_camera.cpp
│   ├── android_main.cpp
│   ├── asi_camera.cpp
│   ├── camera.cpp
│   ├── common_utils.cpp
│   ├── debug_save_processor.cpp
│   ├── downloader.cpp
│   ├── fitsmat.cpp
│   ├── focuser.cpp
│   ├── focuser_ctl.cpp
│   ├── gphoto2_camera.cpp
│   ├── indi_camera.cpp
│   ├── indigo_camera.cpp
│   ├── indigo_camera.h
│   ├── indi_mount.cpp
│   ├── mount.cpp
│   ├── mount_ctl.cpp
│   ├── ols_cmd.cpp
│   ├── ols.cpp
│   ├── plate_solver.cpp
│   ├── post_processor.cpp
│   ├── pre_processor.cpp
│   ├── server_sent_events.cpp
│   ├── service.cpp
│   ├── sim_camera.cpp
│   ├── stacker_processor.cpp
│   ├── svb_camera.cpp
│   ├── tiffmat.cpp
│   ├── toup_camera.cpp
│   ├── toup_oem.h
│   ├── util.cpp
│   ├── uvc_camera.cpp
│   ├── video_generator.cpp
│   └── wdir_camera.cpp
├── test
│   ├── offline_sim.cpp
│   ├── plate_solver_test.cpp
│   ├── stream_camera.cpp
│   ├── test_bin2.py
│   ├── test_bin.cpp
│   ├── test_bin.py
│   ├── test_camera.cpp
│   ├── test_download.cpp
│   └── test_indigo_format_parsing.cpp
├── tools
│   ├── auto_stretch.py
│   ├── build_star_pattern.m
│   └── test_gamma.cpp
└── www-data
    ├── eyepiece.html
    ├── index.html
    └── media
        ├── css
        │   └── style.css
        ├── img
        │   ├── cancel.png
        │   ├── delete.png
        │   ├── E.png
        │   ├── error.png
        │   ├── facicon.png
        │   ├── fast.png
        │   ├── mount.png
        │   ├── no_sync.png
        │   ├── N.png
        │   ├── pause.png
        │   ├── resume.png
        │   ├── save.png
        │   ├── settings.png
        │   ├── sharp.png
        │   ├── slow.png
        │   ├── solve.png
        │   ├── S.png
        │   ├── stop.png
        │   ├── stretch.png
        │   └── W.png
        └── js
            └── code.js






OnStepX
├── Config.h
├── docs                 
│   ├── CAN_NOTES.md
│   ├── COMMAND_REFERENCE.md       
│   ├── GEM LIMITS EXAMPLE EAST OF PIER.jpg
│   ├── GEM LIMITS EXAMPLE KEEP OUT ZONE.jpg
│   ├── GEM LIMITS EXAMPLE WEST OF PIER.jpg
│   ├── GOTO_NOTES.md          
│   ├── HOMING_NOTES.md
│   ├── PEC_NOTES.md
│   ├── SENSING_NOTES.md
│   ├── SERIAL_NOTES.md   
│   ├── SERVO_SETUP.md
│   └── STARTUP_AUTHORITY_NOTES.md
├── Extended.config.h          
├── LICENSE                   
├── OnStepX.ino                           
├── README.md                              
└── src
    ├── Common.h                         
    ├── Config.defaults.h
    ├── Constants.h
    ├── HAL                             // 
    │   ├── esp                         //
    │   │   ├── ESP32Libraries2.h
    │   │   ├── ESP32Libraries3.h
    │   │   ├── ESP32UnoR4WiFi.h
    │   │   └── ESP8266.h
    │   ├── HAL_ANALOG.h                //
    │   ├── HAL.cpp                     //
    │   ├── HAL_FAST_TICKS.h            //
    │   ├── HAL.h                       //
    ├── lib
    │   ├── 1wire                       // Thường dùng để kiểm tra nhiệt độ
    │   │   ├── 1Wire.cpp
    │   │   └── 1Wire.h
    │   ├── analog
    │   │   ├── Analog.cpp
    │   │   └── Analog.h
    │   ├── axis                        //
    │   │   ├── Axis.command.cpp
    │   │   ├── Axis.cpp
    │   │   ├── Axis.h
    │   │   └── motor
    │   │       ├── Drivers.h
    │   │       ├── kTech
    │   │       │   ├── KTech.cpp
    │   │       │   └── KTech.h
    │   │       ├── mksServo
    │   │       │   ├── MksServo.cpp
    │   │       │   └── MksServo.h
    │   │       ├── Motor.cpp
    │   │       ├── Motor.h
    │   │       ├── oDrive
    │   │       │   ├── ODriveCanPlus.cpp
    │   │       │   ├── ODriveCanPlus.h
    │   │       │   ├── ODrive.cpp
    │   │       │   ├── ODrive.h
    │   │       │   ├── ODriveNew.cpp
    │   │       │   └── ODriveNew.h
    │   │       ├── servo
    │   │       │   ├── dc
    │   │       │   │   ├── calibration
    │   │       │   │   │   ├── TrackingVelocity.cpp
    │   │       │   │   │   └── TrackingVelocity.h
    │   │       │   │   ├── DcServoDriver.cpp
    │   │       │   │   ├── DcServoDriver.h
    │   │       │   │   ├── eE
    │   │       │   │   │   ├── EE.cpp
    │   │       │   │   │   └── EE.h
    │   │       │   │   ├── pE
    │   │       │   │   │   ├── PE.cpp
    │   │       │   │   │   └── PE.h
    │   │       │   │   └── tmc
    │   │       │   │       ├── tmc2130
    │   │       │   │       │   ├── Tmc2130.cpp
    │   │       │   │       │   └── Tmc2130.h
    │   │       │   │       └── tmc5160
    │   │       │   │           ├── Tmc5160.cpp
    │   │       │   │           └── Tmc5160.h
    │   │       │   ├── feedback
    │   │       │   │   ├── DualPid
    │   │       │   │   │   ├── DualPid.cpp
    │   │       │   │   │   └── DualPid.h
    │   │       │   │   ├── FeedbackBase.cpp
    │   │       │   │   ├── FeedbackBase.h
    │   │       │   │   ├── Feedback.h
    │   │       │   │   └── Pid
    │   │       │   │       ├── Pid.cpp
    │   │       │   │       └── Pid.h
    │   │       │   ├── filter
    │   │       │   │   ├── FilterBase.cpp
    │   │       │   │   ├── FilterBase.h
    │   │       │   │   ├── Filter.h
    │   │       │   │   ├── Kalman
    │   │       │   │   │   ├── Kalman.cpp
    │   │       │   │   │   └── Kalman.h
    │   │       │   │   ├── Learning
    │   │       │   │   │   ├── Learning.cpp
    │   │       │   │   │   └── Learning.h
    │   │       │   │   └── Rolling
    │   │       │   │       ├── Rolling.cpp
    │   │       │   │       └── Rolling.h
    │   │       │   ├── kTech
    │   │       │   │   ├── KTech.cpp
    │   │       │   │   └── KTech.h
    │   │       │   ├── Servo.cpp
    │   │       │   ├── ServoDriver.cpp
    │   │       │   ├── ServoDriver.h
    │   │       │   ├── Servo.h
    │   │       │   └── tmc
    │   │       │       ├── tmc2209
    │   │       │       │   ├── Tmc2209.cpp
    │   │       │       │   └── Tmc2209.h
    │   │       │       ├── tmc5160
    │   │       │       │   ├── Tmc5160.cpp
    │   │       │       │   └── Tmc5160.h
    │   │       │       ├── TmcServoDriver.cpp
    │   │       │       └── TmcServoDriver.h
    │   │       └── stepDir
    │   │           ├── generic
    │   │           │   ├── Generic.cpp
    │   │           │   └── Generic.h
    │   │           ├── StepDir.cpp
    │   │           ├── StepDirDriver.cpp
    │   │           ├── StepDirDriver.h
    │   │           ├── StepDir.h
    │   │           └── tmc
    │   │               ├── legacy
    │   │               │   ├── tmc2130
    │   │               │   │   ├── Tmc2130.cpp
    │   │               │   │   └── Tmc2130.h
    │   │               │   ├── tmc2209
    │   │               │   │   ├── Tmc2209.cpp
    │   │               │   │   └── Tmc2209.h
    │   │               │   ├── tmc5160
    │   │               │   │   ├── Tmc5160.cpp
    │   │               │   │   └── Tmc5160.h
    │   │               │   ├── TmcSPI.cpp
    │   │               │   └── TmcSPI.h
    │   │               ├── TmcStepDirDriver.cpp
    │   │               ├── TmcStepDirDriver.h
    │   │               ├── TmcStepDirDriverNSG.cpp
    │   │               ├── TmcStepDirDriverNSG.h
    │   │               ├── TmcStepDirDriverSG.cpp
    │   │               ├── TmcStepDirDriverSG.h
    │   │               └── tmcStepper
    │   │                   ├── tmc2130
    │   │                   │   ├── Tmc2130.cpp
    │   │                   │   └── Tmc2130.h
    │   │                   ├── tmc2160
    │   │                   │   ├── Tmc2160.cpp
    │   │                   │   └── Tmc2160.h
    │   │                   ├── tmc2208
    │   │                   │   ├── tmc2208.cpp
    │   │                   │   └── tmc2208.h
    │   │                   ├── tmc2209
    │   │                   │   ├── Tmc2209.cpp
    │   │                   │   └── Tmc2209.h
    │   │                   ├── tmc2660
    │   │                   │   ├── Tmc2660.cpp
    │   │                   │   └── Tmc2660.h
    │   │                   ├── tmc5160
    │   │                   │   ├── Tmc5160.cpp
    │   │                   │   └── Tmc5160.h
    │   │                   └── tmc5161
    │   │                       ├── Tmc5161.cpp
    │   │                       └── Tmc5161.h
    │   ├── bluetooth                               //
    │   │   ├── Bluetooth.defaults.h
    │   │   ├── BluetoothManager.cpp
    │   │   └── BluetoothManager.h
    │   ├── calendars
    │   │   ├── Calendars.cpp
    │   │   └── Calendars.h
    │   ├── canPlus
    │   │   ├── CanPlusBase.cpp
    │   │   ├── CanPlusBase.h
    │   │   ├── CanPlus.h
    │   │   ├── esp32
    │   │   │   ├── Esp32.cpp
    │   │   │   └── Esp32.h
    │   │   ├── mcp2515
    │   │   │   ├── Mcp2515.cpp
    │   │   │   └── Mcp2515.h
    │   │   ├── san
    │   │   │   ├── San.cpp
    │   │   │   └── San.h
    │   │   └── teensy4
    │   │       ├── Can0.cpp
    │   │       ├── Can0.h
    │   │       ├── Can1.cpp
    │   │       ├── Can1.h
    │   │       ├── Can2.cpp
    │   │       ├── Can2.h
    │   │       ├── Can3.cpp
    │   │       └── Can3.h
    │   ├── canTransport
    │   │   ├── CanTransportBase.cpp
    │   │   ├── CanTransportBase.h
    │   │   ├── CanTransportClient.cpp
    │   │   ├── CanTransportClient.h
    │   │   ├── CanTransportServer.cpp
    │   │   └── CanTransportServer.h
    │   ├── commands
    │   │   ├── BufferCmds.cpp
    │   │   ├── BufferCmds.h
    │   │   ├── CommandErrors.h
    │   │   ├── commands.ino
    │   │   ├── SerialWrapper.cpp
    │   │   └── SerialWrapper.h
    │   ├── Constants.h
    │   ├── convert
    │   │   ├── Convert.cpp
    │   │   └── Convert.h
    │   ├── debug
    │   │   ├── Debug.cpp
    │   │   └── Debug.h
    │   ├── encoder
    │   │   ├── bissc
    │   │   │   ├── As37h39bb.cpp
    │   │   │   ├── As37h39bb.h
    │   │   │   ├── Asc85.cpp
    │   │   │   ├── Asc85.h
    │   │   │   ├── Bissc.cpp
    │   │   │   ├── Bissc.h
    │   │   │   ├── Jtw24.cpp
    │   │   │   ├── Jtw24.h
    │   │   │   ├── Jtw26.cpp
    │   │   │   └── Jtw26.h
    │   │   ├── cwCcw
    │   │   │   ├── CwCcw.cpp
    │   │   │   └── CwCcw.h
    │   │   ├── EncoderBase.cpp
    │   │   ├── EncoderBase.h
    │   │   ├── Encoder.h
    │   │   ├── ktech
    │   │   │   ├── KTech.cpp
    │   │   │   └── KTech.h
    │   │   ├── pulseDir
    │   │   │   ├── PulseDir.cpp
    │   │   │   └── PulseDir.h
    │   │   ├── pulseOnly
    │   │   │   ├── PulseOnly.cpp
    │   │   │   └── PulseOnly.h
    │   │   ├── quadrature
    │   │   │   ├── Quadrature.cpp
    │   │   │   └── Quadrature.h
    │   │   ├── quadratureEsp32
    │   │   │   ├── QuadratureEsp32.cpp
    │   │   │   └── QuadratureEsp32.h
    │   │   ├── serialBridge
    │   │   │   ├── SerialBridge.cpp
    │   │   │   └── SerialBridge.h
    │   │   └── virtualEnc
    │   │       ├── VirtualEnc.cpp
    │   │       └── VirtualEnc.h
    │   ├── ethernet
    │   │   ├── cmdServer
    │   │   │   ├── CmdServer.cpp
    │   │   │   └── CmdServer.h
    │   │   ├── EthernetManager.cpp
    │   │   ├── EthernetManager.defaults.h
    │   │   ├── EthernetManager.h
    │   │   └── webServer
    │   │       ├── WebServer.cpp
    │   │       └── WebServer.h
    │   ├── gpioEx
    │   │   ├── ds2413
    │   │   │   ├── Ds2413.cpp
    │   │   │   └── Ds2413.h
    │   │   ├── GpioBase.cpp
    │   │   ├── GpioBase.h
    │   │   ├── GpioEx.h
    │   │   ├── mcp23008
    │   │   │   ├── Mcp23008.cpp
    │   │   │   └── Mcp23008.h
    │   │   ├── mcp23017
    │   │   │   ├── Mcp23017.cpp
    │   │   │   └── Mcp23017.h
    │   │   ├── pcf8574
    │   │   │   ├── Pcf8574.cpp
    │   │   │   └── Pcf8574.h
    │   │   ├── pcf8575
    │   │   │   ├── Pcf8575.cpp
    │   │   │   └── Pcf8575.h
    │   │   ├── ssr74HC595
    │   │   │   ├── Ssr74HC595.cpp
    │   │   │   └── Ssr74HC595.h
    │   │   ├── sws
    │   │   │   ├── Sws.cpp
    │   │   │   └── Sws.h
    │   │   └── tca9555
    │   │       ├── Tca9555.cpp
    │   │       └── Tca9555.h
    │   ├── Macros.h
    │   ├── math
    │   │   ├── Crc.cpp
    │   │   └── Crc.h
    │   ├── nv                              //
    │   │   ├── device
    │   │   │   ├── 24xxI2C.h
    │   │   │   ├── DeviceNull.h
    │   │   │   ├── EepromArduino.h
    │   │   │   ├── EepromEmuEsp.h
    │   │   │   ├── EepromEmuM0.h
    │   │   │   ├── Mb85rcI2C.h
    │   │   │   ├── NvDeviceBase.h
    │   │   │   ├── ShimCached.h
    │   │   │   └── ShimDelayedCommit.h
    │   │   ├── NvConfig.h
    │   │   ├── Nv.cpp
    │   │   ├── Nv.h
    │   │   ├── NvIvPartition.cpp
    │   │   ├── NvIvPartition.h
    │   │   ├── NvKvPartition16.cpp
    │   │   ├── NvKvPartition16.h
    │   │   ├── NvKvPartition32.cpp
    │   │   ├── NvKvPartition32.h
    │   │   ├── NvVolume.cpp
    │   │   └── NvVolume.h
    │   ├── pushButton                          //
    │   │   ├── PushButton.cpp
    │   │   └── PushButton.h
    │   ├── sense
    │   │   ├── Sense.cpp
    │   │   └── Sense.h
    │   ├── serial                              // LX200
    │   │   ├── Serial_IP_Ethernet_Client.cpp
    │   │   ├── Serial_IP_Ethernet_Client.h
    │   │   ├── Serial_IP_Ethernet.cpp
    │   │   ├── Serial_IP_Ethernet.h
    │   │   ├── Serial_IP_Wifi_Client.cpp
    │   │   ├── Serial_IP_Wifi_Client.h
    │   │   ├── Serial_IP_Wifi.cpp
    │   │   ├── Serial_IP_Wifi.h
    │   │   ├── Serial_Local.cpp
    │   │   ├── Serial_Local.h
    │   │   ├── Serial_MEGA2560.cpp
    │   │   ├── Serial_MEGA2560.h
    │   │   ├── Serial_ST4_Master.cpp
    │   │   ├── Serial_ST4_Master.h
    │   │   ├── Serial_ST4_Slave.cpp
    │   │   └── Serial_ST4_Slave.h
    │   ├── softSpi
    │   │   ├── Pins.h
    │   │   ├── SoftSpi.cpp
    │   │   └── SoftSpi.h
    │   ├── sound
    │   │   ├── Sound.cpp
    │   │   └── Sound.h
    │   ├── tasks                           //
    │   │   ├── HAL_ATMEGA328_HWTIMER.h
    │   │   ├── HAL_EMPTY_HWTIMER.h
    │   │   ├── HAL_ESP32_HWTIMER.h
    │   │   ├── HAL_ESP32_V3_HWTIMER.h
    │   │   ├── HAL_HWTIMERS.h
    │   │   ├── HAL_MEGA2560_HWTIMER.h
    │   │   ├── HAL_PROFILER.h
    │   │   ├── HAL_STM32_HWTIMER.h
    │   │   ├── HAL_TEENSY_HWTIMER.h
    │   │   ├── OnTask.cpp
    │   │   ├── OnTaskExample.ino.txt
    │   │   └── OnTask.h
    │   ├── tls
    │   │   ├── ds3231
    │   │   │   ├── DS3231.cpp
    │   │   │   └── DS3231.h
    │   │   ├── ds3234
    │   │   │   ├── DS3234.cpp
    │   │   │   └── DS3234.h
    │   │   ├── gps
    │   │   │   ├── GPS.cpp
    │   │   │   └── GPS.h
    │   │   ├── ntp
    │   │   │   ├── NTP.cpp
    │   │   │   └── NTP.h
    │   │   ├── PPS.cpp
    │   │   ├── PPS.h
    │   │   ├── sd3031
    │   │   │   ├── SD3031.cpp
    │   │   │   └── SD3031.h
    │   │   ├── teensy
    │   │   │   ├── Teensy.cpp
    │   │   │   └── Teensy.h
    │   │   ├── TlsBase.cpp
    │   │   ├── TlsBase.h
    │   │   └── Tls.h
    │   ├── watchdog                        //
    │   │   ├── Watchdog.cpp
    │   │   └── Watchdog.h
    │   └── wifi                            //
    │       ├── cmdServer
    │       │   ├── CmdServer.cpp
    │       │   └── CmdServer.h
    │       ├── webServer
    │       │   ├── WebServer.cpp
    │       │   └── WebServer.h
    │       ├── WifiManager.cpp
    │       ├── WifiManager.defaults.h
    │       └── WifiManager.h
    ├── libApp
    │   ├── commands
    │   │   ├── CommandBroker.cpp
    │   │   ├── CommandBroker.h
    │   │   ├── ProcessCmds.cpp
    │   │   └── ProcessCmds.h
    │   ├── temperature
    │   │   ├── Ds1820.cpp
    │   │   ├── Ds1820.h
    │   │   ├── Temperature.cpp
    │   │   ├── Temperature.h
    │   │   ├── Thermistor.cpp
    │   │   └── Thermistor.h
    │   └── weather
    │       ├── Weather.cpp
    │       └── Weather.h
    ├── pinmaps
    │   ├── Models.h
    │   ├── Pins.CNC3.h
    │   ├── Pins.defaults.h
    │   ├── Pins.FYSETC_E4.h
    │   ├── Pins.FYSETC_S6.h
    │   ├── Pins.JTWPCB2.h
    │   ├── Pins.Manticore.h
    │   ├── Pins.MaxESP3.h
    │   ├── Pins.MaxESP4.h
    │   ├── Pins.MaxPCB36.h
    │   ├── Pins.MaxPCB3.h
    │   ├── Pins.MaxPCB4.h
    │   ├── Pins.MaxPCB.h
    │   ├── Pins.MaxSTM.h
    │   ├── Pins.MicroScope.h
    │   ├── Pins.MiniPCB.h
    │   ├── Pins.OctopusPro.h
    │   ├── Pins.SAL_XB1.h
    │   ├── Pins.SKR_PRO.h
    │   ├── Pins.STM32B.h
    │   └── Validate.h
    ├── plugins
    │   ├── Plugins.config.h
    │   ├── sample
    │   │   ├── Sample.cpp
    │   │   └── Sample.h
    │   └── website
    │       ├── Common.h
    │       ├── Config.h
    │       ├── Constants.h
    │       ├── libApp
    │       │   ├── cmd
    │       │   │   ├── Cmd.cpp
    │       │   │   └── Cmd.h
    │       │   ├── misc
    │       │   │   ├── Misc.cpp
    │       │   │   └── Misc.h
    │       │   └── status
    │       │       ├── StateAuxiliary.cpp
    │       │       ├── StateController.cpp
    │       │       ├── State.cpp
    │       │       ├── StateFocuser.cpp
    │       │       ├── State.h
    │       │       ├── StateMount.cpp
    │       │       ├── StateRotator.cpp
    │       │       ├── Status.cpp
    │       │       ├── Status.h
    │       │       ├── Version.cpp
    │       │       └── Version.h
    │       ├── locales
    │       │   ├── Locale.h
    │       │   ├── Locales.h
    │       │   ├── Strings_ca.h
    │       │   ├── Strings_cn.h
    │       │   ├── Strings_de.h
    │       │   ├── Strings_en.h
    │       │   ├── Strings_es.h
    │       │   ├── Strings_fr.h
    │       │   ├── Strings_it.h
    │       │   └── Strings_jp.h
    │       ├── pages
    │       │   ├── auxiliary
    │       │   │   ├── Auxiliary.cpp
    │       │   │   └── Auxiliary.h
    │       │   ├── Err404.cpp
    │       │   ├── focuser
    │       │   │   ├── BacklashTcfTile.cpp
    │       │   │   ├── BacklashTcfTile.h
    │       │   │   ├── Focuser.cpp
    │       │   │   ├── Focuser.h
    │       │   │   ├── HomeTile.cpp
    │       │   │   ├── HomeTile.h
    │       │   │   ├── SelectTile.cpp
    │       │   │   ├── SelectTile.h
    │       │   │   ├── SlewingTile.cpp
    │       │   │   └── SlewingTile.h
    │       │   ├── htmlHeaders.h
    │       │   ├── htmlMessages.h
    │       │   ├── htmlScripts.h
    │       │   ├── index
    │       │   │   ├── AmbientTile.cpp
    │       │   │   ├── AmbientTile.h
    │       │   │   ├── AxisTile.cpp
    │       │   │   ├── AxisTile.h
    │       │   │   ├── Index.cpp
    │       │   │   ├── Index.h
    │       │   │   ├── ServoTile.cpp
    │       │   │   ├── ServoTile.h
    │       │   │   ├── StatusTile.cpp
    │       │   │   └── StatusTile.h
    │       │   ├── KeyValue.cpp
    │       │   ├── KeyValue.h
    │       │   ├── LibraryHelp.cpp
    │       │   ├── LibraryHelp.h
    │       │   ├── mount
    │       │   │   ├── AlignTile.cpp
    │       │   │   ├── AlignTile.h
    │       │   │   ├── CoordinateTile.cpp
    │       │   │   ├── CoordinateTile.h
    │       │   │   ├── GotoTile.cpp
    │       │   │   ├── GotoTile.h
    │       │   │   ├── GuideTile.cpp
    │       │   │   ├── GuideTile.h
    │       │   │   ├── HomeParkTile.cpp
    │       │   │   ├── HomeParkTile.h
    │       │   │   ├── LibraryTile.cpp
    │       │   │   ├── LibraryTile.h
    │       │   │   ├── LimitsTile.cpp
    │       │   │   ├── LimitsTile.h
    │       │   │   ├── Mount.cpp
    │       │   │   ├── Mount.h
    │       │   │   ├── PecTile.cpp
    │       │   │   ├── PecTile.h
    │       │   │   ├── SiteTile.cpp
    │       │   │   ├── SiteTile.h
    │       │   │   ├── TrackingTile.cpp
    │       │   │   └── TrackingTile.h
    │       │   ├── network
    │       │   │   ├── Network.cpp
    │       │   │   └── Network.h
    │       │   ├── Page.cpp
    │       │   ├── Page.h
    │       │   ├── Pages.common.h
    │       │   ├── Pages.h
    │       │   └── rotator
    │       │       ├── BacklashTile.cpp
    │       │       ├── BacklashTile.h
    │       │       ├── DeRotatorTile.cpp
    │       │       ├── DeRotatorTile.h
    │       │       ├── HomeTile.cpp
    │       │       ├── HomeTile.h
    │       │       ├── Rotator.cpp
    │       │       ├── Rotator.h
    │       │       ├── SlewingTile.cpp
    │       │       └── SlewingTile.h
    │       ├── Website.cpp
    │       └── Website.h
    ├── telescope                               
    │   ├── addonFlasher
    │   │   ├── AddonFlasher.cpp
    │   │   └── AddonFlasher.h
    │   ├── auxiliary
    │   │   ├── dewHeater
    │   │   │   ├── DewHeater.cpp
    │   │   │   └── DewHeater.h
    │   │   ├── FeaturesBase.h
    │   │   ├── Features.h
    │   │   ├── intervalometer
    │   │   │   ├── Intervalometer.cpp
    │   │   │   └── Intervalometer.h
    │   │   ├── local
    │   │   │   ├── Features.can.command.cpp
    │   │   │   ├── Features.command.cpp
    │   │   │   ├── Features.cpp
    │   │   │   └── Features.h
    │   │   ├── powerMonitor
    │   │   │   ├── PowerMonitor.cpp
    │   │   │   └── PowerMonitor.h
    │   │   └── remote
    │   │       ├── Features.can.decode.cpp
    │   │       ├── Features.can.encode.cpp
    │   │       ├── Features.cpp
    │   │       └── Features.h
    │   ├── focuser                             //
    │   │   ├── FocuserBase.h
    │   │   ├── Focuser.h
    │   │   ├── local
    │   │   │   ├── Focuser.axis4.cpp
    │   │   │   ├── Focuser.axis5.cpp
    │   │   │   ├── Focuser.axis6.cpp
    │   │   │   ├── Focuser.axis7.cpp
    │   │   │   ├── Focuser.axis8.cpp
    │   │   │   ├── Focuser.axis9.cpp
    │   │   │   ├── Focuser.axis.prototype.cpp
    │   │   │   ├── Focuser.can.command.cpp
    │   │   │   ├── Focuser.command.cpp
    │   │   │   ├── Focuser.cpp
    │   │   │   └── Focuser.h
    │   │   └── remote
    │   │       ├── Focuser.can.decode.cpp
    │   │       ├── Focuser.can.encode.cpp
    │   │       ├── Focuser.cpp
    │   │       └── Focuser.h
    │   ├── mount                               //
    │   │   ├── coordinates
    │   │   │   ├── Align.hs.cpp
    │   │   │   ├── Align.hs.h
    │   │   │   ├── Align.ref.cpp
    │   │   │   ├── Align.ref.h
    │   │   │   ├── coordinates.ino
    │   │   │   ├── Transform.cpp
    │   │   │   └── Transform.h
    │   │   ├── goto
    │   │   │   ├── Goto.command.cpp
    │   │   │   ├── Goto.cpp
    │   │   │   └── Goto.h
    │   │   ├── guide
    │   │   │   ├── Guide.command.cpp
    │   │   │   ├── Guide.cpp
    │   │   │   └── Guide.h
    │   │   ├── home
    │   │   │   ├── Home.command.cpp
    │   │   │   ├── Home.cpp
    │   │   │   └── Home.h
    │   │   ├── library
    │   │   │   ├── Library.command.cpp
    │   │   │   ├── Library.cpp
    │   │   │   └── Library.h
    │   │   ├── limits
    │   │   │   ├── Limits.command.cpp
    │   │   │   ├── Limits.cpp
    │   │   │   └── Limits.h
    │   │   ├── Mount.axis.cpp
    │   │   ├── Mount.command.cpp
    │   │   ├── Mount.cpp
    │   │   ├── Mount.h
    │   │   ├── park
    │   │   │   ├── Park.command.cpp
    │   │   │   ├── Park.cpp
    │   │   │   └── Park.h
    │   │   ├── pec
    │   │   │   ├── Pec.command.cpp
    │   │   │   ├── Pec.cpp
    │   │   │   └── Pec.h
    │   │   ├── site
    │   │   │   ├── Site.command.cpp
    │   │   │   ├── Site.cpp
    │   │   │   └── Site.h
    │   │   ├── st4
    │   │   │   ├── St4.cpp
    │   │   │   └── St4.h
    │   │   └── status
    │   │       ├── Status.command.cpp
    │   │       ├── Status.cpp
    │   │       └── Status.h
    │   ├── rotator
    │   │   ├── local
    │   │   │   ├── Rotator.axis.cpp
    │   │   │   ├── Rotator.can.command.cpp
    │   │   │   ├── Rotator.command.cpp
    │   │   │   ├── Rotator.cpp
    │   │   │   └── Rotator.h
    │   │   ├── remote
    │   │   │   ├── Rotator.can.decode.cpp
    │   │   │   ├── Rotator.can.encode.cpp
    │   │   │   ├── Rotator.cpp
    │   │   │   └── Rotator.h
    │   │   ├── RotatorBase.h
    │   │   └── Rotator.h
    │   ├── Telescope.command.cpp                   //
    │   ├── Telescope.cpp                           //
    │   └── Telescope.h                             //
    └── Validate.h
