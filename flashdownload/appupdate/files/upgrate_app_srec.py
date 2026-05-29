import serial
import serial.tools.list_ports
import tkinter as tk
from tkinter import ttk, scrolledtext, END, BooleanVar, filedialog
import threading
import time
import os

class UartUpperComputer:
    def __init__(self, root):
        self.root = root
        self.root.title("UART 上位机 - SREC A/B分区 512字节整包FF填充版")
        self.root.geometry("820x750")
        self.root.resizable(False, False)

        self.ser = serial.Serial()
        self.serial_thread = None
        self.is_receiving = False
        self.upgrade_running = False
        self.recv_buffer = b''

        self.srec_records = []
        self.fw_data_final = b''
        self.fw_addr_final = 0
        self.fw_size_final = 0

        self.PKG_SIZE = 512

        # ===================== 串口配置 =====================
        self.frame_config = ttk.LabelFrame(root, text="串口配置")
        self.frame_config.place(x=10, y=10, width=800, height=100)

        ttk.Label(self.frame_config, text="串口号：").place(x=10, y=10)
        self.cmb_port = ttk.Combobox(self.frame_config, width=10)
        self.cmb_port.place(x=70, y=10)

        ttk.Label(self.frame_config, text="波特率：").place(x=180, y=10)
        self.cmb_baudrate = ttk.Combobox(self.frame_config, width=10)
        self.cmb_baudrate["values"] = ["9600", "19200", "38400", "57600", "115200"]
        self.cmb_baudrate.current(4)
        self.cmb_baudrate.place(x=240, y=10)

        self.btn_scan = ttk.Button(self.frame_config, text="扫描串口", command=self.scan_serial)
        self.btn_scan.place(x=10, y=45)

        self.btn_open = ttk.Button(self.frame_config, text="打开串口", command=self.open_serial)
        self.btn_open.place(x=100, y=45)

        self.btn_close = ttk.Button(self.frame_config, text="关闭串口", command=self.close_serial, state=tk.DISABLED)
        self.btn_close.place(x=190, y=45)

        self.hex_mode = BooleanVar()
        self.chk_hex = ttk.Checkbutton(self.frame_config, text="HEX 模式", variable=self.hex_mode)
        self.chk_hex.place(x=300, y=45)

        # ===================== 升级区 =====================
        self.frame_upgrade = ttk.LabelFrame(root, text="SREC 升级（A/B合并固件自动分区提取）")
        self.frame_upgrade.place(x=10, y=120, width=800, height=150)

        ttk.Label(self.frame_upgrade, text="SREC 文件：").place(x=10, y=10)
        self.entry_file = ttk.Entry(self.frame_upgrade, width=45)
        self.entry_file.place(x=80, y=10, height=28)
        self.btn_file = ttk.Button(self.frame_upgrade, text="选择 SREC", command=self.select_upgrade_file)
        self.btn_file.place(x=380, y=10, width=80)

        ttk.Label(self.frame_upgrade, text="当前分区：").place(x=10, y=50)
        self.entry_area = ttk.Entry(self.frame_upgrade, width=12)
        self.entry_area.place(x=90, y=50, height=28)
        self.entry_area.config(state="readonly")

        ttk.Label(self.frame_upgrade, text="分区地址：").place(x=180, y=50)
        self.entry_addr = ttk.Entry(self.frame_upgrade, width=12)
        self.entry_addr.place(x=250, y=50, height=28)
        self.entry_addr.config(state="readonly")

        ttk.Label(self.frame_upgrade, text="有效大小：").place(x=340, y=50)
        self.entry_size = ttk.Entry(self.frame_upgrade, width=12)
        self.entry_size.place(x=400, y=50, height=28)
        self.entry_size.config(state="readonly")

        self.btn_upgrade = ttk.Button(self.frame_upgrade, text="开始升级", command=self.start_upgrade_thread, state=tk.DISABLED)
        self.btn_upgrade.place(x=500, y=50, width=90)

        self.progress_var = tk.DoubleVar()
        self.progress_bar = ttk.Progressbar(self.frame_upgrade, variable=self.progress_var, maximum=100)
        self.progress_bar.place(x=600, y=50, width=180, height=28)

        self.label_status = ttk.Label(self.frame_upgrade, text="状态：等待操作", foreground="blue")
        self.label_status.place(x=10, y=90)
        self.label_srec = ttk.Label(self.frame_upgrade, text="SREC：未加载", foreground="gray")
        self.label_srec.place(x=250, y=90, width=500)

        # ===================== 接收区 =====================
        self.frame_receive = ttk.LabelFrame(root, text="接收日志")
        self.frame_receive.place(x=10, y=280, width=800, height=340)
        self.txt_receive = scrolledtext.ScrolledText(self.frame_receive, font=("Consolas", 10))
        self.txt_receive.place(x=5, y=5, width=785, height=330)
        self.btn_clear = ttk.Button(self.frame_receive, text="清空", command=lambda: self.txt_receive.delete(1.0, END))
        self.btn_clear.place(x=720, y=0, width=70)

        # ===================== 发送区 =====================
        self.frame_send = ttk.LabelFrame(root, text="指令发送")
        self.frame_send.place(x=10, y=630, width=800, height=100)
        self.entry_send = ttk.Entry(self.frame_send, font=("Consolas", 12))
        self.entry_send.place(x=5, y=5, width=690, height=60)
        self.btn_send = ttk.Button(self.frame_send, text="发送", command=self.send_data)
        self.btn_send.place(x=700, y=5, width=90, height=60)

        self.scan_serial()
        self.file_path = ""

    # -------------------------------------------------------------------------
    def scan_serial(self):
        ports = [p.device for p in serial.tools.list_ports.comports()]
        self.cmb_port["values"] = ports
        if ports:
            self.cmb_port.current(0)

    def open_serial(self):
        try:
            self.ser.port = self.cmb_port.get()
            self.ser.baudrate = int(self.cmb_baudrate.get())
            self.ser.bytesize = serial.EIGHTBITS
            self.ser.parity = serial.PARITY_NONE
            self.ser.stopbits = serial.STOPBITS_ONE
            self.ser.timeout = 0.5
            self.ser.open()

            self.is_receiving = True
            self.serial_thread = threading.Thread(target=self.receive_data, daemon=True)
            self.serial_thread.start()

            self.btn_open.config(state=tk.DISABLED)
            self.btn_close.config(state=tk.NORMAL)
            self.btn_upgrade.config(state=tk.NORMAL if self.file_path else tk.DISABLED)
            self.log("✅ 串口已打开")
        except Exception as e:
            self.log(f"❌ 打开失败：{e}")

    def close_serial(self):
        self.is_receiving = False
        self.upgrade_running = False
        if self.ser.is_open:
            self.ser.close()
        self.btn_open.config(state=tk.NORMAL)
        self.btn_close.config(state=tk.DISABLED)
        self.btn_upgrade.config(state=tk.DISABLED)
        self.log("🔌 串口已关闭")

    def receive_data(self):
        while self.is_receiving:
            if self.ser.is_open and self.ser.in_waiting > 0:
                try:
                    data = self.ser.read(self.ser.in_waiting)
                    self.recv_buffer += data
                    if self.hex_mode.get():
                        self.log(f"📥 {data.hex(' ')}")
                except:
                    pass
            time.sleep(0.01)

    def send_data(self, event=None):
        if not self.ser.is_open:
            self.log("⚠️ 请打开串口")
            return
        txt = self.entry_send.get().strip()
        try:
            b = bytes.fromhex(txt.replace("0x",""))
            self.ser.write(b)
            self.log(f"📤 发送: {b.hex(' ')}")
        except:
            self.log("❌ 格式错误")

    def log(self, msg):
        self.txt_receive.insert(END, msg + "\n")
        self.txt_receive.see(END)

    # -------------------------------------------------------------------------
    def parse_srec_full(self, path):
        try:
            with open(path, 'r') as f:
                lines = [l.strip() for l in f if l.strip().startswith('S')]

            self.srec_records = []
            for line in lines:
                stype = line[1]
                if stype not in '123':
                    continue
                count = int(line[2:4], 16)
                addr_len = 4 if stype == '1' else 6 if stype == '2' else 8
                addr = int(line[4:4+addr_len], 16)
                data_len = count - (addr_len//2) - 1
                data_hex = line[4+addr_len : 4+addr_len + data_len*2]
                data = bytes.fromhex(data_hex)
                self.srec_records.append((addr, data))

            addrs = [a for a,d in self.srec_records]
            ends = [a+len(d)-1 for a,d in self.srec_records]
            min_a = min(addrs)
            max_a = max(ends)
            self.label_srec.config(text=f"SREC范围: 0x{min_a:X} ~ 0x{max_a:X}")
            self.log("✅ SREC 解析完成（保存所有段）")
            return True
        except:
            self.log("❌ SREC 解析失败")
            return False

    def extract_partition_data(self, part_start, part_end):
        segments = []
        for addr, data in self.srec_records:
            a_end = addr + len(data) - 1
            if addr > part_end or a_end < part_start:
                continue
            os0 = max(0, part_start - addr)
            os1 = len(data) - max(0, a_end - part_end)
            final_addr = addr + os0
            final_data = data[os0:os1]
            segments.append((final_addr, final_data))

        if not segments:
            return 0, 0, b''

        min_a = min(a for a,d in segments)
        max_a = max(a+len(d)-1 for a,d in segments)
        size = max_a - min_a + 1
        buf = bytearray([0xff]*size)
        for a,d in segments:
            off = a - min_a
            buf[off:off+len(d)] = d
        return min_a, size, bytes(buf)

    def select_upgrade_file(self):
        path = filedialog.askopenfilename(title="选择 SREC", filetypes=[("SREC", "*.srec *.s19 *.mot")])
        if path:
            self.file_path = path
            self.entry_file.delete(0, END)
            self.entry_file.insert(0, path)
            if self.parse_srec_full(path):
                self.label_status.config(text="状态：SREC 加载成功", foreground="green")
                if self.ser.is_open:
                    self.btn_upgrade.config(state=tk.NORMAL)
            else:
                self.label_status.config(text="状态：SREC 加载失败", foreground="red")

    def wait_bytes(self, target, timeout=2):
        t0 = time.time()
        while time.time() - t0 < timeout:
            if target in self.recv_buffer:
                pos = self.recv_buffer.find(target)
                self.recv_buffer = self.recv_buffer[pos + len(target):]
                return True
            time.sleep(0.01)
        return False

    # -------------------------------------------------------------------------
    def start_upgrade_thread(self):
        if not self.file_path or not self.ser.is_open or not self.srec_records:
            self.log("❌ 请检查文件与串口")
            return
        self.upgrade_running = True
        self.btn_upgrade.config(state=tk.DISABLED)
        self.progress_var.set(0)
        threading.Thread(target=self.auto_process, daemon=True).start()

    def auto_process(self):
        try:
            self.log("\n=============== SREC 分区升级(512字节FF填充) ===============")
            self.recv_buffer = b''

            self.ser.write(b'\x02\x02\x02\x02')
            self.log("📤 发送: 02 02 02 02 (进入boot,表示app升级)")
            time.sleep(0.1)

            area = ""
            part_start = 0
            part_end = 0
            bin_filename = ""

            if self.wait_bytes(b'\xA5\xA5', 2):
                area = "AppA"
                part_start = 0x18000
                part_end = 0x2B7FF
                bin_filename = "Lx07_App_A.bin"
            elif self.wait_bytes(b'\xB5\xB5', 2):
                area = "AppB"
                part_start = 0x2B800
                part_end = 0x3EFFF
                bin_filename = "Lx07_App_B.bin"
            else:
                self.log("❌ 未识别分区")
                self.upgrade_fail()
                return

            addr, size, data = self.extract_partition_data(part_start, part_end)
            if size == 0:
                self.log(f"❌ {area} 无有效数据")
                self.upgrade_fail()
                return

            if not os.path.exists(bin_filename):
                self.log(f"❌ 未找到 {bin_filename}")
                self.upgrade_fail()
                return

            bin_file_size = os.path.getsize(bin_filename)
            self.log(f"✅ 读取 {bin_filename} 大小：{bin_file_size} 字节")

            self.entry_area.config(state="normal")
            self.entry_addr.config(state="normal")
            self.entry_size.config(state="normal")
            self.entry_area.delete(0,END); self.entry_area.insert(0, area)
            self.entry_addr.delete(0,END); self.entry_addr.insert(0, f"0x{addr:X}")
            self.entry_size.delete(0,END); self.entry_size.insert(0, f"{bin_file_size} 字节")
            self.entry_area.config(state="readonly")
            self.entry_addr.config(state="readonly")
            self.entry_size.config(state="readonly")

            self.label_status.config(text="状态：擦除中")
            self.ser.write(b'\x12\x34')
            if not self.wait_bytes(b'\xA5\xB5', 3):
                self.log("❌ 擦除失败")
                self.upgrade_fail()
                return
            self.log("✅ 擦除完成")

            self.ser.write(b'\xC5\xD5')
            if not self.wait_bytes(b'\x22\x22', 2):
                self.log("❌ 未就绪")
                self.upgrade_fail()
                return

            pkg_size = self.PKG_SIZE
            send_length = len(data)
            total = (send_length + pkg_size - 1) // pkg_size
            self.log(f"📦 总包数: {total}")

            for i in range(total):
                if not self.upgrade_running:
                    return
                current_start = i * pkg_size
                current_end = current_start + pkg_size
                if current_start >= send_length:
                    break

                pkg_raw = data[current_start:current_end]
                if len(pkg_raw) < pkg_size:
                    pkg = pkg_raw + b'\xFF' * (pkg_size - len(pkg_raw))
                else:
                    pkg = pkg_raw

                self.label_status.config(text=f"状态：发送 {i+1}/{total}")
                self.ser.write(pkg)

                if not self.wait_bytes(b'\x22\x22', 2):
                    self.log(f"❌ 第{i+1}包失败")
                    self.upgrade_fail()
                    return
                self.log(f"✅ 第{i+1}包成功")
                self.progress_var.set((i+1)/total * 100)

            # 发送结束帧
            self.log("📤 发送结束帧: 66 77 88 99")
            self.ser.write(b'\x66\x77\x88\x99')

            # 等待 99 99 → 发送大小
            self.log("⏳ 等待 MCU 请求大小 99 99...")
            if not self.wait_bytes(b'\x99\x99', 5):
                self.log("❌ 未收到 99 99")
                self.upgrade_fail()
                return
            
            time.sleep(0.2)

            size_bytes = bin_file_size.to_bytes(4, 'big', signed=False)
            self.ser.write(size_bytes)
            self.log(f"📤 已发送固件大小: {size_bytes.hex(' ')}")

            # 等待 AA AA → 发送校验
            self.log("⏳ 等待 MCU 请求校验 AA AA...")
            if not self.wait_bytes(b'\xAA\xAA', 5):
                self.log("❌ 未收到 AA AA")
                self.upgrade_fail()
                return
            if len(data) >=4:
                check_data = data[-4:]
            else:
                check_data = data + b'\xFF'*(4-len(data))
            self.ser.write(check_data)
            self.log(f"📤 已发送校验数据: {check_data.hex(' ')}")

            # ===================== 最终判断：等待2秒，BB成功 / CC校验失败 =====================
            self.log("⏳ 等待结果（2秒超时）...")
            t_wait = time.time()
            while time.time() - t_wait < 2.0:
                if self.wait_bytes(b'\xBB\xBB', 0.1):
                    self.log("🎉 升级成功！")
                    self.label_status.config(text="状态：升级成功 ✅", foreground="green")
                    break
                if self.wait_bytes(b'\xCC\xCC', 0.1):
                    self.log("❌ 校验不通过！")
                    self.upgrade_fail()
                    return
                time.sleep(0.01)
            else:
                self.log("❌ 等待结果超时")
                self.upgrade_fail()
                return

            self.upgrade_running = False
            self.btn_upgrade.config(state=tk.NORMAL)

        except Exception as e:
            self.log(f"💥 异常: {e}")
            self.upgrade_fail()

    def upgrade_fail(self):
        self.upgrade_running = False
        self.btn_upgrade.config(state=tk.NORMAL)
        self.label_status.config(text="状态：升级失败 ❌", foreground="red")

    def on_closing(self):
        self.close_serial()
        self.root.destroy()

if __name__ == "__main__":
    root = tk.Tk()
    app = UartUpperComputer(root)
    root.protocol("WM_DELETE_WINDOW", app.on_closing)
    root.mainloop()