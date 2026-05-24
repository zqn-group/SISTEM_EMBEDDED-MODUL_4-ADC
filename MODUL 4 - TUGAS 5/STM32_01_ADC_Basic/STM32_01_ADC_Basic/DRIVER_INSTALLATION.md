# STM32 USB Driver Installation Guide

## Solusi Terbaik: STM32CubeProgrammer Driver

STM32CubeProgrammer sudah include driver yang compatible dengan semua STM32.

### **LANGKAH 1: Download STM32CubeProgrammer**
- URL: https://www.st.com/en/development-tools/stm32cubeprog.html
- Atau direct: https://my.st.com/content/dam/st_community/project/stm32cubeprogrammer/en/latest/STM32CubeProgrammer-w64-latest.zip

### **LANGKAH 2: Install STM32CubeProgrammer**
1. Download file ZIP
2. Extract ke folder `C:\STM32CubeProgrammer`
3. Run installer `SetupSTM32CubeProgrammer.exe`
4. Driver akan automatic install

### **LANGKAH 3: Verify Installation**
1. Buka Device Manager (Win+X → Device Manager)
2. Cari di bagian "Universal Serial Bus devices"
3. Harus ada: **"STM32 STLink" atau "STLink Debugger"**

### **LANGKAH 4: Update PlatformIO Configuration**
Jika menggunakan STM32CubeProgrammer, update `platformio.ini`:

```ini
[env:bluepill_f103c8]
platform = ststm32
board = bluepill_f103c8
framework = arduino
upload_protocol = stlink
monitor_speed = 115200
```

---

## **ALTERNATIF: Manual Uninstall & Reinstall**

### **Step A: Uninstall Current Driver**
1. **Plug in ST-Link ke USB**
2. **Buka Device Manager** (Win+X → Device Manager)
3. **Cari di "Universal Serial Bus devices"** atau **"Other devices"**
4. Cari item yang berwarna kuning atau mencurigakan
5. **Right-click → Uninstall Device**
6. **Centang "Delete the driver software for this device"**
7. **Klik Uninstall**

### **Step B: Manual Driver Delete**
```powershell
# Run as Administrator
pnputil -e | findstr /i "STM32 STLINK"
pnputil -d "STM32..." REM (gunakan nama dari step sebelumnya)
```

### **Step C: Reconnect & Auto-Install**
1. **Cabut USB ST-Link**
2. **Tunggu 10 detik**
3. **Plug kembali USB**
4. Windows akan auto-detect dan install compatible driver
5. Tunggu 30-60 detik sampai installation selesai

---

## **Troubleshooting**

### **Jika device tidak terdeteksi:**
- Coba port USB yang berbeda
- Coba USB cable yang berbeda (bukan charging-only)
- Reset board dengan menekan tombol RESET

### **Jika device ada tapi gabisa upload:**
- Masukkan BOOTLOADER MODE: pin BOOT0 ke 3.3V + tekan RESET
- Atau update firmware ST-Link di: https://www.st.com/en/development-tools/stsw-link007.html

---

## **Rekomendasi**
**Gunakan STM32CubeProgrammer** - paling lengkap dan compatible karena dari ST official.
