# First step in any test will be to generate a machine image containing the AQUA version we're testing.
# So we'll want to clean build the contents here and create an aquarium template.

from dataclasses import dataclass

@dataclass
class Host:
	name: str

@dataclass
class Network:
	hosts: dict[str, Host]
	bridges: list[set[str]]

	def setup(self):
		# Create the hosts.

		for i, (k, host) in enumerate(self.hosts.items()):
			ip = f"10.0.0.{i}"
			# TODO

		# Create the bridges.

		for bridge in self.bridges:
			... # TODO


test = Network(hosts={
	"A": Host(name="Host A"),
	"B": Host(name="Host B"),
	"C": Host(name="Host C"),
}, bridges=[
	{"A", "B"},
	{"B", "C"},
])

fc_graph = Network(hosts={
	"A": Host(name="Host A"),
	"B": Host(name="Host B"),
	"C": Host(name="Host C"),
}, bridges=[
	{"A", "B", "C"}
])

# Each host image should have AQUA + a test VDEV on it, which provides functions to test all the different features AQUA provides.
# Then, we should have an array of different programs the testing framework can run to simulate different scenarios.
# We might need some way of analyzing network traffic on the bridges to know if e.g. packets are going through the right ones.
