import serial
import serial.tools.list_ports
import tkinter as tk
from tkinter import ttk, scrolledtext, END, BooleanVar, filedialog
import threading
import time

class UartUpperComputer:
    def __init__(self, root):
        self.root = root
        self.root.title("UART 上位机 - 升级boot")
        self.root.geometry("820x750")
        self.root.resizable(False, False)

        self.ser = serial.Serial()
        self.serial_thread = None
        self.is_receiving = False
        self.upgrade_running = False
        self.recv_buffer = b''

        # 自动识别的地址和大小
        self.flash_addr = 0
        self.flash_size = 0

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

        # ===================== 升级区（自动识别A/B区） =====================
        self.frame_upgrade = ttk.LabelFrame(root, text="BIN 升级（自动识别BootA/B）")
        self.frame_upgrade.place(x=10, y=120, width=800, height=120)

        ttk.Label(self.frame_upgrade, text="BIN文件：").place(x=10, y=10)
        self.entry_file = ttk.Entry(self.frame_upgrade, width=45)
        self.entry_file.place(x=80, y=10, height=28)
        self.btn_file = ttk.Button(self.frame_upgrade, text="选择BIN", command=self.select_upgrade_file)
        self.btn_file.place(x=380, y=10, width=80)

        # 自动识别显示
        ttk.Label(self.frame_upgrade, text="烧录区域：").place(x=10, y=50)
        self.entry_area = ttk.Entry(self.frame_upgrade, width=12)
        self.entry_area.place(x=90, y=50, height=28)
        self.entry_area.config(state="readonly")

        ttk.Label(self.frame_upgrade, text="地址：").place(x=180, y=50)
        self.entry_addr = ttk.Entry(self.frame_upgrade, width=12)
        self.entry_addr.place(x=230, y=50, height=28)
        self.entry_addr.config(state="readonly")

        ttk.Label(self.frame_upgrade, text="大小：").place(x=320, y=50)
        self.entry_size = ttk.Entry(self.frame_upgrade, width=12)
        self.entry_size.place(x=360, y=50, height=28)
        self.entry_size.config(state="readonly")

        self.btn_upgrade = ttk.Button(self.frame_upgrade, text="开始升级", command=self.start_upgrade_thread, state=tk.DISABLED)
        self.btn_upgrade.place(x=460, y=50, width=90)

        self.progress_var = tk.DoubleVar()
        self.progress_bar = ttk.Progressbar(self.frame_upgrade, variable=self.progress_var, maximum=100)
        self.progress_bar.place(x=560, y=50, width=220, height=28)

        self.label_status = ttk.Label(self.frame_upgrade, text="状态：等待操作", foreground="blue")
        self.label_status.place(x=10, y=85)

        # ===================== 接收区 =====================
        self.frame_receive = ttk.LabelFrame(root, text="接收日志")
        self.frame_receive.place(x=10, y=250, width=800, height=370)

        self.txt_receive = scrolledtext.ScrolledText(self.frame_receive, font=("Consolas", 10))
        self.txt_receive.place(x=5, y=5, width=785, height=350)
        self.btn_clear = ttk.Button(self.frame_receive, text="清空", command=lambda: self.txt_receive.delete(1.0, END))
        self.btn_clear.place(x=720, y=0, width=70)

        # ===================== 发送区 =====================
        self.frame_send = ttk.LabelFrame(root, text="发送")
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
            self.btn_upgrade.config(state=tk.NORMAL)
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
            self.log(f"📤 {b.hex(' ')}")
        except:
            self.log("❌ 格式错误")

    def log(self, msg):
        self.txt_receive.insert(END, msg + "\n")
        self.txt_receive.see(END)

    # -------------------------------------------------------------------------
    # 校验：BIN 最大地址必须 = 0x00017FFF
    # -------------------------------------------------------------------------
    def check_boot_bin(self, bin_path):
        try:
            with open(bin_path, 'rb') as f:
                data = f.read()
            max_addr = len(data) - 1
            self.log(f"📊 BIN 最大地址: 0x{max_addr:08X}")
            if max_addr == 0x00017FFF:
                self.log("✅ BIN 校验通过")
                return True
            else:
                self.log("❌ BIN 错误！不是合法 Boot 文件")
                return False
        except:
            self.log("❌ BIN 读取失败")
            return False

    # -------------------------------------------------------------------------
    def select_upgrade_file(self):
        path = filedialog.askopenfilename(title="选择BIN", filetypes=[("BIN文件", "*.bin")])
        if path:
            self.file_path = path
            self.entry_file.delete(0, END)
            self.entry_file.insert(0, path)
            self.label_status.config(text="状态：已选择文件", foreground="green")

    def start_upgrade_thread(self):
        if not self.file_path:
            self.log("❌ 请选择BIN")
            return
        if not self.ser.is_open:
            self.log("❌ 请打开串口")
            return
        self.upgrade_running = True
        self.btn_upgrade.config(state=tk.DISABLED)
        self.progress_var.set(0)
        threading.Thread(target=self.auto_process, daemon=True).start()

    def wait_bytes(self, target, timeout=2):
        t0 = time.time()
        while time.time() - t0 < timeout:
            if target in self.recv_buffer:
                idx = self.recv_buffer.find(target)
                self.recv_buffer = self.recv_buffer[idx + len(target):]
                return True
            time.sleep(0.01)
        return False

    # -------------------------------------------------------------------------
    # 核心：自动识别A/B区 + BIN地址校验
    # -------------------------------------------------------------------------
    def auto_process(self):
        try:
            self.log("\n=============== 自动升级流程 ===============")
            self.recv_buffer = b''

            # 先校验 BIN
            if not self.check_boot_bin(self.file_path):
                self.upgrade_fail()
                return

            self.label_status.config(text="状态：发送进入Boot指令", foreground="blue")
            self.ser.write(b'\x01\x01\x01\x01')
            self.log("📤 发送：01 01 01 01（进入Boot）")
            time.sleep(0.1)

            # 2. 等待2s识别 A/B
            self.label_status.config(text="状态：等待BootA/B应答(2s)", foreground="blue")
            if self.wait_bytes(b'\xA5\xA5', 2):
                area = "BootA"
                self.flash_addr = 0x00004000
                self.flash_size = 0x0000A000
            elif self.wait_bytes(b'\xB5\xB5', 2):
                area = "BootB"
                self.flash_addr = 0x0000E000
                self.flash_size = 0x0000A000
            else:
                self.log("❌ 2s内未收到A5A5/B5B5，升级终止")
                self.upgrade_fail()
                return

            # 显示到界面
            self.entry_area.config(state="normal")
            self.entry_addr.config(state="normal")
            self.entry_size.config(state="normal")
            self.entry_area.delete(0,END); self.entry_area.insert(0,area)
            self.entry_addr.delete(0,END); self.entry_addr.insert(0,f"0x{self.flash_addr:X}")
            self.entry_size.delete(0,END); self.entry_size.insert(0,f"0x{self.flash_size:X}")
            self.entry_area.config(state="readonly")
            self.entry_addr.config(state="readonly")
            self.entry_size.config(state="readonly")

            self.log(f"✅ 识别到：{area}")
            self.log(f"📍 地址：0x{self.flash_addr:X}")
            self.log(f"📏 大小：0x{self.flash_size:X}")

            # 3. 发送 12 34
            self.label_status.config(text="状态：发送升级开始指令", foreground="blue")
            self.ser.write(b'\x12\x34')
            self.log("📤 发送：12 34（开始升级）")

            if not self.wait_bytes(b'\xA5\xB5', 2):
                self.log("❌ 未收到 A5 B5 擦除完成")
                self.upgrade_fail()
                return
            self.log("✅ 收到擦除完成应答 A5 B5")

            # 4. 发送 C5 D5
            self.ser.write(b'\xC5\xD5')
            self.log("📤 发送：C5 D5（准备数据）")
            if not self.wait_bytes(b'\x22\x22', 2):
                self.log("❌ 未收到 22 22")
                self.upgrade_fail()
                return
            self.log("✅ 可以开始发送数据")

            # 5. 读取BIN并截取大小
            with open(self.file_path, 'rb') as f:
                f.seek(self.flash_addr)
                burn_data = f.read(self.flash_size)
            self.log(f"📄 截取烧录数据：{len(burn_data)} 字节")

            # 6. 512分包发送
            pkg_size = 512
            pkts = [burn_data[i:i+pkg_size] for i in range(0, len(burn_data), pkg_size)]
            total = len(pkts)

            for i, pkg in enumerate(pkts):
                self.label_status.config(text=f"状态：发送 {i+1}/{total}", foreground="blue")
                if len(pkg) < pkg_size:
                    pkg += b'\x00'*(pkg_size-len(pkg))
                self.ser.write(pkg)
                if not self.wait_bytes(b'\x22\x22', 2):
                    self.log(f"❌ 第{i+1}包失败")
                    self.upgrade_fail()
                    return
                self.log(f"✅ 第{i+1}包成功")
                self.progress_var.set((i+1)/total * 95)

            # 7. 结束帧
            self.ser.write(b'\x66\x77\x88\x99')
            self.log("📤 发送结束帧")
            if self.wait_bytes(b'\x99\x99', 2):
                self.log("🎉 升级成功！")
                self.progress_var.set(100)
                self.label_status.config(text="状态：升级成功 ✅", foreground="green")
            else:
                self.log("❌ 未收到成功应答")
                self.upgrade_fail()
                return

            self.upgrade_running = False
            self.btn_upgrade.config(state=tk.NORMAL)

        except Exception as e:
            self.log(f"💥 异常：{e}")
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