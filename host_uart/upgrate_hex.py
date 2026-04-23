import serial
import serial.tools.list_ports
import tkinter as tk
from tkinter import ttk, scrolledtext, END, BooleanVar, filedialog
import threading
import re
import time
from intelhex import IntelHex

class UartUpperComputer:
    def __init__(self, root):
        self.root = root
        self.root.title("UART 串口上位机 (HEX自动升级版)")
        self.root.geometry("820x720")
        self.root.resizable(False, False)

        self.ser = serial.Serial()
        self.serial_thread = None
        self.is_receiving = False
        self.upgrade_running = False
        self.recv_buffer = b''

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

        ttk.Label(self.frame_config, text="数据位：8  校验：None  停止位：1").place(x=350, y=10)

        self.btn_scan = ttk.Button(self.frame_config, text="扫描串口", command=self.scan_serial)
        self.btn_scan.place(x=10, y=45)

        self.btn_open = ttk.Button(self.frame_config, text="打开串口", command=self.open_serial)
        self.btn_open.place(x=100, y=45)

        self.btn_close = ttk.Button(self.frame_config, text="关闭串口", command=self.close_serial, state=tk.DISABLED)
        self.btn_close.place(x=190, y=45)

        self.hex_mode = BooleanVar()
        self.chk_hex = ttk.Checkbutton(self.frame_config, text="HEX 模式(发送/接收)", variable=self.hex_mode)
        self.chk_hex.place(x=300, y=45)

        # ===================== 升级区 =====================
        self.frame_upgrade = ttk.LabelFrame(root, text="MCU 自动升级 (支持 HEX/BIN)")
        self.frame_upgrade.place(x=10, y=120, width=800, height=80)

        ttk.Label(self.frame_upgrade, text="升级文件：").place(x=10, y=10)
        self.entry_file = ttk.Entry(self.frame_upgrade, width=50)
        self.entry_file.place(x=80, y=10, height=28)

        self.btn_file = ttk.Button(self.frame_upgrade, text="选择文件", command=self.select_upgrade_file)
        self.btn_file.place(x=400, y=10, width=80)

        self.btn_upgrade = ttk.Button(self.frame_upgrade, text="开始升级", command=self.start_upgrade_thread, state=tk.DISABLED)
        self.btn_upgrade.place(x=490, y=10, width=80)

        self.progress_var = tk.DoubleVar()
        self.progress_bar = ttk.Progressbar(self.frame_upgrade, variable=self.progress_var, maximum=100)
        self.progress_bar.place(x=580, y=10, width=200, height=28)

        self.label_status = ttk.Label(self.frame_upgrade, text="状态：等待", foreground="blue")
        self.label_status.place(x=10, y=45)

        # ===================== 接收区 =====================
        self.frame_receive = ttk.LabelFrame(root, text="接收数据")
        self.frame_receive.place(x=10, y=210, width=800, height=380)

        self.txt_receive = scrolledtext.ScrolledText(self.frame_receive, font=("Consolas", 10))
        self.txt_receive.place(x=5, y=5, width=785, height=360)

        self.btn_clear_receive = ttk.Button(self.frame_receive, text="清空接收", command=lambda: self.txt_receive.delete(1.0, END))
        self.btn_clear_receive.place(x=720, y=0, width=80)

        # ===================== 发送区 =====================
        self.frame_send = ttk.LabelFrame(root, text="发送数据")
        self.frame_send.place(x=10, y=600, width=800, height=120)

        self.entry_send = ttk.Entry(self.frame_send, font=("Consolas", 12))
        self.entry_send.place(x=5, y=5, width=690, height=60)
        self.entry_send.bind("<Return>", self.send_data)

        self.btn_send = ttk.Button(self.frame_send, text="发送", command=self.send_data)
        self.btn_send.place(x=700, y=5, width=90, height=60)

        self.scan_serial()
        self.file_path = ""

    def scan_serial(self):
        port_list = [port.device for port in serial.tools.list_ports.comports()]
        self.cmb_port["values"] = port_list
        if port_list:
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
            self.log(f"✅ 串口 {self.ser.port} 已打开")
        except Exception as e:
            self.log(f"❌ 打开失败：{str(e)}")

    def close_serial(self):
        self.is_receiving = False
        self.upgrade_running = False
        if self.ser.is_open:
            self.ser.close()
        self.btn_open.config(state=tk.NORMAL)
        self.btn_close.config(state=tk.DISABLED)
        self.btn_upgrade.config(state=tk.DISABLED)
        self.label_status.config(text="状态：已关闭", foreground="red")
        self.log("🔌 串口已关闭")

    def receive_data(self):
        while self.is_receiving:
            if self.ser.is_open and self.ser.in_waiting > 0:
                try:
                    data = self.ser.read(self.ser.in_waiting)
                    self.recv_buffer += data

                    if self.hex_mode.get():
                        hex_str = " ".join(f"{b:02X}" for b in data)
                        self.log(f"📥 RECV: {hex_str}")
                except:
                    pass
            time.sleep(0.01)

    def send_data(self, event=None):
        if not self.ser.is_open:
            self.log("⚠️ 请先打开串口")
            return

        send_text = self.entry_send.get().strip()
        if not send_text:
            return

        try:
            if self.hex_mode.get():
                send_text = re.sub(r'0x', '', send_text)
                send_bytes = bytes.fromhex(send_text)
            else:
                send_bytes = send_text.encode("utf-8")

            self.ser.write(send_bytes)
            self.log(f"📤 SEND: {' '.join(f'{b:02X}' for b in send_bytes)}")
        except Exception as e:
            self.log(f"❌ 发送错误：{str(e)}")

    def log(self, msg):
        self.txt_receive.insert(END, msg + "\n")
        self.txt_receive.see(END)

    # ===================== 升级全自动化 =====================
    def select_upgrade_file(self):
        path = filedialog.askopenfilename(
            title="选择升级文件",
            filetypes=[("HEX文件", "*.hex"), ("BIN文件", "*.bin"), ("所有文件", "*.*")]
        )
        if path:
            self.file_path = path
            self.entry_file.delete(0, END)
            self.entry_file.insert(0, path)
            self.label_status.config(text="状态：已选择文件", foreground="green")

    def start_upgrade_thread(self):
        if not self.file_path:
            self.log("❌ 请先选择升级文件")
            return
        if not self.ser.is_open:
            self.log("❌ 请先打开串口")
            return
        if self.upgrade_running:
            self.log("⚠️ 升级正在进行...")
            return

        self.upgrade_running = True
        self.btn_upgrade.config(state=tk.DISABLED)
        self.progress_var.set(0)
        threading.Thread(target=self.auto_upgrade_process, daemon=True).start()

    def wait_bytes(self, target, timeout=2):
        t0 = time.time()
        self.recv_buffer = b''
        while time.time() - t0 < timeout:
            if target in self.recv_buffer:
                self.recv_buffer = b''
                return True
            time.sleep(0.01)
        return False

    def auto_upgrade_process(self):
        try:
            self.log("\n==================== 自动升级开始 ====================")

            # ===================== 新增：先发进入boot指令 0x99 0x99 0x99 0x99 =====================
            self.label_status.config(text="状态：发送进入boot指令 0x99 0x99 0x99 0x99", foreground="blue")
            self.ser.write(b'\x99\x99\x99\x99')
            self.log("📤 自动发送：0x99 0x99 0x99 0x99 进入boot，等待1秒...")
            time.sleep(1)  # 等待2秒

            # ========== 步骤1：发送开始升级 ==========
            self.label_status.config(text="状态：发送启动指令 0x12 0x34", foreground="blue")
            self.ser.write(b'\x12\x34')
            self.log("📤 自动发送：0x12 0x34 通知开始升级，等待2s应答")

            if not self.wait_bytes(b'\xA5\xB5', 2):
                self.log("❌ 2s内未收到 0xA5 0xB5 擦除完成应答，升级失败")
                self.upgrade_fail()
                return
            self.log("✅ 收到 0xA5 0xB5，擦除完成")

            # ========== 步骤2：发送数据头帧 ==========
            self.label_status.config(text="状态：发送头帧 0xC5 0xD5", foreground="blue")
            self.ser.write(b'\xC5\xD5')
            self.log("📤 自动发送：0xC5 0xD5，等待2s应答")

            if not self.wait_bytes(b'\x22\x22', 2):
                self.log("❌ 2s内未收到 0x22 0x22 应答，升级失败")
                self.upgrade_fail()
                return
            self.log("✅ 收到 0x22 0x22，准备发数据")

            # ========== 读取文件（支持 HEX + BIN 自动解析）==========
            try:
                if self.file_path.lower().endswith(".hex"):
                    ih = IntelHex(self.file_path)
                    file_data = ih.tobinstr()
                    self.log("📄 解析 HEX 文件成功")
                else:
                    with open(self.file_path, 'rb') as f:
                        file_data = f.read()
                    self.log("📄 读取 BIN 文件成功")
            except Exception as e:
                self.log(f"❌ 文件读取失败：{str(e)}")
                self.upgrade_fail()
                return

            # ========== 分包512字节发送 ==========
            pkg_size = 512
            pkts = [file_data[i:i+pkg_size] for i in range(0, len(file_data), pkg_size)]
            total = len(pkts)

            for i, pkg in enumerate(pkts):
                self.label_status.config(text=f"状态：发送数据包 {i+1}/{total}", foreground="blue")
                if len(pkg) < pkg_size:
                    pkg += b'\x00' * (pkg_size - len(pkg))

                self.ser.write(pkg)
                self.log(f"📤 发送第 {i+1} 包 512 字节，等待2s应答")

                if not self.wait_bytes(b'\x22\x22', 2):
                    self.log(f"❌ 第{i+1}包2s内无应答，升级失败")
                    self.upgrade_fail()
                    return

                self.log(f"✅ 第{i+1}包成功")
                self.progress_var.set((i+1)/total * 90)

            # ========== 步骤4：发送结束帧 ==========
            self.label_status.config(text="状态：发送结束帧", foreground="blue")
            self.ser.write(b'\x66\x77\x88\x99')
            self.log("📤 自动发送结束帧：0x66 0x77 0x88 0x99，等待2s应答")

            # ========== 步骤5：等待成功 ==========
            if self.wait_bytes(b'\x99\x99', 2):
                self.log("🎉 2s内收到 0x99 0x99，升级成功！")
                self.progress_var.set(100)
                self.label_status.config(text="状态：升级成功 ✅ 可再次升级", foreground="green")
            else:
                self.log("❌ 2s内未收到升级成功应答 0x99 0x99，升级失败")
                self.upgrade_fail()
                return

            # ========== 允许再次升级（关键）==========
            self.upgrade_running = False
            self.btn_upgrade.config(state=tk.NORMAL)
            self.log("🔄 已重置状态，可直接再次点击【开始升级】")

        except Exception as e:
            self.log(f"💥 升级异常：{e}")
            self.upgrade_fail()

    def upgrade_fail(self):
        self.upgrade_running = False
        self.btn_upgrade.config(state=tk.NORMAL)
        self.label_status.config(text="状态：升级失败 ❌ 可重试", foreground="red")

    def on_closing(self):
        self.close_serial()
        self.root.destroy()

if __name__ == "__main__":
    root = tk.Tk()
    app = UartUpperComputer(root)
    root.protocol("WM_DELETE_WINDOW", app.on_closing)
    root.mainloop()