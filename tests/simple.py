# First step in any test will be to generate a machine image containing the AQUA version we're testing.
# So we'll want to clean build the contents here and create an aquarium template.

import asyncio
import os
import shutil
import subprocess

# TODO Rich is really cool but it has a couple problems which I'd like to fix:
# - There doesn't seem to be a way to make Panel display the end of a renderable rather than the beginning. So I have to resort to a fixed height.

from rich.live import Live
from rich.panel import Panel
from rich.text import Text
from rich.layout import Layout
from rich.console import Console


class AquariumPool:
	POOL_PATH = "pool"
	COUNT = 3

	__avail: set[str]

	def __enter__(self):
		if not os.path.exists(self.POOL_PATH):
			os.mkdir(self.POOL_PATH)

		self.__avail = set()

		for i in range(self.COUNT):
			path = f"{self.POOL_PATH}/aquarium-{i}"

			if not os.path.exists(path):
				subprocess.run(["aquarium", "create", path], check=True)

			self.__avail.add(path)

		return self

	def acquire(self) -> str:
		return self.__avail.pop()

	def release(self, path: str):
		self.__avail.add(path)

	def __exit__(self, exc_type, exc_val, exc_tb):
		del exc_type, exc_val, exc_tb


class Bridge:
	__fake: str | None

	def __init__(self, fake: str | None = None):
		self.__fake = fake

	def __enter__(self):
		if self.__fake is not None:
			subprocess.run(["ifconfig", self.__fake], capture_output=True, check=True)  # Make sure it exists.
			self.name = self.__fake

		else:
			res = subprocess.run(["ifconfig", "bridge", "create"], capture_output=True, text=True)

			if res.stderr.strip():
				print(res.stderr)

			res.check_returncode()

			self.name = res.stdout.strip()

		return self

	def __exit__(self, exc_type, exc_val, exc_tb):
		del exc_type, exc_val, exc_tb

		if self.__fake is None:
			subprocess.run(["ifconfig", self.name, "destroy"], check=False)


class Host:
	name: str
	__machine_path: str

	__aquarium_pool: AquariumPool
	__aquarium: str

	def __init__(self, name: str, machine_path: str):
		self.name = name
		self.__machine_path = machine_path

	async def setup(self, aquarium_pool: AquariumPool, ip: str, bridge: Bridge, buf_lines: list[str]):
		self.__aquarium_pool = aquarium_pool

		# Acquire an aquarium for ourselves.

		self.__aquarium = self.__aquarium_pool.acquire()

		# Install gvd to aquarium.

		for entry in subprocess.run(["aquarium"], capture_output=True, check=True).stdout.splitlines():
			pointer, aquarium = entry.split()

			if pointer.decode() == os.path.realpath(self.__aquarium):
				aquarium = aquarium.decode()
				break

		else:
			raise Exception(f"Couldn't find aquarium directory for {ip}.")

		shutil.copyfile(self.__machine_path + "/runner.sh", aquarium + "/runner.sh")
		prefix = aquarium + "/usr/local"
		assert "/usr/local/aquarium/roots/" in prefix  # Just to be safe!

		if os.path.exists(prefix):
			shutil.rmtree(prefix)

		if os.path.exists(aquarium + "/tmp/gv.lock"):
			os.remove(aquarium + "/tmp/gv.lock")

		os.mkdir(prefix)
		subprocess.run(["bob", "-C", self.__machine_path, "build"], check=True)
		subprocess.run(["bob", "-C", self.__machine_path, "-p", prefix, "install"], check=True)

		# Start the gvd process in the aquarium.

		proc = await asyncio.create_subprocess_exec(
			"aquarium",
			"-v",
			bridge.name,
			"enter",
			self.__aquarium,
			stdin=asyncio.subprocess.PIPE,
			stdout=asyncio.subprocess.PIPE,
			stderr=asyncio.subprocess.STDOUT,
			bufsize=0,
		)

		assert proc.stdin is not None
		assert proc.stdout is not None

		proc.stdin.write(f"IP={ip} sh /runner.sh\n".encode())

		while True:
			line = await proc.stdout.readline()

			if not line:
				break

			text = line.decode(errors="replace").rstrip("\n")
			buf_lines.append(text)

		await proc.wait()

	def teardown(self):
		self.__aquarium_pool.release(self.__aquarium)


class Network:
	hosts: dict[str, Host]

	__bufs: dict[str, list[str]]

	def __init__(self, hosts: dict[str, Host]):
		self.hosts = hosts
		self.__bufs = {}

	def __render(self):
		layout = Layout()
		panels = []
		height = 96  # XXX, see comments on Rich.

		for k, buf in self.__bufs.items():
			colour = "cyan"

			if "Tests passed!" in "\n".join(buf):
				colour = "green"

			tail = buf[-height:]
			panels.append(Panel(Text.from_ansi("\n".join(tail)), title=k, border_style=colour))

		layout.split_row(*map(Layout, panels))
		return layout

	async def setup(self, aquarium_pool: AquariumPool, bridge: Bridge):
		tasks = []

		for i, (k, host) in enumerate(self.hosts.items()):
			ip = f"10.0.0.{i}"
			self.__bufs[k] = []
			task = asyncio.create_task(host.setup(aquarium_pool, ip, bridge, self.__bufs[k]))
			tasks.append(task)

		console = Console(color_system="truecolor")

		with Live(self.__render(), console=console, refresh_per_second=5) as live:
			while not all(map(lambda task: task.done(), tasks)):
				live.update(self.__render())
				await asyncio.sleep(0.1)

	def teardown(self):
		for host in self.hosts.values():
			host.teardown()


if __name__ == "__main__":
	fc_graph = Network(
		hosts={
			"A (running the test program)": Host(name="Host A", machine_path="machine_1"),
			"B (providing the VDEV)": Host(name="Host B", machine_path="machine_2"),
			"C (running the test program)": Host(name="Host C", machine_path="machine_1"),
		}
	)

	# with AquariumPool() as aquarium_pool, Bridge(fake="bridge0") as bridge:
	with AquariumPool() as aquarium_pool, Bridge() as bridge:
		try:
			asyncio.run(fc_graph.setup(aquarium_pool, bridge))

		finally:  # XXX This doesn't work.
			fc_graph.teardown()

	subprocess.run(["aquarium", "sweep"], check=True)

	# Each host image should have AQUA + a test VDEV on it, which provides functions to test all the different features AQUA provides.
	# Then, we should have an array of different programs the testing framework can run to simulate different scenarios.
	# We might need some way of analyzing network traffic on the bridges to know if e.g. packets are going through the right ones.
