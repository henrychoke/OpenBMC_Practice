//#include <sdbusplus/bus.hpp>
#include <boost/asio.hpp>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>
#include <sdbusplus/server/manager.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <tuple>

int main() {
	// build permanent connection
	//auto bus = sdbusplus::bus::new_default();
	//1. build unsynchronize IO core
	boost::asio::io_context io;
	auto bus = std::make_shared<sdbusplus::asio::connection>(io);

	//2. request service name
	bus->request_name("xyz.openbmc_project.VAmbient");

	//3. build object server
	sdbusplus::asio::object_server objectServer(bus);

	//4. build sensor object path
	std::string path = "/xyz/openbmc_project/sensors/temperature/ambient_temp";

	//5. add sensor.value and set as readable and writable
	auto iface = objectServer.add_interface(path, "xyz.openbmc_project.Sensor.Value");
	iface->register_property("Value", 25.5, sdbusplus::asio::PropertyPermission::readWrite);
	iface->initialize();

	//6. add OperationalStatus (let WebUI knows that this is normal)
	auto opIface = objectServer.add_interface(path, "xyz.openbmc_project.State.Decorator.OperationalStatus");
        opIface->register_property("Functional", true, sdbusplus::asio::PropertyPermission::readWrite);
        opIface->initialize();

	//7. add association (exposed to Redfish)
	//format: {"remote role","local role","destination path"}
	auto assocIface = objectServer.add_interface(path, "xyz.openbmc_project.Association.Definitions");
	std::vector<std::tuple<std::string, std::string, std::string>> associations;
	associations.emplace_back("chassis", "all_sensors", "/xyz/openbmc_project/inventory/system/chassis");
	assocIface->register_property("Associations", associations);
	assocIface->initialize();

	std::cout << " VAmbient C++ Daemon with associations start successful!" << std::endl;

	io.run();
	return 0;
}
