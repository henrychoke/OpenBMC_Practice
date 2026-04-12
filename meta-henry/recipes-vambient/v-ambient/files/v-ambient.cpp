//#include <sdbusplus/bus.hpp>
#include <boost/asio.hpp>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>
#include <sdbusplus/server/manager.hpp>
#include <iostream>

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

	//4. build object mapper
	sdbusplus::server::manager_t objManager(*bus, "/xyz/openbmc_project/sensors");

	//5. add virtual temp sensor
	auto iface = objectServer.add_interface(
		"/xyz/openbmc_project/sensors/temperature/ambient_temp",
		"xyz.openbmc_project.Sensor.Value");

	//6. register temp value
	iface->register_property("Value",25.5);

	//7. initialize
	iface->initialize();

	std::cout << " VAmbient C++ Daemon start successful!" << std::endl;

	//keep DBus connection
	/*
	while (true) {
		bus.process_discard();
		bus.wait();
	}
	*/
	io.run();
	return 0;
}
