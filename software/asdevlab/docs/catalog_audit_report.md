# CATALOG AUDIT REPORT

## 1. Tổng quan hiện trạng

Đã kiểm tra cấu trúc catalog hiện tại dưới [software/asdevlab/data/catalog](../data/catalog) và các header C++ liên quan trong [software/asdevlab/include/asdevlab/catalog](../include/asdevlab/catalog).

### Thực tế đã xác nhận
- Có 5 file JSON catalog chính:
  - [software/asdevlab/data/catalog/deep_sky/deep_sky.json](../data/catalog/deep_sky/deep_sky.json)
  - [software/asdevlab/data/catalog/solar_system/solar_system.json](../data/catalog/solar_system/solar_system.json)
  - [software/asdevlab/data/catalog/small_bodies/small_bodies.json](../data/catalog/small_bodies/small_bodies.json)
  - [software/asdevlab/data/catalog/satellites/satellites.json](../data/catalog/satellites/satellites.json)
  - [software/asdevlab/data/catalog/stars/stars.json](../data/catalog/stars/stars.json)
- Tổng số object hiện có: 149
  - Deep sky: 60
  - Satellites: 20
  - Small bodies: 30
  - Solar system: 9
  - Stars: 30
- Các file JSON đều parse được bằng trình phân tích JSON chuẩn, nên về mặt cú pháp không có lỗi JSON.

### Điểm quan trọng
- Hiện tại catalog có thể được đọc và index bởi engine, nhưng phần tọa độ chưa phù hợp cho GOTO vì:
  - RA/DEC trong nhiều object đang là chuỗi như "05h 35m 17.3s" hoặc null.
  - Mô hình C++ hiện tại trong [software/asdevlab/include/asdevlab/catalog/catalog_object.hpp](../include/asdevlab/catalog/catalog_object.hpp) mong đợi `ra` và `dec` là số, không phải chuỗi biểu diễn thiên văn.
  - Mạch resolver trong [software/asdevlab/include/asdevlab/catalog/coordinate_resolver.hpp](../include/asdevlab/catalog/coordinate_resolver.hpp) chỉ dùng `StaticCoordinateProvider` cho phần lớn object, và chỉ chọn ephemeris provider khi `type` là `planet`.

## 2. Những phần đã đúng

### Đúng về cấu trúc cơ bản
- Thư mục phân nhóm theo loại object đã rõ ràng và dễ quản lý.
- Mỗi object có ít nhất `id`, `name`, `type`.
- Các field như `messier`, `ngc`, `ic`, `alias`, `constellation`, `fun_fact` đã có trong hầu hết object.
- Không phát hiện duplicate ID và không phát hiện duplicate name trong 5 file hiện có.

### Đúng về khả năng tìm kiếm ban đầu
- C++ engine có thể index theo `id` và `name` thông qua [software/asdevlab/src/catalog/catalog_engine.cpp](../src/catalog/catalog_engine.cpp).
- Search và filter theo loại object là có thể thực hiện.
- Với các object có dữ liệu đủ, catalog có thể phục vụ UI chọn object và hiển thị thông tin.

## 3. Những lỗi cần sửa

### Lỗi nghiêm trọng
1. Schema không thống nhất về tọa độ
- Deep sky và stars dùng chuỗi RA/DEC như "05h 35m 17.3s".
- Solar system, satellites, small bodies dùng `null` cho RA/DEC.
- C++ hiện tại cần số học cho `ra`/`dec`, nên dữ liệu này không thể dùng trực tiếp cho GOTO.

2. Field name không thống nhất
- File hiện dùng `mag`, trong khi model C++ dùng `magnitude`.
- Parser trong code hỗ trợ cả hai, nhưng về mặt schema thì không nhất quán.

3. Không có schema riêng cho coordinate source
- Với solar system, satellites và small bodies, việc dùng tọa độ tĩnh là không đủ.
- Cần có thông tin về source/epoch để resolver biết nên dùng static hay ephemeris.

4. Một số nhóm không đủ dữ liệu cho GOTO
- Solar system: không có coordinate provider phù hợp cho planet/moon.
- Satellites: không có TLE hoặc orbital data.
- Small bodies: không có orbital elements hoặc định nghĩa ephemeris.

### Lỗi nhẹ hơn
- `constellation` có thể là chuỗi rỗng ở nhiều object; điều này không phá vỡ hệ thống nhưng làm trải nghiệm search/organize kém hơn.
- `alias` có nhiều object rỗng; điều này không gây lỗi nhưng chưa tối ưu cho UX.
- `fun_fact` không ảnh hưởng đến GOTO, chỉ phục vụ UI.

## 4. Schema JSON đề xuất

Để phù hợp với luồng hiện tại của ASDEVLAB, nên dùng schema thống nhất như sau:

```json
{
  "id": "dso_m42",
  "name": "Orion Nebula",
  "type": "nebula",
  "magnitude": 4.0,
  "ra": 5.5881,
  "dec": -5.3911,
  "epoch": "J2000",
  "coordinate_source": "catalog",
  "messier": "42",
  "ngc": "1976",
  "ic": "",
  "alias": ["M42", "Great Orion Nebula"],
  "constellation": "Orion",
  "fun_fact": "..."
}
```

### Ghi chú
- `ra` nên là giờ góc, `dec` nên là độ.
- `epoch` nên có giá trị như `J2000` hoặc `JNOW`.
- `coordinate_source` nên có giá trị như `catalog`, `ephemeris`, `tle`, `orbit`.
- Nếu muốn mở rộng cho future, nên thêm:
  - `catalog_family`
  - `coordinate_system`
  - `source_reference`

## 5. Những field bắt buộc

### Bắt buộc cho mọi object
- `id`
- `name`
- `type`
- `magnitude`
- `ra`
- `dec`

### Bắt buộc cho mục tiêu EAA / lookup
- `alias`
- `constellation`
- `messier`
- `ngc`
- `ic`

### Bắt buộc cho GOTO thực tế
- `ra` và `dec` phải là số hợp lệ.
- `epoch` phải có.
- `coordinate_source` phải có.

## 6. Danh sách JSON cần tạo lại

### A. Deep sky
File: [software/asdevlab/data/catalog/deep_sky/deep_sky.json](../data/catalog/deep_sky/deep_sky.json)
- Cần đổi toàn bộ RA/DEC từ chuỗi sang số.
- Nên chuẩn hóa tên field `mag` thành `magnitude`.
- Đảm bảo object như M42, M31, M51, NGC/IC phổ biến đều có dữ liệu đủ cho chọn object.

### B. Stars
File: [software/asdevlab/data/catalog/stars/stars.json](../data/catalog/stars/stars.json)
- Cần đổi RA/DEC từ chuỗi sang số.
- Nên chuẩn hóa `mag` thành `magnitude`.
- Đã đủ cho alignment và calibration, nhưng vẫn cần số cho GOTO.

### C. Solar system
File: [software/asdevlab/data/catalog/solar_system/solar_system.json](../data/catalog/solar_system/solar_system.json)
- Cần tạo lại vì hiện tại `ra`/`dec` là `null`.
- Với Sun, Moon, Mercury, Venus, Mars, Jupiter, Saturn, Uranus, Neptune nên dùng ephemeris hoặc epoch-based coordinates thay vì static values.
- Đây là nhóm cần xử lý riêng, không nên cố ép dùng tọa độ tĩnh.

### D. Satellites
File: [software/asdevlab/data/catalog/satellites/satellites.json](../data/catalog/satellites/satellites.json)
- Cần đổi schema riêng cho tracking vệ tinh.
- Không nên dùng static RA/DEC như hiện tại.
- Nên bổ sung TLE hoặc orbital data thay vì chỉ tọa độ cố định.

### E. Small bodies
File: [software/asdevlab/data/catalog/small_bodies/small_bodies.json](../data/catalog/small_bodies/small_bodies.json)
- Cần schema riêng cho asteroid/comet.
- Nếu mục tiêu chỉ là chọn object và hiển thị, có thể dùng approximation.
- Nếu mục tiêu GOTO thực sự, cần orbital elements hoặc ephemeris provider.

## 7. Mức độ ưu tiên sửa

### P0 - bắt buộc
- Chuẩn hóa tất cả object về cùng một schema tọa độ: `ra` và `dec` phải là số hợp lệ.
- Bỏ toàn bộ `null` cho RA/DEC và bỏ chuỗi biểu diễn thiên văn.
- Đảm bảo deep sky và stars có dữ liệu đủ cho GOTO.
- Cho solar system cần có cơ chế ephemeris hoặc epoch-based coordinate.

### P1 - nên sửa
- Chuẩn hóa field `mag` thành `magnitude`.
- Thêm `epoch` và `coordinate_source` cho mọi object.
- Tách schema riêng cho satellites và small bodies.
- Bổ sung alias và constellation đầy đủ hơn cho các object không có thông tin.

### P2 - có thể cải thiện
- Thêm `catalog_family`, `source_reference`, `notes`.
- Bổ sung `magnitude` cho các object thiếu hoặc chưa đủ chính xác.
- Tăng số lượng deep sky object từ 60 lên mức 50-60 object phổ biến cho EAA và giữ danh sách đã chọn ổn định.

## Kết luận

Hiện tại catalog đã đủ tốt để phục vụ việc hiển thị và lựa chọn object, nhưng chưa đủ tốt để phục vụ luồng GOTO một cách tin cậy. Điểm nghẽn lớn nhất là tọa độ: dữ liệu hiện tại chưa thống nhất và phần lớn không phù hợp với mô hình C++ đang dùng. Nếu mục tiêu là "User chọn M42 -> Catalog Engine -> Coordinate Resolver -> RA/DEC -> LX200 -> OnStepX GOTO", thì bước đầu tiên cần làm là chuẩn hóa toàn bộ catalog về schema tọa độ số hợp lệ và phân nhóm riêng cho các object cần ephemeris.
